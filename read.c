#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "mpi.h"
#include "adios_read.h"

static inline float solvedeterminant(float a11, float a12, float a13,
                              float a21, float a22, float a23,
                              float a31, float a32, float a33)
{
    return (a11*a22*a33 + a12*a23*a31 + a13*a21*a32
            - a13*a22*a31 - a11*a23*a32 - a12*a21*a33);
}

static inline float interpolation(float *c)
{
    float x[9],y[3]={1,3,8},ret[3],d,i;
    x[0] = c[0] * c[0];
    x[1] = c[0];
    x[2] = 1.0;
    x[3] = c[1] * c[1]+1;
    x[4] = c[1];
    x[5] = 1.0;
    x[6] = c[2] * c[2]+2;
    x[7] = c[2];
    x[8] = 1.0;
    
    d = solvedeterminant(x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],x[8]);
    ret[0] = solvedeterminant(y[0],x[1],x[2],y[1],x[4],x[5],y[2],x[7],x[8])/d;
    ret[1] = solvedeterminant(x[0],y[0],x[2],x[3],y[1],x[5],x[6],y[2],x[8])/d;
    ret[2] = solvedeterminant(x[0],x[1],y[0],x[4],x[5],y[1],x[7],x[8],y[2])/d;
    
    return ret[0];
}

int main (int argc, char ** argv) 
{
    char        filename [256], filename1[256];
    int         rank, i, j, ii, jj;
    int         nx_local, ny_local;
    int         nx_global, ny_global;
    int         offs_x, offs_y;
    float      *data;
    double       t1, t2;
    double       readtime, computetime, writetime;
    int         size;
    uint64_t start[2], count[2];
    uint64_t adios_groupsize, adios_totalsize;
    int64_t  adios_handle;
    MPI_Comm    comm = MPI_COMM_WORLD;
    enum ADIOS_READ_METHOD method = ADIOS_READ_METHOD_BP;
    ADIOS_SELECTION * sel;

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (comm, &rank);
    MPI_Comm_size (comm, &size);

    adios_read_init_method (method, comm, "num_aggregators=4");

    strcpy (filename, "raw.bp");
    ADIOS_FILE * f = adios_read_open (filename, method, comm, ADIOS_LOCKMODE_NONE, 0);
    ADIOS_VARINFO * varinfo = adios_inq_var(f,"data");

    /* Specify a selection that points to a specific writer's block */
    sel = adios_selection_writeblock (rank);
    
    /* First get the scalars to calculate the size of the arrays */
    adios_schedule_read (f, sel, "nx_global", 0, 1, &nx_global);
    adios_schedule_read (f, sel, "ny_global", 0, 1, &ny_global);
    adios_schedule_read (f, sel, "nx_local", 0, 1, &nx_local);
    adios_schedule_read (f, sel, "ny_local", 0, 1, &ny_local);
    adios_perform_reads (f, 1);

    /* Allocate space for the arrays */
    data = (float *) malloc (nx_local*ny_local*sizeof(double));

    sprintf(filename1, "result.bp");
    adios_init("write.xml",comm);
    adios_open(&adios_handle, "writer", filename1, "w", comm);

    readtime = 0.0;
    computetime = 0.0;
    writetime = 0.0;

    for(i = 0; i < nx_global; i++)
        for(j = 0; j < ny_global/(nx_local*ny_local*size); j++)
        {
            offs_x = i ;
            offs_y = j*nx_local*ny_local*size + nx_local*ny_local*rank;

            start[0] = offs_x;
            start[1] = offs_y;
            count[0] = 1;
            count[1] = nx_local*ny_local;

            sel = adios_selection_boundingbox(varinfo->ndim, start, count);

            t1 = MPI_Wtime();
            /* Read the arrays */
            adios_schedule_read (f, sel, "data", 0, 1, data);
            adios_perform_reads (f, 1);
            t2 = MPI_Wtime();
            readtime += t2 - t1;

            t1 = MPI_Wtime();
            /* computing */
            for(ii = 0; ii < nx_local; ii ++)
                for(jj = 0; jj < ny_local; jj++)
                {
                    float c[3];
                    c[1] = data[ii];
                    c[2] = data[jj];
                    c[3] = data[ii * nx_local + jj];
                    data[ii*nx_local+jj] = interpolation(c);
                }
            t2 = MPI_Wtime();
            computetime += t2 - t1;

            t1 = MPI_Wtime();
            /*writing*/
            #include "gwrite_writer.ch"
            t2 = MPI_Wtime();
            writetime += t2 - t1;
        }
    free (data);

    t1 = MPI_Wtime();
    adios_close (adios_handle);
    t2 = MPI_Wtime();
    writetime += t2 - t1;

    if(rank == 0)
    {
        printf("readtime = %f\n", readtime);
        printf("computetime = %f\n", computetime);
        printf("writetime = %f\n", writetime);
    }

    adios_read_close (f);
    MPI_Barrier (comm);
    adios_read_finalize_method (method);
    MPI_Finalize ();

    return 0;
}

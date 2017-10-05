/* 
 * ADIOS is freely available under the terms of the BSD license described
 * in the COPYING file in the top level directory of this source distribution.
 *
 * Copyright (c) 2008 - 2009.  UT-BATTELLE, LLC. All rights reserved.
 */

/*
 * ADIOS example from the User's manual
 *
 * Write a separate file from each process by using the POSIX method or
 * write into a large single file from all processes using the MPI method.
 * You need to change only the method in the config.xml and rerun the 
 * program (no recompile is needed)
 *
 * In case of POSIX method, the output files will have the process rank 
 * appended to the file name (e.g. restart.bp.15).
 *
 * 4_posix_read.c example can read the output data from the same number 
 * of processors and using the same method. 
 * 
 * Application of the example: 
 *    Checkpoint/restart files 
 *
 * Note: bpls utility and the generic read API can see only the array
 * written from one of the processors. You need to use global arrays to
 * make the data available for the utilities or for reading data from 
 * arbitrary number of processors. 
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"
#include "adios.h"


int main (int argc, char ** argv) 
{
    char        filename [256];
    int         rank,size;
    int         nx_global = 1000, ny_global = 1000;
    int         nx_local = 5, ny_local = 5;
    int         offs_x, offs_y;
    int         posx,posy;
    float      data[5][5], c[3], t1, t2 ;
    int         i,j;
    int         readlen = nx_local * ny_local;
    
    /* ADIOS variables declarations for matching gwrite_temperature.ch */
    uint64_t    adios_groupsize, adios_totalsize;
    int64_t     adios_handle;
    MPI_Comm    comm =  MPI_COMM_WORLD;
    
    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD,  &size );
/*
    data = (float**)malloc(nx_local * sizeof(float *));
    for (i=0; i < nx_local; i++)
        data[i] = (float*)malloc(ny_local * sizeof(float));
*/
    for(i = 0; i < nx_local; i++)
        for(j = 0; j < ny_local; j++)
            data[i][j] = 6.6;
    
    sprintf (filename, "raw.bp");
    adios_init ("write.xml", comm);
    adios_open (&adios_handle, "writer", filename, "w", comm);

    t1 = MPI_Wtime();
    for(i = 0; i < nx_global/nx_local; i++)
        for(j = 0 ; j < ny_global/(ny_local*size); j++)
        {
            offs_x = i * nx_local;
            offs_y = j * ny_local * size + ny_local * rank;
            //printf("%d %d\n", offs_x, offs_y);
            #include "gwrite_writer.ch"
        }
    
    adios_close (adios_handle);
    adios_finalize (rank);
   
    t2 = MPI_Wtime();
    if(rank == 0)
        printf( "write time is %f\n", t2 - t1 ); 
    MPI_Finalize ();
    return 0;
}


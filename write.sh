#!/bin/bash

export export PATH=$PATH:/Users/liangweihao/Documents/workspace/adios/adios-1.12.0/build/bin 

mpicc -c write.c -I/Users/liangweihao/Documents/workspace/adios/adios-1.12.0/build/include -I/Users/liangweihao/Documents/workspace/cdo/hdf5-1.8.19/par-build/include

mpicc write.o -L/Users/liangweihao/Documents/workspace/adios/adios-1.12.0/build/lib -ladios -L/Users/liangweihao/Documents/workspace/cdo/hdf5-1.8.19/par-build/lib -lhdf5_hl -lhdf5 -lz

mpirun -n 1 ./a.out

rm *.o a.out

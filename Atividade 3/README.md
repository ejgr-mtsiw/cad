# cad-atividade3
Projects for Activity #3 of High Performance Computation (Computação de Alto Desempenho) 

## mpi-repvc
Reimplement the array sharing exercise from Atividade 1/mpi-repvb but using MPI_Bcast

## mpi-divv
This program shares an array between all the processes in the pool using
blocks of similar, but variable, size using MPI_Scatterv

## mpi-somav
This program uses MPI_Reduce to aggregate partial sums calculated from a random
array scattered using MPI_Scatterv

## mpi-dmoccg
This program reimplements Atividade 2/dmocc using MPI_Scatter and MPI_reduce

## mpi-mcpi
This program calculates pi using the Monte Carlo method
[https://en.wikipedia.org/wiki/Monte_Carlo_method]
through a master-slave topology

# cad-atividade1
Projects for Activity #1 of High Performance Computation (Computação de Alto Desempenho)

## mpi-test
Tries to initialize the MPI environment and prints a simple text message on success or error

## mpi-hello-world
Hello World using MPI

Each spawned process will reply with a greeting

## mpi-somanp
This program calculates the sum of the running processes spawned.

Each process will add its value (rank + 1) to the running total and pass it to the next process, starting with process 0.
The last process sends it back to process 0 that prints the total.
 
Ex: `mpirun -np 5 somanp`
```c
processo 0: enviado 1
processo 1: recebido 1
processo 1: enviado 3
processo 2: recebido 3
processo 2: enviado 6
processo 3: recebido 6
processo 4: recebido 10
processo 3: enviado 10
processo 4: enviado 15
processo 0: recebido 15
Total: 15
```

## mpi-repvb
This program shares an array between all the processes in the pool
 
Process #0 creates an array with the length indicated by the user in the command line.
In each iteration of the algorithm all the processes with rank k < 2^i send the array to the process 2^i + k.

Ex: `mpirun -np 8 repvb.out 15`
```c
processo 7: v[15] = 0, 1, ..., 14
processo 0: v[15] = 0, 1, ..., 14
processo 1: v[15] = 0, 1, ..., 14
processo 2: v[15] = 0, 1, ..., 14
processo 4: v[15] = 0, 1, ..., 14
processo 5: v[15] = 0, 1, ..., 14
processo 6: v[15] = 0, 1, ..., 14
processo 3: v[15] = 0, 1, ..., 14
```

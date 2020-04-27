#!/usr/bin/bash

echo "dmo"
for i in {1..1000}
do
   /usr/lib64/openmpi/bin/mpirun -np 8 ~/projects/ejgr-github/cad-atividade2/dmo/dmo.out 40
done

echo "dmocc"
for i in {1..1000}
do
   /usr/lib64/openmpi/bin/mpirun -np 8 ~/projects/ejgr-github/cad-atividade2/dmocc/dmocc.out 40
done


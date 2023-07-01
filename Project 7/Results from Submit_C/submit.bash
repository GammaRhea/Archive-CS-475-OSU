#!/bin/bash
#SBATCH -J Proj07
#SBATCH -A cs475-575
#SBATCH -p classmpifinal
#SBATCH -N 8	# number of nodes
#SBATCH -n 8	# number of tasks
#SBATCH --constraint=ib
#SBATCH -o proj07.out
#SBATCH -e proj07.err
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=rheac@oregonstate.edu
module load openmpi

for p in 1 2 4 8
do
	mpic++ proj07.cpp -o proj07 -lm
	mpiexec -np $p ./proj07
done
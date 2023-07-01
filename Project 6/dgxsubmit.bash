#!/bin/bash
#SBATCH -J OpenCLMatrixMult
#SBATCH -A cs475-575
#SBATCH -p classgpufinal
#SBATCH --constraint=v100
#SBATCH --gres=gpu:1
#SBATCH -o matrixmult.out
#SBATCH -e matrixmult.err
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=rheac@oregonstate.edu

for s in 32 64 128 256 512 1024
do
        for b in 1 2 4 8 16
        do
                g++ -DMATW=$s -DLOCALSIZE=$b -o proj06 proj06.cpp /usr/local/apps/cuda/cuda-10.1/lib64/libOpenCL.so.1.1 -lm -fopenmp
                ./proj06
        done
done
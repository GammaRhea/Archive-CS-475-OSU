#!/bin/bash
for n in 1024 2048 4096 8192 16384 32768
do
  for m in true true true true true false false false false false false false false false false
  do
       g++  proj3.cpp  -DNUMN=$n -DUSE_MUTEX=$m  -o proj3  -lm  -fopenmp
      ./proj3
  done
done
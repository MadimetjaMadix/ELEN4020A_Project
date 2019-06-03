## ELEN4020A Project 2019

##### Introduction
In this project, MPI-IO and Out of Core Matrix Transposition of Big Data is performed.

##### Problem
The problem definition is simply to develop and implement a parallel matrix transposition 
algorithm for very large datasets that will be held out-of-core, in a file.

##### program execution
The code requires a mpich3 library in order for it to be executed. The [code](https://github.com/MadimetjaMadix/ELEN4020A_Project/tree/master/Code) can be executed
in the following maner:
- mpicc -o transpose transpose.c
- mpiexec -np **np** ./transpose matrixFile_**n**.bin

where: **np** represent the number of threads/processors.

**n** represents the matrix size.

##### Work allocation
The number of commits for this repo does not reflect upon the contribution for the project. The
 true reflection of the work allocation can be found in the project [report](https://github.com/MadimetjaMadix/ELEN4020A_Project/tree/master/Report) in the Appendix.
 

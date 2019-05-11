#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

#define INTS_PER_BLK 3

int main(int argc, char **argv)
{
	int *buf, rank, nprocs, nints, bufsize;
	MPI_File fh;
	MPI_Datatype filetype;
	
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	
	
	
	MPI_File_open(MPI_COMM_WORLD, "test.txt", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	
	MPI_Offset FILESIZE;
	MPI_File_get_size(fh, &FILESIZE);
	
	bufsize = FILESIZE/nprocs;
	buf = (int *)malloc(bufsize);
	
	nints = bufsize/sizeof(int);
	
	MPI_Type_vector(nints/INTS_PER_BLK, INTS_PER_BLK, INTS_PER_BLK*nprocs, MPI_INT, &filetype );
	
	MPI_Type_commit(&filetype);
	
	MPI_File_set_view(fh, INTS_PER_BLK*sizeof(int)*rank, MPI_INT, filetype, "native", MPI_INFO_NULL);
	
	MPI_File_read_all(fh, buf, nints, MPI_INT, MPI_STATUS_IGNORE);
	
	MPI_File_close(&fh);
	
	for(int i = 0; i<INTS_PER_BLK; i++)
		printf("process %d has  %d\n",rank, buf[i]);

	MPI_Type_free(&filetype);
	free(buf);
	MPI_Finalize();
	return 0;



}

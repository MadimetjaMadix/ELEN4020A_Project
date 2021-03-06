/* 
* This program demonstrates the use of MPI_Alltoall when
* transpsong a square matrix.
* For simplicity, the number of processes is 4 and the dimension
* of the matrix is fixed to 128
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>


int getElementLocation2D(int index[2], int N)
{
	int row    = index[0];
	int column = index[1];

	return ((row*N)+column);
}

void swapElements(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void trans(int *subMatrix, int N)
{
	for(int i=0; i<N; ++i)
	{
		for(int j=i+1; j<N; ++j)
		{
			int index[2] = {i,j};
			int offsetA = getElementLocation2D(index, N);
			index[0] = j;
			index[1] = i;			
			int offsetB = getElementLocation2D(index, N);
			swapElements(subMatrix+offsetA, subMatrix+offsetB);
		}
	}
}

void trans2 (int* a, int n)
/* transpose square matrix a, dimension nxn
 * Consider this as a black box for the MPI course
 */

{
  int i, j;
  int ij, ji, l;
  double tmp;
  ij = 0;
  l = -1;
  for (i = 0; i < n; i++)
    {
      l += n + 1;
      ji = l;
      ij += i + 1;
      for (j = i+1; j < n; j++)
	{
	  tmp = a[ij];
	  a[ij] = a[ji];
	  a[ji] = tmp;
	  ij++;
	  ji += n;
	}
    }
}

void print(int *matrix, int N, int number_of_elements)
{
	if(matrix != NULL)
	{	
		printf("\n");
		for(int i = 0; i < number_of_elements; ++i)
		{
			if( i%N == 0) printf("\n");

			printf("%4d ", *matrix);
			++matrix;
		}
		printf("\n");

	}else printf(" The Matrix is empty");

}

int main (int argc, char *argv[])
{
	char *filename = argv[1];

	int nprocs, rank, err = 0, errs = 0, *buf, n;
	MPI_File fh;
	MPI_Datatype filetype;
	MPI_Status status;
	
	MPI_Init (&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;
	MPI_Comm_size (MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank (MPI_COMM_WORLD, &rank);
	
	err = MPI_File_open(comm, filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	if (err)
	{
		MPI_Abort(MPI_COMM_WORLD, 911);
	}
	
	buf = (int *)malloc(sizeof(int) );
	MPI_File_read( fh, buf, 1, MPI_INT, &status ); // Matrix size
	n = buf[0];
    
	if (n%nprocs != 0 && (nprocs<n))
	{
	  if (rank == 0)
			printf ("Error, number of processes must be multiples of %d \n", n );
		
		MPI_File_close(&fh);
		free(buf);
	  	MPI_Finalize ();
	  	return 1;
	}
  
	int m  = (int)(n/nprocs);
	int* a = (int *)malloc(n*m*sizeof(int));
	int* b = (int *)malloc(n*m*sizeof(int));

	MPI_Offset block_length = n*m;
	// Read data from file:
	MPI_Offset displace = ((rank*block_length)+1)*sizeof(int);// Set view for each processor using an offset
	MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
	MPI_File_read(fh, a, block_length, MPI_INT, &status );
	MPI_File_close(&fh);
	
	//
	if(rank == 1)
	{	
		printf("======================== Rank  %d ============================\n",rank);
		for (int i = 0; i < nprocs; i++)
		print (a+(i*m*m), n, m*m);
	}
	// do the MPI part of the transpose 

	// Tricky here is the number of items to send and receive. 
	// Not 128x32 as one may guess, but the amount to send to one process
	// and the amount to receive from any process 
	for(int i = 0; i< m)
	MPI_Alltoall (a,	// address of data to send 
		m,	// number of items to send to one process
		MPI_INT,//	type of data 
		b,	// address for receiving the data 
		// NOTE: send data and receive data may NOT overlap
		m,	// number of items to receive 
				//   from any process 
		MPI_INT,	// type of receive data 
		comm);

	// MPI_Alltoall does not a transpose of the data received, we have to
	// do this ourself:

	// transpose 4 square matrices, order mxm:

	/*for (i = 0; i < nprocs; i++)
	  trans(b+(i*m), m);*/

	// now check the result 
	/*if(rank == 1)
	{
		printf("========================Transposed rank %d ============================\n",rank);
		for (i = 0; i < nprocs; i++)
		print(b+(i*m*m),n,m*m);
		
	}
	// Write Transposed Matrix:
	err = MPI_File_open(comm, "output.bin", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
	if (err)
    {
        MPI_Abort(MPI_COMM_WORLD, 911);
    }
    
	displace = ((rank*block_length))*sizeof(int); // start of the view for each processor
	MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
	err = MPI_File_write(fh, b, block_length, MPI_INT, &status );
	if (err) { errs++; }
	MPI_File_close(&fh);
	
	free(buf);
	free(b);
	
	if(rank == 0)
	{
		// Check transpose:
		int* originalMatrix = (int*)malloc(n*n*sizeof(int));
		err = MPI_File_open(MPI_COMM_SELF, "matrixFile_8.bin", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
		displace = 4;
		MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
		MPI_File_read(fh, originalMatrix, n*n, MPI_INT, &status);
		MPI_File_close(&fh);
		
		
		int* transposedMatrix = (int*)malloc(n*n*sizeof(int));
		err = MPI_File_open(MPI_COMM_SELF, "output.bin", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
		MPI_File_read(fh, transposedMatrix, n*n, MPI_INT, &status);
		MPI_File_close(&fh);
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
			{
				int index[2] = {i,j};
				int offsetTransposed = getElementLocation2D(index, n);
				index[0] = j;
				index[1] = i;
				int offsetOriginal   = getElementLocation2D(index, n);
				if (*(transposedMatrix+offsetTransposed) != *(originalMatrix+offsetOriginal))
					{
						printf ("process %d found transposed[%d][%d] = %d, but %d was expected\n",
						rank, i, j, *(transposedMatrix+offsetTransposed), *(originalMatrix+offsetOriginal));
						MPI_Abort (MPI_COMM_WORLD, 1);
						MPI_File_close(&fh);
						free(a);
						free(transposedMatrix);
						free(originalMatrix);
						MPI_Finalize ();
						return 1;
					}
		}//for
	 
		printf ("Transpose seems ok\n");
		free(transposedMatrix);
		free(originalMatrix);
	}//if
	
	*/
	free(a);
	MPI_Finalize ();
	return errs;
	}


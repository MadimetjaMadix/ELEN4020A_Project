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

void print(int *matrix, int N)
{
	if(matrix != NULL)
	{	
		printf("\n");
		for(int i=0; i< N*N; ++i)
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

	int nprocs, rank, err, *buf, n = 8;
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
	MPI_Offset offset = 1; 
    MPI_Offset displace = 4; // start of the view for each processor
    
    buf = (int *)malloc(sizeof(int) );
    
	if(rank == 0)
	{	
		printf("about to view ...\n");
		MPI_File_set_view( fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
		
		printf("about to read ...\n");
		
		MPI_File_read( fh, buf, 1, MPI_INT, &status );
		//n = buf[0];
		printf("Transposing a %d x %d matrix, divided among 4 processors\n",  buf[0], buf[0]);
		MPI_File_close(&fh);
	}
	if (n%nprocs != 0 && (nprocs<n))
	{
	  if (rank == 0)
			printf ("Error, number of processes must be multiples of %d \n", n );
		
		MPI_File_close(&fh);
		free(buf);
	  	MPI_Finalize ();
	  	return 1;
	}
  /*
	int m = (int)(n/nprocs);
	int a[n][m];
	int b[n][m];
	int i,j;


	for ( i = 0; i < n; i++)
	for ( j = 0; j < m; j++)
	  a[i][j] =(int)( 1 * i + j + m * rank); // give every element a unique value

	printf("======================== Rank  %d ============================\n",rank);
	//for (i = 0; i < 4; i++)
	// print (&a[i * m][0], m);
	// do the MPI part of the transpose 

	// Tricky here is the number of items to send and receive. 
	// Not 128x32 as one may guess, but the amount to send to one process
	// and the amount to receive from any process 

	MPI_Alltoall (&a[0][0],	// address of data to send 
		m * m,	// number of items to send to one process
		MPI_INT,	type of data 
		&b[0][0],	// address for receiving the data 
		// NOTE: send data and receive data may NOT overlap
		m * m,	// number of items to receive 
				//   from any process 
		MPI_INT,	// type of receive data 
		MPI_COMM_WORLD);

	// MPI_Alltoall does not a transpose of the data received, we have to
	// do this ourself:

	// transpose 4 square matrices, order 32x32:

	for (i = 0; i < nprocs; i++)
	  trans (&b[i * m][0], m);

	// now check the result 

	printf("========================Transposed rank %d ============================\n",rank);
	//for (i = 0; i < 4; i++)
	//  print (&b[i * m][0],m);


	for (i = 0; i < n; i++)
	for (j = 0; j < m; j++)
	  {
	if (b[i][j] != (int)(1* (j + m * rank) + i) )
	  {
		printf ("process %d found b[%d][%d] = %d, but %d was expected\n",
			rank, i, j, b[i][j], (int) (1 * (j + m * rank) + i));
		MPI_Abort (MPI_COMM_WORLD,1);
		return 1;
	  }
	  }
	if (rank == 0)
	printf ("Transpose seems ok\n");
	*/
	MPI_File_close(&fh);
	free(buf);
	MPI_Finalize ();
	return 0;
	}


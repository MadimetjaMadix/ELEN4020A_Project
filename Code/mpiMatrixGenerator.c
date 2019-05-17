#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void generateRandomNumbers(int* buffer, int N)
{	
	for(int i = 0; i < N; ++i)
	{
		 *buffer = rand()%99;
		 ++buffer;
	}// for
	
}

int main( int argc, char *argv[] )
{
		
    int errs = 0, err;
    int size, rank, *randomNumbersBuffer;
    
    int n[6]= {8, 16, 32, 64, 128, 4096};
		char nsize[30];
    
    MPI_File fh;
    MPI_Comm comm;
    MPI_Status status;

    MPI_Init( &argc, &argv );
    comm = MPI_COMM_WORLD;
    
		for(int i = 0; i < 6; i++)
		{
			
			char* filename = "matrixFile_";
			int N = n[i];
			
			// Create filename:
			sprintf(nsize,"%d.bin",N);
			filename = concat(filename,nsize);
			
			// Open file:
		  err = MPI_File_open(comm, filename, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
		  MPI_Offset displace = 0;
		  if (err)
		  {
		      MPI_Abort(MPI_COMM_WORLD, 911);
		  }
		  
		  MPI_Comm_size( comm, &size );
		  MPI_Comm_rank( comm, &rank );
		  srand(rank*time(NULL));
		  if(rank == 0)
		  {	
		  	MPI_Offset temp = 1;
		  	// Write Size of matrix as first element:
				err = MPI_File_write(fh, (n+i), temp, MPI_INT, &status);
		  }//if
		  
		  // Divide the matrix size into chunks:
		  MPI_Offset chunk_size = (N*N/size);
		  
		  // Allocate memory for the chunk size:
		  randomNumbersBuffer = (int*)malloc((int)chunk_size * sizeof(int));
		  
		  // Generate numbers:
			generateRandomNumbers(randomNumbersBuffer, (int)chunk_size);
			
			// Write to file:
		  // Create the offset based on rank:
		  displace = ((rank*chunk_size)+1 )*sizeof(int); // start of the view for each processor
			
		  MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
		  err = MPI_File_write_all(fh, randomNumbersBuffer, chunk_size, MPI_INT, &status );
		  if (err) { errs++; }
		  
		  // Free buffer:
		  free(randomNumbersBuffer);
		  
		  // Close file handle:
		  err = MPI_File_close( &fh );
		  if (err) { errs++; }
    }
    MPI_Finalize();
    return errs;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <mpi.h>


int getElementLocation2D(int index[2], int N)
{
	int row    = index[0];
	int column = index[1];

	return ((row*N)+column);
}


int main(int argc, char** argv)
{
	struct timeval start, end;
	gettimeofday(&start, NULL);
	
	char *input_filename = argv[1];
	char *output_file_name = argv[1]; 

	int rank, num_procs, N, *tmp_buffer, err = 0, errs = 0;

	MPI_Win win;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	

	MPI_File fh;
	MPI_Status status;
	
	
	// Open input file for reading:
	err = MPI_File_open(MPI_COMM_WORLD, input_filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	tmp_buffer = (int *)malloc(sizeof(int) );
	MPI_File_read(fh, tmp_buffer, 1, MPI_INT, &status ); // Matrix size
	N = tmp_buffer[0];
	free(tmp_buffer);
	
	if (N%num_procs != 0 && ((num_procs < N ) || (num_procs > N)))
	{
	  if (rank == 0)
			printf ("Error, number of processes must be multiples of %d , and less than it. \n", N);
		
		MPI_File_close(&fh);
	  	MPI_Finalize ();
	  	return 1;
	}
	
	int m  = (int)(N/num_procs);
	
	int *localbuffer  = (int *)malloc(N*m*sizeof(int));
	int *sharedbuffer = (int *)malloc(N*m*sizeof(int));
	
	// Create window:
	MPI_Win_create(sharedbuffer, N*m*sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
	
	
	// Read data from file:
	MPI_Offset block_length = N*m;
	MPI_Offset displace = ((rank*block_length)+1)*sizeof(int);// Set view for each processor using an offset
	MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
	
	// :
	MPI_File_read(fh, sharedbuffer, block_length, MPI_INT, &status);
	MPI_File_close(&fh);

	if (err)
	{
		MPI_Abort(MPI_COMM_WORLD, 911);
	}
	
	MPI_Win_fence(0, win);
	
	int counter = 0;
	for(int column = rank*m; column < ((rank*m)+m); column++)
	{
		// Traverse along columns of shared buffer and extract block NxM :
		for(int processor_id = 0; processor_id < num_procs; processor_id++)
		{	
			int row_start_index_in_proc_block  = 0;
			for(int row_in_shared_buffer = 0; row_in_shared_buffer < m; row_in_shared_buffer++)
			{
				MPI_Get(localbuffer+counter, 1, MPI_INT, processor_id, row_start_index_in_proc_block+column, 1, MPI_INT, win);
				++counter;
				row_start_index_in_proc_block+=N;
			}
		
		}
		//column++;
	}
	MPI_Win_fence(0, win);
	
	// Open file for writing:
	err = MPI_File_open(MPI_COMM_WORLD, output_file_name, MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
	if (err)
    {
        MPI_Abort(MPI_COMM_WORLD, 911);
    }
 	
 	// Save local buffers with transposed elements to file:
	displace = ((rank*block_length)+1)*sizeof(int); // start of the view for each processor
	MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );

	err = MPI_File_write(fh, localbuffer, block_length, MPI_INT, &status);
	MPI_File_close(&fh);
	if (err) { errs++; }

	
	MPI_Win_free(&win);
	free(sharedbuffer);
	free(localbuffer);
	MPI_Finalize();
	
	if(rank == 0)
	{
		gettimeofday(&end, NULL);

		double time_taken; 
		time_taken = (end.tv_sec - start.tv_sec) * 1e6; 
		time_taken = (time_taken + (end.tv_usec -  
		                          start.tv_usec)) * 1e-6;
		printf("time taken %7f \n", time_taken);
	}
	return err;
}


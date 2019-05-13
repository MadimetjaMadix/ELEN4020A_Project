#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


int getElementLocation2D(int index[2], int N)
{
	int row    = index[0];
	int column = index[1];

	return ((row*N)+column);
}


int main(int argc, char** argv)
{
	char *input_filename = argv[1];
	char *output_file_name = argv[2]; 

	int rank, num_procs, N, *tmp_buffer, err = 0, errs = 0;
	double startTime, endTime; 

	MPI_Win win;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	MPI_File fh;
	MPI_Status status;
	
	startTime = MPI_Wtime();
	// Open input file for reading:
	err = MPI_File_open(MPI_COMM_WORLD, input_filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	tmp_buffer = (int *)malloc(sizeof(int) );
	MPI_File_read(fh, tmp_buffer, 1, MPI_INT, &status ); // Matrix size
	N = tmp_buffer[0];
	free(tmp_buffer);
	
	int m  = (int)(N/num_procs);
	
	int *localbuffer  = (int *)malloc(N*m*sizeof(int));
	int *sharedbuffer = (int *)malloc(N*m*sizeof(int));
	
	// Create window:
	MPI_Win_create(sharedbuffer, N*m, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);
	
	
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
	displace = ((rank*block_length))*sizeof(int); // start of the view for each processor
	MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );

	err = MPI_File_write(fh, localbuffer, block_length, MPI_INT, &status);
	MPI_File_close(&fh);
	if (err) { errs++; }
	
	endTime = MPI_Wtime();
	MPI_Barrier(MPI_COMM_WORLD);
	if(rank == 0) printf("===================== Transposition Results ===================== \n");
	printf("Process %d: elapsed time: %f \n", rank, endTime - startTime);
	
	// Check Transpose:
	if(rank == 0)
	{
		// Check transpose:
		int* originalMatrix = (int*)malloc(N*N*sizeof(int));
		err = MPI_File_open(MPI_COMM_SELF, input_filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
		displace = 4;
		MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
		MPI_File_read(fh, originalMatrix, N*N, MPI_INT, &status);
		MPI_File_close(&fh);
		
		
		int* transposedMatrix = (int*)malloc(N*N*sizeof(int));
		err = MPI_File_open(MPI_COMM_SELF, output_file_name, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
		MPI_File_read(fh, transposedMatrix, N*N, MPI_INT, &status);
		MPI_File_close(&fh);
		
		// Check elements:
		for (int i = 0; i < N; i++)
			for (int j = 0; j < N; j++)
			{
				int index[2] = {i,j};
				int offsetTransposed = getElementLocation2D(index, N);
				index[0] = j;
				index[1] = i;
				int offsetOriginal   = getElementLocation2D(index, N);
				if (*(transposedMatrix+offsetTransposed) != *(originalMatrix+offsetOriginal))
				{
					printf ("process %d found transposed[%d][%d] = %d, but %d was expected\n",
					rank, i, j, *(transposedMatrix+offsetTransposed), *(originalMatrix+offsetOriginal));
					MPI_Abort (MPI_COMM_WORLD, 1);
					MPI_File_close(&fh);
				
					// Free memory
					free(transposedMatrix);
					free(originalMatrix);
					MPI_Win_free(&win);
					free(sharedbuffer);
					free(localbuffer);
					MPI_Finalize ();
					return 1;
				}
			}//for
	 
		printf("Transpose is looking good!\n");
		
		free(transposedMatrix);
		free(originalMatrix);
	}//if
	
	
	MPI_Win_free(&win);
	free(sharedbuffer);
	free(localbuffer);
	MPI_Finalize();
	return err;
}


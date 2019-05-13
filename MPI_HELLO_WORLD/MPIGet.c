/*
// Copyright 2003-2014 Intel Corporation. All Rights Reserved.
// 
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors.     Title to the Material remains with Intel Corporation
// or its suppliers and licensors.     The Material is protected by worldwide
// copyright and trade secret laws and treaty provisions.     No part of the
// Material may be used, copied, reproduced, modified, published, uploaded,
// posted, transmitted, distributed, or disclosed in any way without Intel's
// prior express written permission.
// 
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel
// or otherwise.     Any license under such intellectual property rights must
// be express and approved by Intel in writing.
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define NUM_ELEMENT 4

int main(int argc, char** argv)
{
	char *input_filename = argv[1];
	char *output_file_name = argv[2]; 

	int i, id, num_procs, len, N, *tmp_buffer, err = 0;

	char name[MPI_MAX_PROCESSOR_NAME];
	MPI_Win win;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
	MPI_Get_processor_name(name, &len);

	MPI_File fh;
	MPI_Status status;

	printf("Rank %d running on %s\n", id, name);

	err = MPI_File_open(MPI_COMM_WORLD, input_filename, MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
	tmp_buffer = (int *)malloc(sizeof(int) );
	MPI_File_read(fh, tmp_buffer, 1, MPI_INT, &status ); // Matrix size
	N = tmp_buffer[0];
	int m = (int)(N/num_procs);
	
	
	int* sharedbuffer = (int *)malloc(N*sizeof(int));
	MPI_Win_create(sharedbuffer, N, sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &win);


	if (err)
	{
		MPI_Abort(MPI_COMM_WORLD, 911);
	}


	int *localbuffer = (int *)malloc(N*sizeof(int));
	for (i = 0; i < N; i++)
	{
		sharedbuffer[i] = 10*id + i;
		localbuffer[i] = 0;
	}

	printf("Rank %d sets data in the shared memory:", id);
	
	for (int i = 0; i < N*m; i++)
	{
		if(i%N == 0 && i!=0 ) printf("\n");
		printf("%4d ", sharedbuffer[i]);
		
	}
	printf("\n");

	MPI_Win_fence(0, win);

	if (id != 0)
		     MPI_Get(localbuffer, N, MPI_INT, id-1, 0, N, MPI_INT, win);
	else
		     MPI_Get(localbuffer, N, MPI_INT, num_procs-1, 0, N, MPI_INT, win);

	MPI_Win_fence(0, win);

	printf("Rank %d gets data from the shared memory:", id);

	for (i = 0; i < N; i++)
		     printf(" %02d", localbuffer[i]);

	printf("\n");

	MPI_Win_fence(0, win);

	if (id < num_procs-1)
		     MPI_Put(localbuffer, N, MPI_INT, id+1, 0, N, MPI_INT, win);
	else
		     MPI_Put(localbuffer, N, MPI_INT, 0, 0, N, MPI_INT, win);

	MPI_Win_fence(0, win);

	printf("Rank %d has new data in the shared memory:", id);

	for (i = 0; i < N; i++)
		     printf(" %02d", sharedbuffer[i]);

	printf("\n");

	MPI_Win_free(&win);
	free(sharedbuffer);
	free(localbuffer);
	free(tmp_buffer);
	MPI_Finalize();
	return err;
}


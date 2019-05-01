#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
		int my_rank, p; // process rank and number of processes
		int source, dest; // rank of sender and receiving process
		int tag = 0; // tag for messages
		char mesg[100]; // storage for message
		MPI_Status status; // stores status for MPI_Recv statements
		MPI_Init(&argc, &argv);
		MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
		MPI_Comm_size(MPI_COMM_WORLD, &p);
		if (my_rank!=0)
		{
			sprintf(mesg, "Greetings from %d!", my_rank);
			// stores into character array
			dest = 0; // sets destination for MPI_Send to process 0
			MPI_Send(mesg, strlen(mesg)+1, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
			// sends string to process 0
		}
		else {
			for(source = 1; source < p; source++){
				MPI_Recv(mesg, 100, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
				// recv from each process
				printf("%s\n", mesg); // prints out greeting to screen
			}
			MPI_Finalize(); // shuts down MPI
		}
	
	return 0;
}

#include <stdio.h>
#include <mpi.h>

int main(int argc, char **argv)
{ 

	int rank, i; 
	char a[10]; 
	char mesg[100];
	char b[10];
	
	
	MPI_Offset n = 10; 
	MPI_File fh; 
	MPI_Status status; 

	MPI_Init(&argc, &argv); 

	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	if(rank!=0)
	{
		sprintf(mesg, "\nGreetings from %d!",rank);
		printf("%s\n", mesg);
	}
	for (i=0; i<10; i++)
		a[i] = (char)( '0' + rank);  // e.g. on processor 3 creates a[0:9]=’3333333333’ 

	 
	MPI_Offset displace = rank*n*sizeof(char); // start of the view for each processor 
	
	
	MPI_File_open (MPI_COMM_WORLD, "data.txt", MPI_MODE_CREATE|MPI_MODE_RDWR , MPI_INFO_NULL, &fh);
	
	MPI_File_set_view (fh , displace , MPI_CHAR, MPI_CHAR, "native" ,MPI_INFO_NULL);
	// note that etype and filetype are the same 

	MPI_File_write(fh, a, n, MPI_CHAR, &status);
	
	MPI_File_set_view (fh , displace , MPI_CHAR, MPI_CHAR, "native" ,MPI_INFO_NULL);
	// note that etype and filetype are the same 
	
	MPI_File_read (fh, b, n, MPI_CHAR, &status);
	printf("printing from %d\n, we have : %s\n",rank,b);
	MPI_File_close(&fh ) ; 

	MPI_Finalize ( ) ; 

	return 0;
	
}

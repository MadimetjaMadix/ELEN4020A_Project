#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

/* Test set_view with DISPLACEMENT_CURRENT */
int main( int argc, char *argv[] )
{
    int errs = 0, err;
    int size, rank, *buf;
    MPI_Offset offset;
    
    
    MPI_File fh;
    MPI_Comm comm;
    MPI_Status status;

    MPI_Init( &argc, &argv );
    
    /* This test reads a header then sets the view to every "size" int,
        using set view and current displacement. The file is first written
        using a combination of collective and ordered writes */

    comm = MPI_COMM_WORLD;
    err = MPI_File_open( comm, "test.txt", MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh );
    
    //
    
    //
    
    if (err)
    {
        MPI_Abort(MPI_COMM_WORLD, 911);
    }
    MPI_Comm_size( comm, &size );
    MPI_Comm_rank( comm, &rank );
    
    MPI_Offset n = 1; 
    MPI_Offset displace = (rank+1)*n*sizeof(int); // start of the view for each processor
    
    buf = (int *)malloc( size * sizeof(int) );
    
    buf[0] = size;
    err = MPI_File_write_all(fh, buf, 1, MPI_INT, &status);
    
    MPI_File_set_view(fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
    buf[0] = rank;
    err = MPI_File_write( fh, buf, n, MPI_INT, &status );
    if (err) { errs++; }
   /* if (err) { errs++; }
    err = MPI_File_get_position( fh, &offset );
    if (err) { errs++; }
    err = MPI_File_seek_shared( fh, offset, MPI_SEEK_SET );
    if (err) { errs++; }
    */
	err = MPI_File_close( &fh );
    if (err) { errs++; }
    
    /* Reopen the file as sequential */
    err = MPI_File_open( comm, "test.txt", MPI_MODE_RDONLY, MPI_INFO_NULL, &fh );
    if (err)
    {
        MPI_Abort(MPI_COMM_WORLD, 911);
    }

    if (rank == 0) {
        err = MPI_File_read_shared( fh, buf, n, MPI_INT, &status );
        if (err) { errs++; }
        if (buf[0] != size) { 
            errs++;
            fprintf(stderr, "Unexpected value for the header = %d, should be %d\n", buf[0], size ); fflush(stderr);
        }
    }
    
	 
    /* All processes must provide the same file view for MODE_SEQUENTIAL */
    err = MPI_File_set_view( fh, displace, MPI_INT, MPI_INT, "native", MPI_INFO_NULL );
    if (err) { errs++; }
    buf[0] = -1;
    err = MPI_File_read( fh, buf, n, MPI_INT, &status );
    if (err) { errs++; }
    if (buf[0] != rank) {
        errs++;
        fprintf( stderr, "%d: buf[0] = %d\n", rank, buf[0] ); fflush(stderr);
    }
    printf("Printing from rank %d size %d \n", rank, buf[0]);
    free( buf );
    err = MPI_File_close( &fh );
    if (err) { errs++; }
    MPI_Finalize();
    return errs;
}
#include <stdio.h>
#include "mpi.h"


int main( argc, argv )
int  argc;
char **argv;
{
    int rank, size;
    MPI_Init( &argc, &argv );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    //code here

    // get diameter

    // loop i< diameter
    //      1. send to all
    //      2. receive from all
    //      3. compare and update

    // print max
    printf( "Hello world from process %d of %d\n", rank, size );
    MPI_Finalize();
    return 0;
}
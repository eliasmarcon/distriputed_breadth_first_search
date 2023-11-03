#include <stdio.h>
#include "mpi.h"

void electLeader(int rank, int size, int diameter, int* leaderUID){
    int i;
    int max = rank;
    int maxRank = rank;
    int *buf = (int *)malloc(sizeof(int)*size);
    for(i=0; i<diameter; i++){
        MPI_Allgather(&max, 1, MPI_INT, buf, 1, MPI_INT, MPI_COMM_WORLD);
        int j;
        for(j=0; j<size; j++){
            if(buf[j] > max){
                max = buf[j];
                maxRank = j;
            }
        }
    }
    if(rank == maxRank){
        printf("I am the leader\n");
    }
}



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
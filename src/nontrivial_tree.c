#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>

int main(argc, argv)
int argc;
char **argv;
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Assuming a non-trivial graph with nodes represented by processes
    int num_nodes = size;
    int *graph = (int *)malloc(num_nodes * num_nodes * sizeof(int));

    // Initialize the graph
    for (int i = 0; i < num_nodes; i++)

    {
        for (int j = 0; j < num_nodes; j++)

        {
            graph[i * num_nodes + j] = 0;
        }
    }

    // Define edges in the graph (non-trivial example)
    if (rank == 0)

    {
        for (int i = 0; i < num_nodes; ++i)
        {
            graph[i * num_nodes + (i + 1)] = 1; // Node 0 connected to Node 1
        }
    }

    // Broadcast the graph to all processes
    MPI_Bcast(graph, num_nodes * num_nodes, MPI_INT, 0, MPI_COMM_WORLD);

    // Print the graph (each process prints its part)
    printf("Graph representation in process %d:\n", rank);
    for (int i = 0; i < num_nodes; i++)

    {
        for (int j = 0; j < num_nodes; j++)

        {
            printf("%d ", graph[i * num_nodes + j]);
        }
        printf("\n");
    }

    MPI_Finalize();
    free(graph);
    return 0;
}
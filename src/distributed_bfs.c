#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int vertices = 8; // Number of vertices in the graph
    int edges = 9;    // Number of edges in the graph

    // Create a graph topology
    int *index = (int *)malloc(vertices * sizeof(int));
    int *edges_array = (int *)malloc(edges * sizeof(int));
    if (rank == 0)
    {
        for (int i = 0; i < vertices; i++)
        {
            index[i] = i;
        }
    }

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, vertices, index, edges_array, 0, &graph_comm);

    // ---

    //  0
    //  / \
//1---2

    int n = 3;                         // Number of nodes
    int index[3] = {2, 4, 6};          // Cumulative degree of nodes
    int edges[6] = {1, 2, 0, 2, 0, 1}; // Edges in the graph

    //   0
    //  / \
// 1   2

    int n = 3;                   // Number of nodes
    int index[3] = {2, 3, 4};    // Cumulative degree of nodes
    int edges[4] = {1, 2, 0, 0}; // Edges in the graph

    // Create a graph communicator based on the topology
    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, n, index, edges, 0, &graph_comm);

    // ---

    // Create the graph structure
    // Broadcast the graph structure
    // MPI_Bcast(index, vertices, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Bcast(edges_array, edges, MPI_INT, 0, MPI_COMM_WORLD);
    // MPI_Bcast(&graph_comm, 1, MPI_GRAPH, 0, MPI_COMM_WORLD);

    // Initialize BFS data structures
    int *visited = (int *)malloc(vertices * sizeof(int));
    for (int i = 0; i < vertices; i++)
    {
        visited[i] = 0;
    }

    // Perform BFS from source vertices
    for (int s = 0; s < 1; s++)
    {
        if (rank == 0)
        {
            visited[s] = 1;
        }
        MPI_Bcast(visited, vertices, MPI_INT, 0, graph_comm);

        for (int level = 0; level < vertices; level++)
        {
            for (int i = 0; i < edges; i++)
            {
                int neighbor = edges_array[i];
                if (visited[neighbor] == 0)
                {
                    visited[neighbor] = 1;
                }
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    // Print the visited vertices
    if (rank == 0)
    {
        printf("Visited vertices: ");
        for (int i = 0; i < vertices; i++)
        {
            if (visited[i] == 1)
            {
                printf("%d ", i);
            }
        }
        printf("\n");
    }

    free(index);
    free(edges_array);
    free(visited);

    MPI_Finalize();

    return 0;
}

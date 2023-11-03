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
    int* index = malloc(size * sizeof(int));  // Array to store the index of each node
    int* edges = malloc(2 * size * sizeof(int));  // Array to store the edges

    // Define the graph structure here. For example, a ring topology:
    for (int i = 0; i < size; i++) {
        index[i] = i * 2;
        edges[i * 2] = (i - 1 + size) % size;  // Previous node
        edges[i * 2 + 1] = (i + 1) % size;      // Next node
    }

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, size, index, edges, 0, &graph_comm);

    int graph_rank, graph_size;
    MPI_Comm_rank(graph_comm, &graph_rank);
    MPI_Comm_size(graph_comm, &graph_size);

    // Perform communication
    int message = rank;
    for (int i = 0; i < size; i++) {
        int dest;
        MPI_Cart_rank(graph_comm, &i, &dest);
        if (rank == i) {
            printf("Node %d sending a message to Node %d\n", rank, dest);
            MPI_Send(&message, 1, MPI_INT, dest, 0, graph_comm);
        }
        if (rank == dest) {
            MPI_Recv(&message, 1, MPI_INT, i, 0, graph_comm, MPI_STATUS_IGNORE);
            printf("Node %d received a message from Node %d: %d\n", rank, i, message);
        }
    }

    

    // print max
    printf( "Hello world from process %d of %d\n", rank, size );
    MPI_Finalize();
    return 0;
}




/*
int source_node = 0;  // Choose a source node (can be any node)

// Define the adjacency matrix for a directed graph (example)
int adjacency_matrix[MAX_NODES * MAX_NODES] = {
    0, 1, 0, 0, 0, 0,
    0, 0, 1, 1, 0, 0,
    0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0
};

int bfs_tree[MAX_NODES];

// Define the connectivity information for the graph using index and edges arrays
int index[MAX_NODES];
int edges[MAX_NODES * 2];

for (int i = 0; i < MAX_NODES; i++) {
    index[i] = i * 2;
    for (int j = 0; j < MAX_NODES; j++) {
        if (adjacency_matrix[i * MAX_NODES + j]) {
            edges[index[i]++] = j;
        }
    }
}

// Create a communicator for the graph using MPI_Graph_create
MPI_Comm comm_graph;
int reorder = 0;  // Set to 0 to keep the original ranks
MPI_Graph_create(MPI_COMM_WORLD, MAX_NODES, index, edges, reorder, &comm_graph);

*/
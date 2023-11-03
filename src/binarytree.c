#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int* index = malloc(size * sizeof(int));  // Array to store the index of each node
    int* edges = malloc(2 * size * sizeof(int));  // Array to store the edges

    // Define a simple binary tree structure:
    if (rank == 0) {
        // Root node
        index[0] = 0;
        edges[0] = 1;  // Child 1
        edges[1] = 2;  // Child 2
    } else if (rank % 2 == 1) {
        // Odd-ranked nodes (left child)
        index[rank] = rank;
        edges[rank] = (rank - 1) / 2;  // Parent
    } else {
        // Even-ranked nodes (right child)
        index[rank] = rank;
        edges[rank] = (rank - 2) / 2;  // Parent
    }

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, size, index, edges, 0, &graph_comm);

    int graph_rank, graph_size;
    MPI_Comm_rank(graph_comm, &graph_rank);
    MPI_Comm_size(graph_comm, &graph_size);

    // Perform communication
    int message = rank;
    int parent, child1, child2;
    MPI_Cart_rank(graph_comm, &rank, &parent);

    if (rank * 2 + 1 < size) {
        child1 = rank * 2 + 1;
        MPI_Send(&message, 1, MPI_INT, child1, 0, graph_comm);
        printf("Node %d sending a message to Node %d (child1)\n", rank, child1);
    }

    if (rank * 2 + 2 < size) {
        child2 = rank * 2 + 2;
        MPI_Send(&message, 1, MPI_INT, child2, 0, graph_comm);
        printf("Node %d sending a message to Node %d (child2)\n", rank, child2);
    }

    if (rank != 0) {
        MPI_Recv(&message, 1, MPI_INT, parent, 0, graph_comm, MPI_STATUS_IGNORE);
        printf("Node %d received a message from Node %d (parent): %d\n", rank, parent, message);
    }

    MPI_Finalize();

    free(index);
    free(edges);

    return 0;
}

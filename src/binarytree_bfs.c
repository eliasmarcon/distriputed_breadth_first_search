#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Function to perform Breadth-First Search on the tree
void BFS(MPI_Comm graph_comm, int root) {
    int rank, size;
    MPI_Comm_rank(graph_comm, &rank);
    MPI_Comm_size(graph_comm, &size);

    int* visited = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        visited[i] = 0;
    }

    int* queue = (int*)malloc(size * sizeof(int));
    int queue_size = 0;

    if (rank == root) {
        visited[rank] = 1;
        printf("Node %d visited\n", rank);
        queue[queue_size++] = rank;
    }

    int message;
    while (queue_size > 0) {
        int current_node = queue[0];
        for (int i = 0; i < queue_size - 1; i++) {
            queue[i] = queue[i + 1];
        }
        queue_size--;

        int child1, child2;
        int neighbors[2];
        if (rank * 2 + 1 < size) {
            child1 = rank * 2 + 1;
            neighbors[0] = child1;
        }
        if (rank * 2 + 2 < size) {
            child2 = rank * 2 + 2;
            neighbors[1] = child2;
        }

        for (int i = 0; i < 2; i++) {
            if (neighbors[i] >= size) {
                continue;  // Skip invalid neighbors
            }

            if (!visited[neighbors[i]]) {
                MPI_Send(&message, 1, MPI_INT, neighbors[i], 0, graph_comm);
                MPI_Recv(&message, 1, MPI_INT, neighbors[i], 0, graph_comm, MPI_STATUS_IGNORE);
                visited[neighbors[i]] = 1;
                printf("Node %d visited\n", neighbors[i]);
                queue[queue_size++] = neighbors[i];
            }
        }
    }

    free(visited);
    free(queue);
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int* index = (int*)malloc(size * sizeof(int));  // Array to store the index of each node
    int* edges = (int*)malloc(2 * size * sizeof(int));  // Array to store the edges

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

    // Perform BFS starting from the root (node 0)
    BFS(graph_comm, 0);

    MPI_Finalize();

    free(index);
    free(edges);

    return 0;
}

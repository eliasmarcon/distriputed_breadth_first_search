#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

struct Graph
{
    int nodeAmount;
    int *index;
    int *edges_array;
};

struct Graph *getGraph(int nodes)
{

    struct Graph *graph = (int *)malloc(sizeof(struct Graph));
    if (nodes != 5 && nodes != 10 && nodes != 15)
    {
        // choose nearest graph size
        if (nodes < 5)
        {
            nodes = 5;
        }
        else if (nodes < 10)
        {
            nodes = 10;
        }
        else
        {
            nodes = 15;
        }
    }
    graph->nodeAmount = nodes;

    // Check if memory allocation was successful
    if (graph->index == NULL || graph->edges_array == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Populate the index and edges_array for the 5-node graph
    // Allocate memory for index array
    graph->index = (int *)malloc(nodes * sizeof(int));
    if (nodes == 5)
    {
        // Allocate memory for index and edges_array
        graph->edges_array = (int *)malloc(10 * sizeof(int)); // 10 edges for the 5-node graph
        graph->index[0] = 3;
        graph->index[1] = 5;
        graph->index[2] = 7;
        graph->index[3] = 9;
        graph->index[4] = 10;

        // Node 0
        graph->edges_array[0] = 1;
        graph->edges_array[1] = 2;
        graph->edges_array[2] = 3;

        // Node 1
        graph->edges_array[3] = 0;
        graph->edges_array[4] = 4;

        // Node 2
        graph->edges_array[5] = 0;

        // Node 3
        graph->edges_array[6] = 0;
        graph->edges_array[7] = 4;

        // Node 4
        graph->edges_array[8] = 1;
        graph->edges_array[9] = 3;
    }
    else if (nodes == 10)
    {
        // Allocate memory for index and edges_array
        graph->edges_array = (int *)malloc(20 * sizeof(int));                         // 10 edges for the 5-node graph
        int index[10] = {2, 4, 6, 8, 10, 12, 14, 16, 18, 20};                         // Cumulative degree of nodes
        int edges[20] = {1, 2, 0, 3, 1, 4, 2, 5, 3, 6, 4, 7, 5, 8, 6, 9, 7, 0, 8, 1}; // Edges in the graph
        memcpy(graph->index, index, sizeof(index));
        memcpy(graph->edges_array, edges, sizeof(edges));
    }
    else if (nodes == 15)
    {
        graph->edges_array = (int *)malloc(34 * sizeof(int));                      // 10 edges for the 5-node graph
        int index[15] = {3, 5, 8, 10, 12, 14, 16, 19, 22, 24, 26, 28, 30, 32, 34}; // Cumulative degree of nodes
        int edges[34] = {
            1, 2, 3,   // Node 0
            0, 4,      // Node 1
            0, 3, 5,   // Node 2
            0, 2,      // Node 3
            1, 5,      // Node 4
            2, 4, 6,   // Node 5
            5, 7,      // Node 6
            6, 8, 9,   // Node 7
            7, 10, 11, // Node 8
            7, 12,     // Node 9
            8, 13,     // Node 10
            8, 14,     // Node 11
            9, 13,     // Node 12
            10, 12,    // Node 13
            11         // Node 14
        };
        memcpy(graph->index, index, sizeof(index));
        memcpy(graph->edges_array, edges, sizeof(edges));
    }

    return graph;
}

int *mergeLists(int *list1, int *list2, int size1, int size2)
{
    int *mergedList = calloc(size1 + size2, sizeof(int));
    memcpy(mergedList, list1, size1 * sizeof(int));
    memcpy(mergedList + size1, list2, size2 * sizeof(int));
    free(list1);
    // list2 is implicitly allocated by MPI
    MPI_Free_mem(list2);
    return mergedList;
}

void notifyNeighbours(int *neighbors, int n_neighbors, int *visited, MPI_Comm graph_comm)
{
    // TODO: Distribute depth and max depth to neighbors
    char comp[6] = {'A', 'C', 'K', ':', '0', '\0'};
    for (int i = 1; i < n_neighbors + 1; i++)
    {
        MPI_send(n_neighbors + 1, 1, MPI_INT, neighbors[i], 0, graph_comm);
        // recieve ACK:<number> || DEC:0
        comp[4] = (char)(n_neighbors + 1);
        char recieveBuffer[6];
        MPI_Recv(&recieveBuffer, 6, MPI_CHAR, neighbors[i], 0, graph_comm, MPI_STATUS_IGNORE);
        // compare strings
        if (strcmp(&recieveBuffer, &comp) == 0)
        {
            printf("ACK recieved from %d\n", neighbors[i]);
        }
        else
        {
            printf("ERROR: ACK not recieved from %d\n", neighbors[i]);
        }
        MPI_send(&visited, n_neighbors + 1, MPI_INT, neighbors[i], 0, graph_comm);
    }
}

int receiveFromParent(int *neighbors, int n_neighbors, MPI_Comm graph_comm, int rank, int *visited)
{
    int parentVisitedSize = 0;
    MPI_Status status;

    MPI_Request request[n_neighbors + 1];

    // recieve neighbour array sizefrom parent
    MPI_recv(&parentVisitedSize, 1, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);

    // send ack to parent
    char ack[6] = {'A', 'C', 'K', ':', '0', '\0'};
    ack[4] = (char)parentVisitedSize;
    int parentRank = status.MPI_SOURCE;
    MPI_send(&ack, 6, MPI_CHAR, parentRank, 0, graph_comm);

    // visited = calloc(parentVisitedSize, sizeof(int));

    // recieve neighbour array from parent
    MPI_recv(visited, parentVisitedSize, MPI_INT, parentRank, 0, graph_comm, MPI_STATUS_IGNORE);
    return parentRank == rank ? -1 : parentRank;
}

int *distributedBFS(MPI_Comm *graph_comm, int rank, int size, int bfsdepth)
{
    // -------------  Prepare Structures --------------------------
    MPI_Request request;

    // Get the number of neighbors for the current node
    int n_neighbors;
    MPI_Graph_neighbors_count(graph_comm, rank, &n_neighbors);

    // Get the neighbors
    int *neighbors;
    MPI_Graph_neighbors(graph_comm, rank, n_neighbors, neighbors);
    int *visited = calloc(n_neighbors + 1, sizeof(int));
    // mark ourselves as visited
    visited[0] = rank;
    int *recData = calloc(n_neighbors + 1, sizeof(int));

    for (int i = 1; i < n_neighbors + 1; i++)
    {
        // mark neighbors as visited
        visited[i] = neighbors[i];
    }
    int parent;
    if (rank == 0)
    {
        notifyNeighbours(neighbors, n_neighbors, visited, graph_comm);
        parent = -1;
    }
    else
    {
        int *visitedParent;
        parent = receiveFromParent(neighbors, n_neighbors, graph_comm, rank, visitedParent);
        // merge lists
        visited = mergeLists(visited, visitedParent, n_neighbors + 1, sizeof(visitedParent) / sizeof(int));
        // TODO: only use BFS if depth is not reached
        notifyNeighbours(neighbors, n_neighbors, visited, graph_comm);
    }
    // --------------------- start recieving ------------------------

    // TODO: Notify Structure (see flow chart)
    //  Don't forget to free the memory allocated for 'neighbors' after you're done.
    // TODO: 8. Recieve from children(see flow chart)
    MPI_Free_mem(neighbors);
    free(recData);
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int bfsdepth;
    if (argc > 1)
    {
        bfsdepth = atoi(argv[1]);
    }
    else
    {
        bfsdepth = 2;
    }
    const struct Graph *graph = getGraph(size);

    // Create a graph communicator based on the topology
    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, graph->nodeAmount, *(graph->index), *(graph->edges_array), 0, &graph_comm);

    // ---
    // Perform BFS
    int *result = distributedBFS(&graph_comm, rank, graph->nodeAmount, bfsdepth);

    // TODO: Process results (extract diameter, etc.)
    //  Cleanup
    MPI_Comm_free(&graph_comm);
    free(graph->index);
    free(graph->edges_array);
    free(graph);
    MPI_Finalize();

    return 0;
}

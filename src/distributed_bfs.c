#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

struct ArrayList
{
    int size;
    int *list;
};

struct Graph
{
    int nodeAmount;
    int *index;
    int *edges_array;
};

struct Graph *getGraph(int nodes)
{

    struct Graph *graph = malloc(sizeof(struct Graph));
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

    // Populate the index and edges_array for the 5-node graph
    // Allocate memory for index array
    graph->index = calloc(nodes, sizeof(int));
    if (nodes == 5)
    {

        // // Allocate memory for index and edges_array
        graph->edges_array = (int *)malloc(8 * sizeof(int)); // 10 edges for the 5-node graph
        int index[5] = {1, 3, 5, 7, 8};
        int edges[8] = {1, 0, 2, 1, 3, 2, 4, 3};

        memcpy(graph->index, &index, sizeof(index));
        memcpy(graph->edges_array, &edges, sizeof(edges));
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

    // Check if memory allocation was successful
    if (graph->index == NULL || graph->edges_array == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    return graph;
}

struct ArrayList *mergeLists(struct ArrayList *list1, struct ArrayList *list2)
{
    int size1 = list1->size;
    int size2 = list2->size;
    int newSize = size1 + size2;
    printf("Merging lists of size %d and %d\n", size1, size2);
    printf("New size: %d\n", newSize);
    int *mergedList = malloc(newSize * sizeof(int));
    memcpy(mergedList, list1->list, size1 * sizeof(int));
    memcpy(mergedList + size1, list2->list, size2 * sizeof(int));
    free(list2);
    free(list1);
    struct ArrayList *new = malloc(sizeof(struct ArrayList));
    new->list = mergedList;
    new->size = newSize;
    return new;
}

// char *toString(struct ArrayList *list)
// {
//     char *str = malloc(sizeof(char) * list->size * 2);
//     for (int i = 0; i < list->size; i++)
//     {
//         str[i * 2] = (char)list->list[i];
//         str[i * 2 + 1] = ',';
//     }
//     str[list->size * 2 - 1] = '\0';
//     return str;
// }

char *toString(struct ArrayList *list)
{
    // Initial estimate for string size
    int estimatedSize = 1; // Start with 1 for the null terminator
    for (int i = 0; i < list->size; i++)
    {
        estimatedSize += snprintf(NULL, 0, "%d,", list->list[i]);
    }

    char *str = malloc(sizeof(char) * estimatedSize);
    if (!str)
    {
        return NULL; // Return NULL if memory allocation fails
    }

    char *current = str;
    for (int i = 0; i < list->size; i++)
    {
        current += sprintf(current, "%d,", list->list[i]);
    }

    if (list->size > 0)
    {
        current[-1] = '\0'; // Replace the last comma with a null terminator
    }
    else
    {
        *str = '\0';
    }

    return str;
}

int inList(int value, struct ArrayList *list)
{
    for (int i = 0; i < list->size; i++)
        if (list->list[i] == value)
            return 1;
    return 0;
}

void notifyNeighbours(struct ArrayList *neighbors, struct ArrayList *visited, MPI_Comm *graph_comm, int *depth)
{
    char comp[6];
    for (int i = 0; i < neighbors->size; i++)
    {
        if (inList(neighbors->list[i], visited))
        {
            continue;
        }
        printf("Notifying %d\n", (neighbors->list[i]));
        MPI_Send(&(visited->size), 1, MPI_INT, neighbors->list[i], 0, *graph_comm);
        // recieve ACK:<number> || DEC:0
        sprintf(comp, "ACK:%d", visited->size);
        char recieveBuffer[6];
        MPI_Recv(&recieveBuffer, 6, MPI_CHAR, neighbors->list[i], 0, *graph_comm, MPI_STATUS_IGNORE);
        // compare strings
        if (strcmp((const char *)&recieveBuffer, (const char *)&comp) == 0)
        {
            printf("ACK recieved from %d\n", neighbors->list[i]);
        }
        else
        {
            printf("ERROR: ACK not recieved from %d\n", neighbors->list[i]);
            printf("Recieved: %s\n", recieveBuffer);
        }
        char *str = toString(visited);
        if (*depth > 1)
            printf("send visited: %s, size: %d\n", str, visited->size);
        free(str);
        MPI_Send(visited->list, visited->size, MPI_INT, neighbors->list[i], 0, *graph_comm);
        // Distribute depth to neighbors
        int newdepth = *depth + 1;
        MPI_Send(&newdepth, 1, MPI_INT, neighbors->list[i], 0, *graph_comm);
    }
}

int receiveFromParent(struct ArrayList *neighbors, MPI_Comm *graph_comm, int rank, struct ArrayList *visited, int *depth)
{
    MPI_Status status;

    // recieve neighbour array sizefrom parent
    MPI_Recv(&(visited->size), 1, MPI_INT, MPI_ANY_SOURCE, 0, *graph_comm, &status);
    // printf("Rank: %d, Recieved size: %d\n", rank, visited->size);

    char ack[6];
    sprintf(ack, "ACK:%d", visited->size);
    int parentRank = status.MPI_SOURCE;
    // printf("Rank: %d, Sending ACK: '%s' to %d\n", rank, ack, parentRank);
    MPI_Send(&ack, 6, MPI_CHAR, parentRank, 0, *graph_comm);
    // allocate memory for visited array + 1 (for current node)
    visited->list = calloc(visited->size + 1, sizeof(int));

    // recieve visited array from parent
    printf("Rank: %d, Reicieving size: %d\n", rank, visited->size);
    MPI_Recv(visited->list, visited->size, MPI_INT, parentRank, 0, *graph_comm, MPI_STATUS_IGNORE);
    // incerease size by 1 -> add current node
    visited->size++;
    printf("Rank: %d, Recieved visited: %s\n", rank, toString(visited));
    // recieve depth from parent
    MPI_Recv(depth, 1, MPI_INT, parentRank, 0, *graph_comm, MPI_STATUS_IGNORE);
    // printf("Rank: %d, Recieved depth: %d\n", rank, *depth);
    return parentRank == rank ? -1 : parentRank;
}

int *distributedBFS(MPI_Comm *graph_comm, int rank, int size, int bfsdepth)
{
    // -------------  Prepare Structures --------------------------
    MPI_Request request;
    int depth = 0;

    // Get the number of neighbors for the current node
    int n_neighbors;
    MPI_Graph_neighbors_count(*graph_comm, rank, &n_neighbors);

    // Get the neighbors
    int *neighbors = malloc(n_neighbors * sizeof(int));
    MPI_Graph_neighbors(*graph_comm, rank, n_neighbors, neighbors);

    struct ArrayList *neighborsList = malloc(sizeof(struct ArrayList));
    neighborsList->size = n_neighbors;
    neighborsList->list = neighbors;
    // Calculate visited members
    struct ArrayList *visited = malloc(sizeof(struct ArrayList));
    visited->size = 1;
    visited->list = calloc(1, sizeof(int));
    // mark ourselves as visited
    visited->list[0] = rank;
    // mark neighbors as visited
    // printf("Rank: %d, n_neighbors: %d\n", rank, n_neighbors);
    int parent;
    if (rank == 0)
    {
        notifyNeighbours(neighborsList, visited, graph_comm, &depth);
        parent = -1;
    }
    else
    {
        // memcpy(visited->list + 1, neighbors, n_neighbors * sizeof(int));
        struct ArrayList *visitedParent = malloc(sizeof(struct ArrayList));
        parent = receiveFromParent(neighborsList, graph_comm, rank, visitedParent, &depth);
        // printf("Rank: %d has Parent: %d\n", rank, parent);
        //  merge lists
        // visited = mergeLists(visitedParent, visited);
        free(visited);
        visited = visitedParent;
        visited->list[visited->size - 1] = rank;
        // end merge lists
        char *str = toString(visited);
        printf("Parent: %d, Rank: %d, depth: %d, visited: %s\n", parent, rank, depth, str);
        free(str);
        // only use BFS if depth is not reached
        if (depth < bfsdepth)
        {
            notifyNeighbours(neighborsList, visited, graph_comm, &depth);
        }
        else
        {
            printf("ERROR: Not implemented yet\n");
            // notifyNeighbours(neighborsList, visited, graph_comm, &depth);
            //  TODO: do DFS
        }
    }
    // --------------------- start recieving ------------------------

    // TODO: Notify Structure (see flow chart)
    //  Don't forget to free the memory allocated for 'neighbors' after you're done.
    // TODO: 8. Recieve from children(see flow chart)
    free(neighbors);
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
        if (rank == 0)
            printf("Setting depth to %d\n", bfsdepth);
    }
    else
    {
        bfsdepth = 2;
    }
    struct Graph *graph = getGraph(size);

    // Create a graph communicator based on the topology
    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, graph->nodeAmount, (graph->index), (graph->edges_array), 0, &graph_comm);

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

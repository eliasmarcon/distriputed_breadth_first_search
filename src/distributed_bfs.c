#include "Graph.h"
#include "ArrayList.h"
#include <mpi.h>

void notifyNeighbour(int neighborRank, struct ArrayList *visited, MPI_Comm *graph_comm, int *depth);
void notifyNeighbours(struct ArrayList *neighbors, struct ArrayList *visited, MPI_Comm *graph_comm, int *depth);
void notifyParent(int parentRank, struct ArrayList *visited, MPI_Comm *graph_comm);
void receiveFromChild(int childRank, struct ArrayList *visited, MPI_Comm *graph_comm);
void receiveFromChildren(struct ArrayList *neighbors, MPI_Comm *graph_comm, int rank, struct ArrayList *visited, int *depth);
int receiveFromParent(struct ArrayList *neighbors, MPI_Comm *graph_comm, int rank, struct ArrayList *visited, int *depth);
void DFS(struct ArrayList *neighbors, MPI_Comm *graph_comm, int rank, int size, int bfsdepth, struct ArrayList *visited, int *depth);
int *distributedBFS(MPI_Comm *graph_comm, int rank, int size, int bfsdepth);

/**
 * Notifies a neighbor process of the current depth level and the visited nodes.
 *
 * @param neighborRank The rank of the neighbor process to notify.
 * @param visited The list of visited nodes.
 * @param graph_comm The MPI communicator for the graph.
 * @param depth The current depth level.
 */
void notifyNeighbour(int neighborRank, struct ArrayList *visited, MPI_Comm *graph_comm, int *depth)
{
    char comp[6];
    printf("Notifying %d\n", (neighborRank));
    MPI_Send(&(visited->size), 1, MPI_INT, neighborRank, 0, *graph_comm);
    // recieve ACK:<number> || DEC:0
    sprintf(comp, "ACK:%d", visited->size);
    char recieveBuffer[6];
    MPI_Recv(&recieveBuffer, 6, MPI_CHAR, neighborRank, 0, *graph_comm, MPI_STATUS_IGNORE);
    // compare strings
    if (strcmp((const char *)&recieveBuffer, (const char *)&comp) == 0)
    {
        printf("ACK recieved from %d\n", neighborRank);
    }
    else
    {
        printf("ERROR: ACK not recieved from %d\n", neighborRank);
        printf("Recieved: %s\n", recieveBuffer);
    }
    char *str = toString(visited);
    if (*depth > 1)
        printf("send visited: %s, size: %d\n", str, visited->size);
    free(str);
    MPI_Send(visited->list, visited->size, MPI_INT, neighborRank, 0, *graph_comm);
    // Distribute depth to neighbors
    int newdepth = *depth + 1;
    MPI_Send(&newdepth, 1, MPI_INT, neighborRank, 0, *graph_comm);
}

/**
 * Notify the neighbors of a node that it has been visited at a certain depth.
 *
 * @param neighbors The list of neighbors of the node.
 * @param visited The list of visited nodes.
 * @param graph_comm The MPI communicator for the graph.
 * @param depth The depth at which the node was visited.
 */
void notifyNeighbours(struct ArrayList *neighbors, struct ArrayList *visited, MPI_Comm *graph_comm, int *depth)
{
    for (int i = 0; i < neighbors->size; i++)
    {
        if (inList(neighbors->list[i], visited))
        {
            continue;
        }
        notifyNeighbour(neighbors->list[i], visited, graph_comm, depth);
    }
}

/**
 * Sends the visited nodes list to the parent node.
 *
 * @param parentRank The rank of the parent process.
 * @param visited A pointer to the ArrayList of visited vertices.
 * @param graph_comm A pointer to the MPI communicator.
 */
void notifyParent(int parentRank, struct ArrayList *visited, MPI_Comm *graph_comm)
{
    // send visited array size to parent
    MPI_Send(&(visited->size), 1, MPI_INT, parentRank, 0, *graph_comm);
    // recieve ACK:<number> || DEC:0
    char ack[6];
    sprintf(ack, "ACK:%d", visited->size);
    char recieveBuffer[6];
    MPI_Recv(&recieveBuffer, 6, MPI_CHAR, parentRank, 0, *graph_comm, MPI_STATUS_IGNORE);
    // compare strings
    if (strcmp((const char *)&recieveBuffer, (const char *)&ack) == 0)
    {
        printf("ACK recieved from %d\n", parentRank);
    }
    else
    {
        printf("ERROR: ACK not recieved from %d\n", parentRank);
        printf("Recieved: %s\n", recieveBuffer);
    }
    // send visited array to parent
    MPI_Send(visited->list, visited->size, MPI_INT, parentRank, 0, *graph_comm);
}

/**
 * Performs a depth-first search on a graph represented by a list of neighbors.
 * Uses MPI for distributed computing.
 *
 * @param neighbors A pointer to a struct ArrayList containing the neighbors of the current node.
 * @param graph_comm A pointer to the MPI communicator for the graph.
 * @param rank The rank of the current process.
 * @param size The total number of processes.
 * @param bfsdepth The depth of the current BFS iteration.
 * @param visited A pointer to a struct ArrayList containing the visited nodes.
 * @param depth A pointer to an integer representing the depth of the current node.
 */
void DFS(struct ArrayList *neighbors, MPI_Comm *graph_comm, int rank, int size, int bfsdepth, struct ArrayList *visited, int *depth)
{
    struct ArrayList *visitedChildren = malloc(sizeof(struct ArrayList));

    for (int i = 0; i < neighbors->size; i++)
    {
        if (inList(neighbors->list[i], visited))
        {
            continue;
        }
        // do DFS for one path
        notifyNeighbour(neighbors->list[i], visited, graph_comm, depth);
        receiveFromChild(neighbors->list[i], visitedChildren, graph_comm);
    }
    free(visited);
    visited = visitedChildren;
}

/**
 * Receives messages from child processes and updates the visited nodes and depth of the graph.
 *
 * @param neighbors Pointer to the ArrayList containing the neighbors of the current node.
 * @param graph_comm Pointer to the MPI communicator for the graph.
 * @param rank Rank of the current process.
 * @param visited Pointer to the ArrayList containing the visited nodes.
 * @param depth Pointer to the current depth of the graph.
 */
void receiveFromChildren(struct ArrayList *neighbors, MPI_Comm *graph_comm, int rank, struct ArrayList *visited, int *depth)
{
    struct ArrayList *visitedChildren = malloc(sizeof(struct ArrayList));
    for (int i = 0; i < neighbors->size; i++)
    {
        if (inList(neighbors->list[i], visited))
        {
            continue;
        }
        // do DFS for one path
        receiveFromChild(neighbors->list[i], visitedChildren, graph_comm);
    }
    free(visited);
    visited = visitedChildren;
}

/**
 * Receives data from a child process and updates the visited nodes list.
 *
 * @param childRank The rank of the child process sending the data.
 * @param visited A pointer to the ArrayList containing the visited nodes.
 * @param graph_comm A pointer to the MPI communicator for the graph.
 */
void receiveFromChild(int childRank, struct ArrayList *visited, MPI_Comm *graph_comm)
{
    struct ArrayList *visitedChild = malloc(sizeof(struct ArrayList));
    MPI_Recv(&(visitedChild->size), 1, MPI_INT, childRank, 0, *graph_comm, MPI_STATUS_IGNORE);
    char ack[6];
    sprintf(ack, "ACK:%d", visited->size);
    // printf("Rank: %d, Sending ACK: '%s' to %d\n", rank, ack, parentRank);
    MPI_Send(&ack, 6, MPI_CHAR, childRank, 0, *graph_comm);
    visitedChild->list = calloc(visitedChild->size, sizeof(int));
    MPI_Recv(visitedChild->list, visitedChild->size, MPI_INT, childRank, 0, *graph_comm, MPI_STATUS_IGNORE);
    // merge lists (visited is the output)
    visited = mergeLists(visitedChild, visited);
}

/**
 * Receives messages from the parent node and updates the list of visited nodes.
 *
 * @param neighbors Pointer to the ArrayList of neighbors to be updated.
 * @param graph_comm Pointer to the MPI communicator.
 * @param rank Rank of the current node.
 * @param visited Pointer to the ArrayList of visited nodes to be updated.
 * @param depth Pointer to the depth of the current node in the BFS tree.
 * @return The rank of the parent node.
 */
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

/**
 * Perform a distributed breadth-first search on a graph using MPI.
 *
 * @param graph_comm MPI communicator for the graph
 * @param rank rank of the current process
 * @param size total number of processes
 * @param bfsdepth depth of the BFS to perform
 * @return pointer to an array containing the BFS result
 */
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
        receiveFromChildren(neighborsList, graph_comm, rank, visited, &depth);
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
            receiveFromChildren(neighborsList, graph_comm, rank, visited, &depth);
        }
        else
        {
            DFS(neighborsList, graph_comm, rank, size, bfsdepth, visited, &depth);
        }
        notifyParent(parent, visited, graph_comm);
    }
    if (rank == 0)
    {
        // TODO: recieve from all children
    }
    free(neighbors);
}

/**
 * @brief The main function of the program.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of strings containing the command-line arguments.
 * @return int The exit status of the program.
 */
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

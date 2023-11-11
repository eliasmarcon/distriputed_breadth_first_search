#include "Graph.h"
#include "ArrayList.h"
#include "Node.h"
#include <stdbool.h>
#include <mpi.h>

bool sendSize(int *len, int rank, MPI_Comm *graph_comm);
bool recieveSize(int *len, int *rank, MPI_Comm *graph_comm);
void notifyNeighbour(int neighborRank, struct ArrayList *visited, MPI_Comm *graph_comm);
void notifyNeighbours(struct ArrayList *neighbors, struct ArrayList *visited, MPI_Comm *graph_comm);
void notifyParent(int parentRank, struct Node *children, MPI_Comm *graph_comm);
struct Node *receiveFromChild(MPI_Comm *graph_comm);
struct Node *receiveFromChildren(struct ArrayList *neighbors, MPI_Comm *graph_comm, struct ArrayList *visited);
int receiveFromParent(struct ArrayList *neighbors, MPI_Comm *graph_comm, struct ArrayList *visited);
struct Node *DFS(struct ArrayList *neighbors, MPI_Comm *graph_comm, int size, int bfsdepth, struct ArrayList *visited);
int *distributedBFS(MPI_Comm *graph_comm, int size, int bfsdepth);

int rank, *depth;

bool sendSize(int *len, int otherRank, MPI_Comm *graph_comm)
{
    printf("Rank: %d, Sending size: %d to Parent %d\n", rank, *len, otherRank);
    MPI_Send(len, 1, MPI_INT, otherRank, 0, *graph_comm);
    // recieve ACK:<number> || DEC:0
    char ack[15];
    sprintf(ack, "ACK:%d", *len);
    int recieveBufferLen = strlen(ack) + 1;
    char *recieveBuffer = malloc(recieveBufferLen * sizeof(char));
    MPI_Status ackstatus;
    MPI_Recv(recieveBuffer, recieveBufferLen, MPI_CHAR, otherRank, 0, *graph_comm, &ackstatus);
    // compare strings
    bool succ;
    if (strcmp((const char *)recieveBuffer, (const char *)&ack) == 0)
    {
        printf("Rank: %d, ACK recieved from %d\n", rank, otherRank);
        succ = true;
    }
    else
    {
        printf("Rank: %d, ERROR: ACK not recieved from %d\n", rank, otherRank);
        printf("Rank: %d, Recieved: %s from %d\n", rank, recieveBuffer, ackstatus.MPI_SOURCE);
        succ = false;
    }
    free(recieveBuffer);
    return succ;
}
bool recieveSize(int *len, int *otherRank, MPI_Comm *graph_comm)
{
    MPI_Status status;
    MPI_Recv(len, 1, MPI_INT, MPI_ANY_SOURCE, 0, *graph_comm, &status);
    *otherRank = status.MPI_SOURCE;
    char ack[6];
    sprintf(ack, "ACK:%d", *len);
    printf("Rank: %d, Sending ACK: '%s' to %d\n", rank, ack, *otherRank);
    MPI_Send(&ack, 6, MPI_CHAR, *otherRank, 0, *graph_comm);
    return true;
}

/**
 * Notifies a neighbor process of the current depth level and the visited nodes.
 *
 * @param neighborRank The rank of the neighbor process to notify.
 * @param visited The list of visited nodes.
 * @param graph_comm The MPI communicator for the graph.
 * @param depth The current depth level.
 */
void notifyNeighbour(int neighborRank, struct ArrayList *visited, MPI_Comm *graph_comm)
{
    printf("Rank: %d, Notifying %d\n", rank, (neighborRank));
    if (sendSize(&visited->size, neighborRank, graph_comm))
    {
        char *str = toString(visited);
        if (*depth > 1)
        { // printf("send visited: %s, size: %d\n", str, visited->size);
        }
        free(str);
        MPI_Send(visited->list, visited->size, MPI_INT, neighborRank, 0, *graph_comm);
        // Distribute depth to neighbors
    }
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
void notifyNeighbours(struct ArrayList *neighbors, struct ArrayList *visited, MPI_Comm *graph_comm)
{
    for (int i = 0; i < neighbors->size; i++)
    {
        if (inList(neighbors->list[i], visited))
        {
            continue;
        }
        notifyNeighbour(neighbors->list[i], visited, graph_comm);
    }
}

/**
 * Sends the visited nodes list to the parent node.
 *
 * @param parentRank The rank of the parent process.
 * @param visited A pointer to the ArrayList of visited vertices.
 * @param graph_comm A pointer to the MPI communicator.
 */
void notifyParent(int parentRank, struct Node *children, MPI_Comm *graph_comm)
{
    // send visited array size to parent
    char *json = convertToJson(children);
    int len = strlen(json);
    if (sendSize(&len, parentRank, graph_comm))
    {
        MPI_Send(json, len, MPI_CHAR, parentRank, 0, *graph_comm);
    }
    free(json);
}

/**
 * Performs a depth-first search on a graph represented by a list of neighbors.
 * Uses MPI for distributed computing.
 *
 * @param neighbors A pointer to a struct ArrayList containing the neighbors of the current node.
 * @param graph_comm A pointer to the MPI communicator for the graph.
 * @param size The total number of processes.
 * @param bfsdepth The depth of the current BFS iteration.
 * @param visited A pointer to a struct ArrayList containing the visited nodes.
 * @param depth A pointer to an integer representing the depth of the current node.
 *
 * @return A pointer to the current node.
 */
struct Node *DFS(struct ArrayList *neighbors, MPI_Comm *graph_comm, int size, int bfsdepth, struct ArrayList *visited)
{
    // save current node as Object
    int nodeSize = sizeof(struct Node);
    struct Node *node = malloc(nodeSize);
    node->rank = rank;
    node->depth = *depth;
    node->childrenCount = neighbors->size - 1;
    node->children = malloc((node->childrenCount) * sizeof(struct Node *));
    struct ArrayList *visitedChildren = malloc(sizeof(struct ArrayList));

    for (int i = 0; i < neighbors->size; i++)
    {
        if (inList(neighbors->list[i], visited))
        {
            continue;
        }
        // do DFS for one path
        notifyNeighbour(neighbors->list[i], visited, graph_comm);
        node->children[i] = receiveFromChild(graph_comm);
    }
    return node;
}

/**
 * Receives messages from child processes and updates the visited nodes and depth of the graph.
 *
 * @param neighbors Pointer to the ArrayList containing the neighbors of the current node.
 * @param graph_comm Pointer to the MPI communicator for the graph.
 * @param visited Pointer to the ArrayList containing the visited nodes.
 * @param depth Pointer to the current depth of the graph.
 *
 * @return A pointer to the current node.
 */
struct Node *receiveFromChildren(struct ArrayList *neighbors, MPI_Comm *graph_comm, struct ArrayList *visited)
{
    // save current node as Object
    int nodeSize = sizeof(struct Node);
    struct Node *node = malloc(nodeSize);
    node->rank = rank;
    node->depth = *depth;
    node->childrenCount = neighbors->size - 1;
    node->children = malloc((node->childrenCount) * sizeof(struct Node *));
    for (int i = 0; i < neighbors->size; i++)
    {
        if (inList(neighbors->list[i], visited))
        {
            continue;
        }
        node->children[i] = receiveFromChild(graph_comm);
        // struct ArrayList *visitedChild = receiveFromChild(graph_comm);
        // merge lists (visited is the output)
        // visitedChildren = mergeLists(visitedChildren, visitedChild);
    }
    return node;
}

/**
 * Receives data from a child process and updates the visited nodes list.
 *
 * @param graph_comm A pointer to the MPI communicator for the graph.
 *
 * @return A pointer to the child node.
 */
struct Node *receiveFromChild(MPI_Comm *graph_comm)
{
    int len = 0;
    int childRank = 0;
    struct Node *visitedChild;
    if (recieveSize(&len, &childRank, graph_comm))
    {
        char *json = calloc(len, sizeof(int));
        MPI_Recv(json, len, MPI_CHAR, childRank, 0, *graph_comm, MPI_STATUS_IGNORE);
        visitedChild = convertFromJson(json);
        char *str = NodetoString(visitedChild);
        printf("Rank: %d, Received from: %d, Recieved json: %s\n", rank, childRank, str);
        free(str);
    }
    return visitedChild;
}

/**
 * Receives messages from the parent node and updates the list of visited nodes.
 *
 * @param neighbors Pointer to the ArrayList of neighbors to be updated.
 * @param graph_comm Pointer to the MPI communicator.
 * @param visited Pointer to the ArrayList of visited nodes to be updated.
 * @param depth Pointer to the depth of the current node in the BFS tree.
 * @return The rank of the parent node.
 */
int receiveFromParent(struct ArrayList *neighbors, MPI_Comm *graph_comm, struct ArrayList *visited)
{
    int parentRank = 0;
    struct Node *visitedChild;
    if (recieveSize(&visited->size, &parentRank, graph_comm))
    {
        // allocate memory for visited array + 1 (for current node)
        visited->list = calloc(visited->size + 1, sizeof(int));

        // recieve visited array from parent
        // printf("Rank: %d, Reicieving size: %d\n", rank, visited->size);
        MPI_Recv(visited->list, visited->size, MPI_INT, parentRank, 0, *graph_comm, MPI_STATUS_IGNORE);
    }
    // incerease size by 1 -> add current node
    visited->size++;
    // recieve depth from parent
    MPI_Recv(depth, 1, MPI_INT, parentRank, 0, *graph_comm, MPI_STATUS_IGNORE);
    // printf("Rank: %d, Recieved depth: %d\n", rank, *depth);
    return parentRank == rank ? -1 : parentRank;
}

/**
 * Perform a distributed breadth-first search on a graph using MPI.
 *
 * @param graph_comm MPI communicator for the graph
 * @param size total number of processes
 * @param bfsdepth depth of the BFS to perform
 * @return pointer to an array containing the BFS result
 */
int *distributedBFS(MPI_Comm *graph_comm, int size, int bfsdepth)
{
    // -------------  Prepare Structures --------------------------
    MPI_Request request;
    depth = calloc(1, sizeof(int));

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
    struct Node *node;
    if (rank == 0)
    {
        notifyNeighbours(neighborsList, visited, graph_comm);
        parent = -1;
        node = receiveFromChildren(neighborsList, graph_comm, visited);
        free(visited);
    }
    else
    {
        // memcpy(visited->list + 1, neighbors, n_neighbors * sizeof(int));
        struct ArrayList *visitedParent = malloc(sizeof(struct ArrayList));
        parent = receiveFromParent(neighborsList, graph_comm, visitedParent);
        // printf("Rank: %d, has Parent: %d\n", rank, parent);
        //  merge lists
        // visited = mergeLists(visitedParent, visited);
        free(visited);
        visited = visitedParent;
        visited->list[visited->size - 1] = rank;
        // end merge lists
        char *str = toString(visited);
        printf("Rank: %d, Parent: %d, depth: %d, visited: %s\n", rank, parent, *depth, str);
        free(str);
        // only use BFS if depth is not reached
        if (*depth < bfsdepth)
        {
            notifyNeighbours(neighborsList, visited, graph_comm);
            printf("Rank: %d, recieveFromChildren\n", rank);
            node = receiveFromChildren(neighborsList, graph_comm, visited);
        }
        else
        {
            node = DFS(neighborsList, graph_comm, size, bfsdepth, visited);
        }
        free(visited);
        node->parent = parent;
        str = NodetoString(node);
        printf("Rank: %d, notifyParent: json = %s\n", rank, str);
        free(str);

        notifyParent(parent, node, graph_comm);
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

    int size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int bfsdepth;
    if (argc > 1)
    {
        bfsdepth = atoi(argv[1]);
        if (rank == 0)
            printf("Rank: %d, Setting depth to %d\n", rank, bfsdepth);
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
    int *result = distributedBFS(&graph_comm, graph->nodeAmount, bfsdepth);

    // TODO: Process results (extract diameter, etc.)
    //  Cleanup
    MPI_Comm_free(&graph_comm);
    free(graph->index);
    free(graph->edges_array);
    free(graph);
    free(depth);
    MPI_Finalize();

    return 0;
}

#include "../include/Graph.h"

/**
 * @brief Returns a pointer to a Graph struct with the specified number of nodes.
 *
 * @param nodes The number of nodes in the graph.
 * @return struct Graph* Pointer to the created Graph struct.
 */
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
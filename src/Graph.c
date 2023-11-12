#include "../include/Graph.h"

/**
 * @brief Returns a pointer to a Graph struct with the specified number of nodes.
 *
 * @param nodes The number of nodes in the graph.
 * @return struct Graph* Pointer to the created Graph struct.
 */

struct Graph *getGraph(int nodes, int tree)
{
    struct Graph *graph = malloc(sizeof(struct Graph));
    graph->nodeAmount = nodes;
    if (tree)
    {
        printf("Generating2\n");
        generateBinaryTree(nodes, &(graph->index), &(graph->edges_array));
        return graph;
    }
    printf("Generating3\n");
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

// Function to create the binary tree topology
/**
 * Generates a binary tree with the specified number of nodes.
 *
 * @param num_nodes The number of nodes in the binary tree.
 * @param indexes A pointer to a pointer that will hold the indexes of the nodes.
 * @param edges A pointer to a pointer that will hold the edges of the nodes.
 */
// void generateBinaryTree(int num_nodes, int **indexes, int **edges)
// {
//     *indexes = (int *)malloc(num_nodes * sizeof(int)); // Allocate memory for the indexes array.
//     int total_edges = (num_nodes - 1) * 2;             // Calculate the total number of edges in the tree.
//     *edges = (int *)malloc(total_edges * sizeof(int)); // Allocate memory for the edges array.

//     int edge_pos = 0; // Position in the edges array to insert new edges.
//     for (int i = 0; i < num_nodes; i++)
//     {
//         // For each node in the tree...
//         if (i < num_nodes / 2)
//         { // If it's an internal node...
//             // Set the cumulative degree in the indexes array.
//             (*indexes)[i] = (i == 0) ? 2 : (*indexes)[i - 1] + 2;
//             // Set the left and right children in the edges array.
//             (*edges)[edge_pos++] = i * 2 + 1; // Left child
//             (*edges)[edge_pos++] = i * 2 + 2; // Right child
//         }
//         else
//         { // If it's a leaf node...
//             // Set the cumulative degree for the leaf node.
//             (*indexes)[i] = (*indexes)[i - 1] + 1;
//             // Leaf nodes do not have children, so we don't add edges for them.
//         }
//     }
// }

void generateBinaryTree(int num_nodes, int **indexes, int **edges)
{
    *indexes = (int *)malloc(num_nodes * sizeof(int));
    int total_edges = (num_nodes - 1) * 2;
    *edges = (int *)malloc(total_edges * sizeof(int));

    int edge_pos = 0; // Position in the edges array

    printf("numnodes %d", num_nodes);
    for (int i = 0; i < num_nodes; i++)
    {
        if (i < num_nodes / 2)
        {
            // Internal nodes
            (*indexes)[i] = edge_pos;
            if (i * 2 + 1 < num_nodes)
            {
                (*edges)[edge_pos++] = i * 2 + 1; // Left child
            }
            if (i * 2 + 2 < num_nodes)
            {
                (*edges)[edge_pos++] = i * 2 + 2; // Right child
            }
        }
        else
        {
            // Leaf nodes
            (*indexes)[i] = edge_pos; // Leaf nodes do not add new edges
        }
    }
    printf("done");
}

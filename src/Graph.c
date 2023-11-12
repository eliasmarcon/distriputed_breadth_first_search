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
    graph->nodeAmount = nodes;
    generateBinaryTree(nodes, &(graph->index), &(graph->edges_array));
    return graph;
}

/**
Generates a binary tree with the specified number of nodes.
*@param num_nodes The number of nodes in the binary tree.
*@param indexes A pointer to a pointer that will hold the indexes of the nodes.
*@param edges A pointer to a pointer that will hold the edges of the nodes.
*/
void generateBinaryTree(int num_nodes, int **indexes, int **edges)
{
    *indexes = (int *)malloc(num_nodes * sizeof(int));
    *edges = (int *)malloc((num_nodes - 1) * 2 * sizeof(int));

    if (!(*indexes) || !(*edges))
    {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    int nrOfLeafs = 0;
    int parentCumulativeIndex = 0;
    int hasParent = 0;

    int edge_pos = 0; // Position in the edges array

    for (int i = 0; i < num_nodes; i++)
    {
        if (i > 0)
        {
            hasParent = 1;
        }
        int parentIndex = (i - 1) / 2;
        if (parentIndex >= 0 && i != 0)
        {
            (*edges)[edge_pos++] = parentIndex; // Parent node
        }

        // Add left child if it exists
        if (2 * i + 1 < num_nodes)
        {
            nrOfLeafs++;
            (*edges)[edge_pos++] = 2 * i + 1; // Left child
        }

        // Add right child if it exists
        if (2 * i + 2 < num_nodes)
        {
            nrOfLeafs++;
            (*edges)[edge_pos++] = 2 * i + 2; // Right child
        }

        int cumulativeIndex = nrOfLeafs + parentCumulativeIndex + hasParent;
        (*indexes)[i] = cumulativeIndex;
        nrOfLeafs = 0;
        parentCumulativeIndex = cumulativeIndex;
    }
}

/**
 * Prints the given graph.
 *
 * @    void printGraph(struct Graph *graph)
 * @param graph The graph to print.
 */
void printGraph(struct Graph *graph)
{
    printf("\n=======================================\n");
    printf("Graph with %d nodes\n", graph->nodeAmount);
    printf("Index: ");
    for (int i = 0; i < graph->nodeAmount; i++)
    {
        printf("%d ", graph->index[i]);
    }
    printf("\nEdges: ");
    for (int i = 0; i < graph->index[graph->nodeAmount - 1]; i++)
    {
        printf("%d ", graph->edges_array[i]);
    }
    printf("\n");
    printf("=======================================\n\n");
}

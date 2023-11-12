#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

bool findDuplicate(int* row, int max, int target)
{
    for (int i = 0; i < max; i++)
    {
        if (row[i] == target)
        {
            return true;
        }
        if (row[i] == -1)
        {
            return false;
        }
    }
    return false;
}

void insertEdge(int* row, int max, int newEdge)
{
    for (int i = 0; i < max; i++)
    {
        if (row[i] == -1)
        {
            row[i] = newEdge;
            return;
        }
    }
}


void generateRandomConnectedNetwork(int numNodes, int insertConnections, int **index, int **edges) {
    if (numNodes < 2) {
        // A connected network is not possible with less than 2 nodes.
        *index = NULL;
        *edges = NULL;
        return;
    }

    srand(time(NULL));   // Initialization, should only be called once.

    //prepare 2d array for inital graph generation
    int** graphBluePrint = (int**)malloc(numNodes * sizeof(int*));
    for (int i = 0; i < numNodes; i++) {
        graphBluePrint[i] = (int*)malloc(numNodes * sizeof(int));
        for (int j = 0; j < numNodes; j++) {
            graphBluePrint[i][j] = -1;
        }
    }

    int edgeCounter = 0;
    
    //generate graph
    for (int currentNode = 1; currentNode < numNodes; currentNode++)
    {
        for (int j = 0; j < insertConnections; j++)
        {
            int target = rand() % currentNode;
            if (findDuplicate(graphBluePrint[currentNode], numNodes, target))
            {
                continue;
            }
            insertEdge(graphBluePrint[currentNode], numNodes, target);
            insertEdge(graphBluePrint[target], numNodes, currentNode);
            edgeCounter += 2;
        }   
    }

    //transform graph into mpi readable arrays
    *index = (int *)malloc((numNodes + 1) * sizeof(int));
    *edges = (int *)malloc(edgeCounter * sizeof(int));

    edgeCounter = 0;
    for (int currentIndex = 0; currentIndex < numNodes; currentIndex++)
    {
        int currentEdge = 0;
        while (graphBluePrint[currentIndex][currentEdge] != -1) {
            (*edges)[edgeCounter + currentEdge] = graphBluePrint[currentIndex][currentEdge];
            currentEdge++;
        }
        edgeCounter += currentEdge;
        (*index)[currentIndex] = edgeCounter;
    }

    //freeing the blue print
    for (int currentNode = 0; currentNode < numNodes; currentNode++)
    {
        free(graphBluePrint[currentNode]);
    }
    free(graphBluePrint);
}



// Function to print the graph
void printGraph(int numNodes, int *index, int *edges) {
    if (index == NULL || edges == NULL) {
        printf("Graph is empty or not initialized.\n");
        return;
    }

    printf("Graph representation (Adjacency List):\n");
    for (int i = 0; i < numNodes; i++) {
        printf("Node %d: ", i);
        int start = 0;
        if ( i > 0)
        {
            start = index[i- 1];
        }
        for (int j = start; j < index[i]; j++) {
            printf("%d ", edges[j]);
        }
        printf("\n");
    }
}

// Example usage
int main() {
    int numNodes = 10;
     int *index = NULL;
    int *edges = NULL;

    generateRandomConnectedNetwork(numNodes, 1, &index, &edges);
    printGraph(numNodes, index, edges);

    free(index);
    free(edges);

    return 0;
}

#include "mpi.h" // Include the MPI library for parallel computing.
#include <stdio.h> // Standard input/output library for C.
#include <stdlib.h> // Standard library for C, includes memory allocation, process control, etc.
#include <stdbool.h> // Standard boolean library for C.
#include <string.h> // String handling functions.

#define NODES 15 // Define the number of nodes in the binary tree topology.

#define MAX_LEAVES (NODES - 1) // Maximum number of leaves we can track, which is all nodes minus the root.

// Define a structure to hold information about each node in the tree.
typedef struct {
    int is_leaf; // Indicator if the node is a leaf.
    int depth; // The depth of the node in the tree.
    int leaf_count; // Number of leaves under this node.
    int leaf_ranks[MAX_LEAVES]; // Array to store the ranks of the leaves.
} NodeInfo;

// Function to create the binary tree topology
void create_binary_tree_topology(int num_nodes, int **indexes, int **edges) {
    *indexes = (int *)malloc(num_nodes * sizeof(int)); // Allocate memory for the indexes array.
    int total_edges = (num_nodes - 1) * 2; // Calculate the total number of edges in the tree.
    *edges = (int *)malloc(total_edges * sizeof(int)); // Allocate memory for the edges array.

    int edge_pos = 0; // Position in the edges array to insert new edges.
    for (int i = 0; i < num_nodes; i++) {
        // For each node in the tree...
        if (i < num_nodes / 2) { // If it's an internal node...
            // Set the cumulative degree in the indexes array.
            (*indexes)[i] = (i == 0) ? 2 : (*indexes)[i - 1] + 2;
            // Set the left and right children in the edges array.
            (*edges)[edge_pos++] = i * 2 + 1; // Left child
            (*edges)[edge_pos++] = i * 2 + 2; // Right child
        } else { // If it's a leaf node...
            // Set the cumulative degree for the leaf node.
            (*indexes)[i] = (*indexes)[i - 1] + 1;
            // Leaf nodes do not have children, so we don't add edges for them.
        }
    }
}

int main(int argc, char **argv) {
    // Start of the main function, entry point of the program.
    
    // Initialize MPI environment and determine the rank and size.
    int my_rank, size;
    int all_leaf_ranks[MAX_LEAVES] = {0}; // Array to store all leaf ranks.
    int total_leaf_count = 0; // Counter for the total number of leaves.
    MPI_Init(&argc, &argv); // Initialize the MPI environment.
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); // Get the rank of the current process.
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get the total number of processes.

    // Check if there are enough processes to form the tree.
    if (size < NODES) {
        printf("This program requires at least %d processes.\n", NODES);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE); // Abort if not enough processes.
    }

    // Create the binary tree topology.
    int *indexes, *edges; // Pointers to arrays for topology.
    create_binary_tree_topology(NODES, &indexes, &edges); // Call the function to create topology.

    // Create a graph communicator based on the provided topology.
    MPI_Comm tree_communicator; // Declare the graph communicator.
    MPI_Graph_create(MPI_COMM_WORLD, NODES, indexes, edges, 0, &tree_communicator); // Create the graph communicator.

    // Check if the current process is part of the graph communicator.
    if (tree_communicator == MPI_COMM_NULL) {
        printf("MPI process %d is not part of the graph\n", my_rank);
        MPI_Finalize(); // Finalize the MPI environment.
        return 0; // Exit the program.
    }

    int neighbor_count; // Variable to store the number of neighbors.
    MPI_Graph_neighbors_count(tree_communicator, my_rank, &neighbor_count); // Get the number of neighbors.

    int neighbors[neighbor_count]; // Array to store the neighbors.
    MPI_Graph_neighbors(tree_communicator, my_rank, neighbor_count, neighbors); // Get the neighbors.

    // Print the neighbors of the current process.
    if(neighbor_count > 1){
        printf("MPI process %d has %d children.\n", my_rank, neighbor_count);
    }else{
        printf("MPI process %d has no children.\n", my_rank);
    }
   
    //for (int i = 0; i < neighbor_count; ++i) {
    //    printf("%d ", neighbors[i]); // Print each neighbor rank.
    //}
    //printf("\n");
    
    // Different behavior for the root node and other nodes.
    if (my_rank == 0) {
        // If it's the root node...

        // Start the Breadth-First Search (BFS).
        printf("Root node %d starting BFS...\n", my_rank);
        for (int i = 0; i < neighbor_count; ++i) {
            int child = neighbors[i]; // Get the rank of the child node.
            // Send a signal to each child to start its part of the BFS.
            printf("Root node %d sending BFS start signal to child %d\n", my_rank, child);
            MPI_Send(NULL, 0, MPI_BYTE, child, 0, tree_communicator);
        }

        // Variables to track the leaf count and maximum depth.
        int leaf_count = 0;
        int max_depth = 0;
        NodeInfo info; // Structure to store node information.

        // The root node waits for acknowledgments from its children.
        for (int i = 0; i < neighbor_count; ++i) {
            MPI_Status status; // Status of the receive operation.
            // Receive node information from any child.
            MPI_Recv(&info, sizeof(NodeInfo), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, tree_communicator, &status);
            // Process the received information.
            if (info.is_leaf) {
                leaf_count++; // Increment leaf count if it's a leaf.
            }
            if (info.depth > max_depth) {
                max_depth = info.depth; // Update the maximum depth if necessary.
            }
            // Accumulate all leaf ranks.
            for (int j = 0; j < info.leaf_count; ++j) {
                all_leaf_ranks[total_leaf_count++] = info.leaf_ranks[j];
            }
        }
        // Print the overall depth of the tree and the ranks of the leaves.
        printf("Root node %d knows the depth of the graph is %d and there are %d leaves. Leaf ranks are: ", my_rank, max_depth, total_leaf_count);
        for (int i = 0; i < total_leaf_count; ++i) {
            printf("%d ", all_leaf_ranks[i]); // Print each leaf rank.
        }
        printf("\n");

    } else {
        // If it's not the root node...

        MPI_Status status; // Status of the receive operation.
        // Wait for the signal from the parent.
        MPI_Recv(NULL, 0, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, tree_communicator, &status);
        int parent = status.MPI_SOURCE; // The rank of the parent node is stored here.

        int children_count = 0; // Counter for the number of children.
        // Signal children if any.
        for (int i = 0; i < neighbor_count; ++i) {
            int child = neighbors[i];
            if (child > my_rank) { // If the neighbor is a child (has a higher rank)...
                children_count++; // Increment the children count.
                // Send a signal to the child.
                printf("Node %d sending signal to node %d\n", my_rank, neighbors[i]);
                MPI_Send(NULL, 0, MPI_BYTE, child, 0, tree_communicator);
            }
        }

        // Each node maintains a count and list of leaf ranks under it.
        NodeInfo info = {0, 0, 0, {0}}; // Initialize node information.
        int leaf_ranks[MAX_LEAVES] = {0}; // Local array to store leaf ranks.
        int total_leaf_count = 0; // Counter for the number of leaves under this node.

        NodeInfo child_info; // Structure to receive data from children.

        // If this node is a leaf, report and send acknowledgment to the parent.
        if (children_count == 0) {

            // Print that this node is a leaf node.
            printf("Leaf node reached: MPI process %d\n", my_rank);
            // Send acknowledgment back to the parent.


            // Populate the info structure for the leaf node.
            NodeInfo info = {1, 1, 1, {my_rank}}; // Leaf node with depth 1, is a leaf, and includes its own rank.

            

            MPI_Send(&info, sizeof(NodeInfo), MPI_BYTE, parent, 0, tree_communicator);
            
            printf("Leaf node %d sent acknowledgment to parent %d\n", my_rank, parent);
        } else {
            // Not a leaf, wait for acknowledgments from children
            for (int i = 0; i < children_count; ++i) {
                MPI_Recv(&child_info, sizeof(NodeInfo), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, tree_communicator, &status);
                printf("Node %d received acknowledgment from child %d\n", my_rank, status.MPI_SOURCE);

                // Update depth if the child's depth is greater
                if (child_info.depth > info.depth) {
                    info.depth = child_info.depth;
                }

                // Accumulate leaf ranks from this child
                for (int j = 0; j < child_info.leaf_count; ++j) {
                    leaf_ranks[total_leaf_count++] = child_info.leaf_ranks[j];
                }
            }
            // After receiving from all children, send acknowledgment to parent
            info.is_leaf = 0; // This node is not a leaf
            info.depth++; // Increment depth for this node
            info.leaf_count = total_leaf_count; // Set the total number of leaf nodes under this node
            memcpy(info.leaf_ranks, leaf_ranks, total_leaf_count * sizeof(int)); // Copy the accumulated leaf ranks
            MPI_Send(&info, sizeof(NodeInfo), MPI_BYTE, parent, 0, tree_communicator);
            printf("Node %d sent acknowledgment to parent %d with %d leaves under it.\n", my_rank, parent, total_leaf_count);
        }
    }

    // Finalize the MPI environment
    MPI_Finalize();
    return 0;
}


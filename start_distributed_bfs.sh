#!/bin/bash

# Check for the number of tasks and diameter arguments
if [ "$#" -gt 3 ]; then
    echo "Usage: $0 [<runtype>] [<number_of_tasks>] [<tree_depth>]"
    exit 1
fi

# Set default values if arguments are not provided
run_type=${1:-"local"}
num_tasks=${2:-5}
tree_depth=${3:-10}

# Output file to save results
output_file="./distributed_bfs_result.txt"

if [ "$run_type" == "cluster" ]; then
    # Running on the Slurm cluster
    echo "Running on the Slurm cluster..."
    echo "Running distributed BFS with $num_tasks MPI tasks and tree_depth $tree_depth..."
    srun -n $num_tasks --mpi=pmi2 ./out/mpi_distributed_bfs $tree_depth >> $output_file
elif [ "$run_type" == "local" ]; then
    # Running locally
    echo "Running locally..."
    echo "Running distributed BFS with $num_tasks MPI tasks and tree_depth $tree_depth..."
    mpirun -np $num_tasks ./out/mpi_distributed_bfs $tree_depth >> $output_file
else
    echo "Invalid run type. Use 'local' or 'cluster'."
    exit 1
fi

echo "Completed."
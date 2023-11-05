EXECS=mpi_distributed_bfs
MPICC?=mpicc.mpich

# Define the output directory
OUTDIR=out

# Create the output directory if it doesn't exist
$(OUTDIR):
	mkdir -p $(OUTDIR)

all: ${EXECS}

mpi_distributed_bfs: src/distributed_bfs.c
	${MPICC} -o out/mpi_distributed_bfs src/distributed_bfs.c

mpi_bfs: src/binarytree_bfs.c
	${MPICC} -o out/mpi_bfs src/binarytree_bfs.c

clean:
	rm -f ${EXECS}
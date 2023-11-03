EXECS=mpi_distributed_bfs
MPICC?=mpicc.mpich

all: ${EXECS}

mpi_distributed_bfs: src/distributed_bfs.c
	${MPICC} -o out/mpi_distributed_bfs src/distributed_bfs.c

mpi_bfs: src/binarytree_bfs.c
	${MPICC} -o out/mpi_bfs src/binarytree_bfs.c

clean:
	rm -f ${EXECS}
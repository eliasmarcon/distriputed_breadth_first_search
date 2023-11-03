EXECS=mpi_bfs
MPICC?=mpicc.mpich

all: ${EXECS}

mpi_bfs: BFS.c
	${MPICC} -o mpi_bfs FloodMax.c

clean:
	rm -f ${EXECS}
EXECS=mpi_distributed_bfs
MPICC?=mpicc.mpich

# Define the output directory
OUTDIR=out

# Create the output directory if it doesn't exist
$(OUTDIR):
	mkdir -p $(OUTDIR)

all: ${EXECS}

mpi_distributed_bfs: $(OUTDIR) src/distributed_bfs.c
	${MPICC} -o $(OUTDIR)/mpi_distributed_bfs src/distributed_bfs.c

clean:
	rm -f $(OUTDIR)/*

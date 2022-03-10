build:
	mpiCC tasks_mpi.cpp -o tasks_mpi -lm

run:
	mpirun --oversubscribe -np 9 ./tasks_mpi 12

clean:
	rm tasks_mpi
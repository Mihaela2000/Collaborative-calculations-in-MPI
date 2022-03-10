// Buzdugan Mihaela
#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

void send_mess(int size, int source, int destination, int tag, int *data) {
    MPI_Send(data, size, MPI_INT, destination, tag, MPI_COMM_WORLD);
    cout << "M(" + to_string(source) + ',' + to_string(destination) + ")" << "\n";
}


//////////////////////////  Task 1  //////////////////////////

// afiseaza topologia
void print_topology(int rank, int coordinators[][100], int *process_worker) {
    string topologie = to_string(rank) + " ->";
    int i, j;

    i = 0;
    while (i < 3) {
        topologie += ' ' + to_string(i) + ':';

        j = 0;
        while (j < process_worker[i] - 1) {
            topologie += to_string(coordinators[i][j]) + ',';
            j++;
        }
        topologie += to_string(coordinators[i][j]);
        i++;
    }

    cout << topologie << "\n";
}

// trimite topologia propirie catre ceilalti coordonatori
void send_topology(int rank, int coordinators[][100], int *process_worker) {
    int i = 1;

    while (i < 3) {
        send_mess(1, rank, (rank + i) % 3, 0, &process_worker[rank]);
        i++;
    }

    i = 1;
    while (i < 3) {
        send_mess(process_worker[rank], rank, (rank + i) % 3, 0, coordinators[rank]);
        i++;
    }
}

// primesc numarul de workeri si workerii
void recv_pr_and_no(MPI_Status status, int rank, int coordinators[][100], int *process_worker) {
    int i = 1;
    int size;
    while (i < 3) {
        size = 1;
        MPI_Recv(&process_worker[(rank + i) % 3], size, MPI_INT, (rank + i) % 3, 0, MPI_COMM_WORLD, &status);
        i++;
    }

    i = 1;
    while (i < 3) {
        size = process_worker[(rank + i) % 3];
        MPI_Recv(coordinators[(rank + i) % 3], size, MPI_INT, (rank + i) % 3, 0, MPI_COMM_WORLD, &status);
        i++;
    }
}

// trimite topologia catre workeri
void send_top_to_workers(int rank, int coordinators[][100], int *process_worker){
    int i = 0;

    while (i < process_worker[rank]) {
        send_mess(3, rank, coordinators[rank][i], 0, process_worker);

        int j = 0;
        while(j < 3) {
            send_mess(process_worker[j], rank, coordinators[rank][i], j, coordinators[j]);
            j++;
        }
        i++;
    }
}

int task1(MPI_Status status, int rank, int coordinator, int coordinators[][100], int *process_worker) {
    
    if (rank >= 3 ) { // workerii primesc numarul de procese si topologia
        int index = 0;
        int size;

        size = 3;
        MPI_Recv(process_worker, size, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        coordinator = status.MPI_SOURCE;

        while (index < 3) {
            size = process_worker[index];
            MPI_Recv(coordinators[index], size, MPI_INT, coordinator, index, MPI_COMM_WORLD, &status);
            index++;
        }

    } 
    // coordonatorii citesc datele de intrare si isi trimit unul altuia topologia
    // iar apoi o trimit catre toti workerii
    else { 
        string filename = "cluster" + to_string(rank) + ".txt";
        ifstream fin(filename);
        int i = 0;

        fin >> process_worker[rank];

        while (i < process_worker[rank]) {
            fin >> coordinators[rank][i];
            i++;
        }

        fin.close();

        send_topology(rank, coordinators, process_worker);

        recv_pr_and_no(status, rank, coordinators, process_worker);

        send_top_to_workers(rank, coordinators, process_worker);
    }
    print_topology(rank, coordinators, process_worker);
    return coordinator;
}


//////////////////////////  Task 2  //////////////////////////

// afiseaza vectorul final
void print_final_vector(int rank, int k, int v[]) {
    if (rank == 0) {
        string result;

        int i = 0;
        while (i < k) {
            result += ' ' + to_string(v[i]);
            i++;
        }
        /*result += '\n';*/
        cout << "Rezultat:" << result << "\n";
    }
}

// 0 primeste de la coordonatorii 1 si 2
void reacv_from_coord (int nr_coord, MPI_Status status, int thread_start, int thread_end, int v[]) {
    int size;
    size = 1;
    MPI_Recv(&thread_start, size, MPI_INT, nr_coord, 1, MPI_COMM_WORLD, &status);
    MPI_Recv(&thread_end, size, MPI_INT, nr_coord, 2, MPI_COMM_WORLD, &status);
    size = thread_end - thread_start;
    void* data = v + thread_start;
    MPI_Recv(data, size, MPI_INT, nr_coord, 0, MPI_COMM_WORLD, &status);
}

// trimite catre coordonatorul 0
void send_to_coord0 (MPI_Status status, int rank, int thread_start, int thread_end, int v[]) {
    int size;
    size = 1;
    send_mess(size, rank, 0, 1, &thread_start);
    send_mess(size, rank, 0, 2, &thread_end);
    size = thread_end - thread_start;
    int* data = v + thread_start;
    send_mess(size, rank, 0, 0, data);
}

void send_recv_zero (int rank, MPI_Status status, int thread_start, int thread_end, int v[]) {
    if (rank == 0) { // 0 primeste de la coordonatorii 1 si 2
        reacv_from_coord (1, status, thread_start, thread_end, v);
        reacv_from_coord (2, status, thread_start, thread_end, v);
        
    } else { // trimite catre coordonatorul 0
        send_to_coord0 (status, rank, thread_start, thread_end, v);
    }
}

// seteaza indexul bucatii pe care trebuie sa o trimita coordonatorii catre workeri
int set_index(int i, int rank, int process_rank, int *process_worker, int coordinators[][100]) {
    process_rank = coordinators[rank][i];

    int index = i;
    int j = 1;
    while (j <= rank) {
        index = index + process_worker[j - 1];
        j++;
    }

    return index;
}

void task2(MPI_Status status, int rank, int v[], int k , int coordinator, int coordinators[][100], int *process_worker, int no_procs) {

    int start, end;
    int size;
    int thread_start = 10000;
    int thread_end = -1;

    if (rank == 1 || rank == 2) {
        size = k;
        MPI_Recv(v, size, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    } else if (rank == 0) { // trimite vectorul catre ceilalti coordonatori
        for (int i = 0; i < k; ++i) {
            v[i] = i;
        }

        int i = 1;
        while (i < 3) {
            send_mess(k, rank, i, 0, v);
            i++;
        }
    }

    if (rank >= 3) { // workerii primesc si trimit bucata dupa ce au inmultit-o cu 2
        size = k;
        MPI_Recv(v, size, MPI_INT, coordinator, 0, MPI_COMM_WORLD, &status);
        size = 1;
        MPI_Recv(&start, size, MPI_INT, coordinator, 1, MPI_COMM_WORLD, &status);
        MPI_Recv(&end, size, MPI_INT, coordinator, 2, MPI_COMM_WORLD, &status);

        int i = start;
        while (i < end) {
            v[i] *= 2;
            i++;
        }

        send_mess(end - start, rank, coordinator, 0, v + start);
        
    } else { // fiecare coordonator trimite procesului 0 partea facuta de workerii lui
        int process_rank;
        int start;
        int end;
        int index;

        int i = 0;
        while (i < process_worker[rank]) {
            process_rank = coordinators[rank][i];
            index = set_index(i, rank, process_rank, process_worker, coordinators);

            start = index * (double)k / (no_procs - 3);
            end = k < ((index + 1) * k / (no_procs - 3)) ? k : ((index + 1) * k / (no_procs - 3));

            send_mess(k, rank, process_rank, 0, v);
            send_mess(1, rank, process_rank, 1, &start);
            send_mess(1, rank, process_rank, 2, &end);

            if (start < thread_start) thread_start = start;
            if (end > thread_end) thread_end = end;

            void* data = v + start;
            int size = end - start;
            MPI_Recv(data, size, MPI_INT, process_rank, 0, MPI_COMM_WORLD, &status);
            i++;
        }

        send_recv_zero(rank, status, thread_start, thread_end, v);
    }

    print_final_vector(rank, k, v);
}

int main(int argc, char *argv[]) {
    int no_procs, rank;
    int coordinators[3][100];
    int coordinator = -1;
    int process_worker[3]{1, 1, 1};
    int v[10000];
    int k = atoi(argv[1]);

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &no_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;

    // Task 1
    coordinator = task1(status, rank, coordinator, coordinators, process_worker);

    // Task 2
    task2(status, rank, v, k , coordinator, coordinators, process_worker, no_procs);

    MPI_Finalize();

    return 0;
}
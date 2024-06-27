#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <vector>

using namespace std;

// Generate a random number between 0 and M
int generate_random(int M) {
    return rand() % M;
}

// Find the next process that is still playing
int find_next_rank(int* playing, int size, int current_rank) {
    int next_rank = (current_rank + 1) % size;
    while (playing[next_rank] == 0) {
        next_rank = (next_rank + 1) % size;
    }
    //printf("Rank %d -> Rank %d\n", current_rank, next_rank);
    return next_rank;
}

int main(int argc, char *argv[]) {
    int rank, size, token, M, remaining;

    // Check the number of arguments
    if (argc != 5) {
        fprintf(stderr, "Usage: %s -t <initial_token> -M <max_random>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse the arguments
    token = atoi(argv[2]);
    M = atoi(argv[4]);
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    srand(time(NULL));
    remaining = size; 
    int playing[size];
    for (int i = 0; i < size; i++) {
        playing[i] = 1;
    }

    // Determine the initial process to start with the token
    if (rank == 0) {
        token -= generate_random(M);
        if (token > 0){
            int next_rank = find_next_rank(playing, size, rank);
            MPI_Send(&token, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
            MPI_Send(&playing, size, MPI_INT, next_rank, 2, MPI_COMM_WORLD);
            MPI_Send(&remaining, 1, MPI_INT, next_rank, 3, MPI_COMM_WORLD);
            printf("Proceso %d tiene la papa con valor %d\n", rank, token);   
        }else{
            printf("Proceso %d tiene la papa con valor %d (proceso %d sale del juego)\n", rank, token, rank);
            playing[rank] = 0;
            remaining--;
            token = generate_random(M);
            int next_rank = find_next_rank(playing, size, rank);
            MPI_Send(&token, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
            MPI_Send(&playing, size, MPI_INT, next_rank, 2, MPI_COMM_WORLD);
            MPI_Send(&remaining, 1, MPI_INT, next_rank, 3, MPI_COMM_WORLD);
        }
    }
    
    while (remaining > 1) {
        sleep(1);
        if (playing[rank] == 1) {
            MPI_Recv(&token, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&playing, size, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status);
            MPI_Recv(&remaining, 1, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &status);
            
            if (remaining == 1){
                break;
            }
            token -= generate_random(M);
            if (token > 0){
                printf("Proceso %d tiene la papa con valor %d\n", rank, token);
                int next_rank = find_next_rank(playing, size, rank);
                //printf("Rank %d -> New Rank %d\n", rank, next_rank);
                MPI_Send(&token, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
                MPI_Send(&playing, size, MPI_INT, next_rank, 2, MPI_COMM_WORLD);
                MPI_Send(&remaining, 1, MPI_INT, next_rank, 3, MPI_COMM_WORLD);
            } else{
                printf("Proceso %d tiene la papa con valor %d (proceso %d sale del juego)\n", rank, token, rank);
                playing[rank] = 0;
                remaining--;
                token = generate_random(M);
                
                int next_rank = find_next_rank(playing, size, rank);
                //printf("Rank %d -> New Rank %d\n", rank, next_rank);
                MPI_Send(&token, 1, MPI_INT, next_rank, 0, MPI_COMM_WORLD);
                MPI_Send(&playing, size, MPI_INT, next_rank, 2, MPI_COMM_WORLD);
                MPI_Send(&remaining, 1, MPI_INT, next_rank, 3, MPI_COMM_WORLD);

            }
        }else {
            break;
        }
    }

    // Synchronize all processes before finalizing
    MPI_Barrier(MPI_COMM_WORLD);

    // Determine the winner
    if (playing[rank] == 1) {
        printf("Proceso %d es el ganador\n", rank);
    }

    // Finalize MPI
    MPI_Finalize();
    return 0;
}

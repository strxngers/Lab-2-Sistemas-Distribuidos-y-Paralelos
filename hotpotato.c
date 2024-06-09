#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define TAG 0

int random_number(int M) {
    return rand() % M;
}

int main(int argc, char** argv) {
    int rank, size, token, M;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 5) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s -t <initial_token_value> -M <max_random_value>\n", argv[0]);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-t") != 0 || strcmp(argv[3], "-M") != 0) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s -t <initial_token_value> -M <max_random_value>\n", argv[0]);
        }
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    token = atoi(argv[2]);
    M = atoi(argv[4]);

    srand(time(NULL) + rank);

    int *playing = malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        playing[i] = 1;
    }

    int remaining_players = size;
    int current_token = token;

    while (remaining_players > 1) {
        if (playing[rank]) {
            if (rank == 0) {
                // Proceso 0 inicia con el token
                current_token = token;
                printf("Proceso %d tiene la papa con valor %d\n", rank, current_token);
                fflush(stdout);
            } else {
                // Recibir el token del proceso anterior
                MPI_Recv(&current_token, 1, MPI_INT, (rank - 1 + size) % size, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                printf("Proceso %d tiene la papa con valor %d\n", rank, current_token);
                fflush(stdout);
            }

            current_token -= random_number(M);

            if (current_token < 0) {
                printf("Proceso %d tiene la papa con valor %d (proceso %d sale del juego)\n", rank, current_token, rank);
                playing[rank] = 0;
                remaining_players--;
                fflush(stdout);
                for (int i = 0; i < size; i++) {
                    if (i != rank) {
                        MPI_Send(&rank, 1, MPI_INT, i, TAG, MPI_COMM_WORLD);
                    }
                }
            } else {
                int next_rank = (rank + 1) % size;
                while (!playing[next_rank]) {
                    next_rank = (next_rank + 1) % size;
                }
                MPI_Send(&current_token, 1, MPI_INT, next_rank, TAG, MPI_COMM_WORLD);
                if (rank == 0) {
                    MPI_Recv(&current_token, 1, MPI_INT, (size - 1), TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    printf("Proceso %d tiene la papa con valor %d\n", rank, current_token);
                }
                fflush(stdout);
            }
        } else {
            int loser;
            MPI_Recv(&loser, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if (loser == rank) {
                playing[rank] = 0;
                remaining_players--;
            }
        }
    }

    if (playing[rank]) {
        printf("Proceso %d es el ganador\n", rank);
        fflush(stdout);
    }

    free(playing);
    MPI_Finalize();
    return 0;
}

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include  <pthread.h>

void show_error_message_and_exit(char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

void do_signal(int id, int number) {
    struct sembuf sops;
    sops.sem_num = number;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    if (semop(id, &sops, 1) == -1) {
        perror(NULL);
        show_error_message_and_exit("[error] at do signal function\n");
    }
}

void do_wait(int id, int number) {
    struct sembuf sops;
    sops.sem_num = number;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    if (semop(id, &sops, 1) == -1) {
        perror(NULL);
        show_error_message_and_exit("[error] at do wait function\n");
    }
}

void initialize_semaphore(int id, int number, int value) {
    if (semctl(id, number, SETVAL, value) < 0) {
        perror(NULL);
        show_error_message_and_exit("[error] initialize semaphore\n");
    }
}

int main() {
    puts("synchronization exercise with semaphores:\n");

    int semaphore;
    int duration;
    if ((semaphore = semget(IPC_PRIVATE, 1, IPC_CREAT | 0700)) < 0) {
        perror(NULL);
        show_error_message_and_exit("[error] semget");
    }
    
    initialize_semaphore(semaphore, 0, 0);
    srand(time(NULL));

    for (int index=0; index<6; index++) {

        duration = rand() % 1000000 + 10000;
        switch (fork())
        {
            case -1:
                show_error_message_and_exit("[error] fork");

            case 0:
                printf("[son_%d] start\n", index);
                usleep(duration);
                printf("[son_%d] end\n\n", index);

                do_signal(semaphore, 0);
                exit(0);

            default:
                do_wait(semaphore, 0);

                printf("[father_%d] start\n", index);
                usleep(duration);
                printf("[father_%d] end\n\n", index);
        }

        usleep(1000);
    }
    
    if ((semctl(semaphore, 0, IPC_RMID)) == -1) {
        perror(NULL);
        show_error_message_and_exit("[error] semctl");
    }

    return 0;
}
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include  <pthread.h>

struct parameters {
    int *available;
    int *semaphore;
};

void show_error_message_and_exit(char *message) {
    fprintf(stderr, "%s", message);
    exit(1);
}

void do_signal(int id, int number) {
    struct sembuf sops;
    sops.sem_num = number;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    if (semop(id, &sops, 1) == -1) {
        perror(NULL);
        show_error_message_and_exit("[error] at do signal function");
    }
}

void do_wait(int id, int number) {
    struct sembuf sops;
    sops.sem_num = number;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    if (semop(id, &sops, 1) == -1) {
        perror(NULL);
        show_error_message_and_exit("[error] at do wait function");
    }
}

void initialize_semaphore(int id, int number, int value) {
    if (semctl(id, number, SETVAL, value) < 0) {
        perror(NULL);
        show_error_message_and_exit("[error] initialize semaphore");
    }
}

void *thread_1(void *arg) {
    int *available;
    int *semaphore;
    int available_in_thread;
    struct parameters *parameters;

    parameters = (struct parameters *) arg;
    available = parameters->available;
    semaphore = parameters->semaphore;

    do_wait(*semaphore, 0);

    sleep(1);
    puts("[thread_1] start");
    available_in_thread = *available;

    sleep(1);
    available_in_thread++;
    *available = available_in_thread;
    printf("[thread_1] available: %d \n", available_in_thread);
    puts("[thread_1] end\n");

    do_signal(*semaphore, 0);
}

void *thread_2(void *arg) {
    int *available;
    int *semaphore;
    int available_in_thread;
    struct parameters *parameters;

    parameters = (struct parameters *) arg;
    available = parameters->available;
    semaphore = parameters->semaphore;

    do_wait(*semaphore, 0);

    sleep(1);
    puts("[thread_2] start");
    available_in_thread = *available;

    sleep(1);
    available_in_thread++;
    *available = available_in_thread;
    printf("[thread_2] available: %d \n", available_in_thread);
    puts("[thread_2] end\n");

    do_signal(*semaphore, 0);
}

int main() {
    puts("syncronization exercise with semaphore");

    int semaphore;
    int availables = 0;
    pthread_t h1;
    pthread_t h2;
    struct parameters parameters;

    if ((semaphore = semget(IPC_PRIVATE, 1, IPC_CREAT | 0700)) < 0) {
        perror(NULL);
        show_error_message_and_exit("[error] semget");
    }

    initialize_semaphore(semaphore, 0, 1);

    parameters.available = &availables;
    parameters.semaphore = &semaphore;


    for (int ind = 0; ind < 5; ind++) {

        pthread_create(&h1, NULL, thread_1, (void *) &parameters);
        pthread_create(&h2, NULL, thread_2, (void *) &parameters);
        pthread_join(h1, NULL);
        pthread_join(h2, NULL);

        sleep(2);
    }

    if ((semctl(semaphore, 0, IPC_RMID)) == -1) {
        perror(NULL);
        show_error_message_and_exit("[error] semctl");
    }

    return 0;
}

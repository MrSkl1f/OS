#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>

#define SIZE 24

#define FULL 0
#define EMPTY 1
#define BIN 2

#define PROD 3
#define CONS 3

char* shared_buffer = NULL;
char* cons_pos = 0;
char* prod_pos = 0;

char alph[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int len = 26;

struct sembuf prod_start[2] = {
    {EMPTY, -1, 0},
    {BIN, -1, 0}
};
struct sembuf prod_stop[2] = {
    {BIN, 1, 0},
    {FULL, 1, 0}
};

struct sembuf cons_start[2] = {
    {FULL, -1, 0},
    {BIN, -1, 0}
};
struct sembuf cons_stop[2] = {
    {BIN, 1, 0},
    {EMPTY, 1, 0}
};

int perms = S_IRWXU | S_IRWXG | S_IRWXO;

int get_sem() 
{
    int sem = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);
    if (sem == -1)
    {
        perror("sem\n");
        exit(1);
    }
    int s1 = semctl(sem, FULL, SETVAL, 0);
    int s2 = semctl(sem, EMPTY, SETVAL, SIZE);
    int s3 = semctl(sem, BIN, SETVAL, 1);
    
    return sem;
}

int get_shared_memory()
{
    int shared_memory = shmget(IPC_PRIVATE, (SIZE + 2) * sizeof(char), IPC_CREAT | perms);
    if (shared_memory == -1) 
    {
        perror("shmget\n");
        exit(1);
    }

    prod_pos = (char*)shmat(shared_memory, 0, 0);
    if (prod_pos == (char*)-1) 
    {
        perror("shmat\n");
        exit(1);
    }
    cons_pos = prod_pos + sizeof(char);
    shared_buffer = cons_pos + sizeof(char);
    
    return shared_memory;
}

void producer(int sem, int id)
    {
    while(1) 
    {    
        if (semop(sem, prod_start, 2) == -1)
        {
            perror("semop\n");
            exit(1);
        }

        shared_buffer[*prod_pos] = alph[*prod_pos];
        printf("Producer %d: %c\n", id, alph[*prod_pos]);

        if (++(*prod_pos) == len) 
            (*prod_pos) = 0;

        if (semop(sem, prod_stop, 2) == -1)
        {
            perror("semop\n");
            exit(1);
        }

        sleep(rand() % 5);
    }
}

void consumer(int sem, int id)
{
    while(1) 
    {
        if (semop(sem, cons_start, 2) == -1)
        {
            perror("semop\n");
            exit(1);
        }

        printf("Consumer %d: %c\n", id, shared_buffer[*cons_pos]);
        
        if (++(*cons_pos) == len)
            (*cons_pos) = 0;

        if (semop(sem, cons_stop, 2) == -1)
        {
            perror("semop\n");
            exit(1);
        }

        sleep(rand() % 5);
    }
}

void fork_proc(void (*func)(int sem, int id), int sem, int count)
{
    for (int i = 0; i < count; i++) 
    {
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork");
            exit(1);
        }

        if (pid == 0) 
        {
            func(sem, i + 1);
            exit(0);
        }
    }
}

void catch_signal(int signalNum)
{
    printf("Catched\n");
}

int main()
{
    srand(time(NULL));
    int shared_memory = get_shared_memory();
    int sem = get_sem();

    fork_proc(producer, sem, PROD);
    fork_proc(consumer, sem, CONS);

    signal(SIGINT, catch_signal);

    for (int i = 0; i < PROD + CONS; i++)
    {
        int status;
        wait(&status);
    }

    shmctl(shared_memory, IPC_RMID, NULL);
    semctl(sem, BIN, IPC_RMID, 0);

    return 0;
}

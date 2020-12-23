#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define ACTIVE_WRITER 0
#define WAITING_WRITER 1
#define ACTIVE_READER 2
#define WAITING_READER 3

int* shared_buffer = NULL;

struct sembuf canread[3] = 
{
    {ACTIVE_WRITER, 0, 0},
    {WAITING_WRITER, 0, 0},
    {WAITING_READER, 1, 0}
};
struct sembuf startread[2] =
{
    {ACTIVE_READER, 1, 0},
    {WAITING_READER, -1, 0}
};
struct sembuf stopread[1] = 
{
    {ACTIVE_READER, -1, 0}
};

struct sembuf canwrite[3] = 
{
    {ACTIVE_READER, 0, 0},
    {ACTIVE_WRITER, 0, 0},
    {WAITING_WRITER, 1, 0}
};
struct sembuf startwrite[2] =
{
    {ACTIVE_WRITER, 1, 0},
    {WAITING_WRITER, -1, 0}
};
struct sembuf stopwrite[1] = 
{
    {ACTIVE_WRITER, -1, 0}
};

int perms = S_IRWXU | S_IRWXG | S_IRWXO;

int readers = 5;
int writers = 3;

int get_sem() 
{
    int sem = semget(IPC_PRIVATE, 5, IPC_CREAT | perms);
    if (sem == -1)
    {
        perror("sem\n");
        exit(1);
    }
    int s1 = semctl(sem, ACTIVE_WRITER, SETVAL, 0);
    int s2 = semctl(sem, ACTIVE_READER, SETVAL, 0);
    int s3 = semctl(sem, WAITING_WRITER, SETVAL, 0);
    int s4 = semctl(sem, WAITING_READER, SETVAL, 0);

    return sem;
}

int get_shared_memory()
{
    int shared_memory = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | perms);
    if (shared_memory == -1) 
    {
        perror("shmget\n");
        exit(1);
    }

    shared_buffer = (int*)shmat(shared_memory, 0, 0);
    if (shared_buffer == (int*)-1) 
    {
        perror("shmat\n");
        exit(1);
    }
    
    *shared_buffer = 0;
    
    return shared_memory;
}

void start_read(int sem)
{
    if (semop(sem, canread, 3) == -1)
    {
        perror("semop\n");
        exit(1);
    }
    if (semop(sem, startread, 2) == -1)
    {
        perror("semop\n");
        exit(1);
    }
}

void stop_read(int sem)
{
    if (semop(sem, stopread, 1) == -1)
    {
        perror("semop\n");
        exit(1);
    }
}

void start_write(int sem)
{
    if (semop(sem, canwrite, 3) == -1)
    {
        perror("semop\n");
        exit(1);
    }
    if (semop(sem, startwrite, 2) == -1)
    {
        perror("semop\n");
        exit(1);
    }
}

void stop_write(int sem)
{
    if (semop(sem, stopwrite, 1) == -1)
    {
        perror("semop\n");
        exit(1);
    }
}

void writer(int sem, int id) 
{
    while (1)
    {
        start_write(sem);

        *shared_buffer += 1;
        printf("Writer %d: %d\n", id, *shared_buffer);

        stop_write(sem);

        sleep(rand() % 5);
    }
}

void reader(int sem, int id) 
{
    while (1)
    {
        start_read(sem);

        printf("Reader %d: %d\n", id, *shared_buffer);

        stop_read(sem);

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
    int shared_memory = get_shared_memory();
    int sem = get_sem();

    fork_proc(writer, sem, writers);
    fork_proc(reader, sem, readers);
 
    signal(SIGINT, catch_signal);

    for (int i = 0; i < writers + readers; i++)
    {
        int status;
        wait(&status);
    }

    shmctl(shared_memory, IPC_RMID, NULL);

    return 0;
}

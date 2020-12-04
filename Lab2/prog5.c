#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>

int flag = 0;

void catch_signal(int sig_numb)
{
    printf("\nSignal Cntrl + C\n");
    flag = 1;
}

void wait_signal()
{
    printf("Press Ctrl + C to write.\n");
	signal(SIGINT, catch_signal);
    sleep(3);
}

pid_t fork_child(int child_num, int *fd) 
{
    pid_t child = fork();
    if (child == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (child == 0) 
    {
        int child_pid = getpid();
        void *pid = &child_pid;
        close(fd[0]);
        write(fd[1], pid, sizeof(pid));
        exit(0);
    }
    return child;
}

void wait_for_childs(int *fd)
{
    close(fd[1]);
    printf("YES\n");
    int stat_val;
    void *pid;
    read(fd[0], pid, sizeof(pid));
    
    pid_t child = wait(&stat_val);
    printf("Child %d wrote %d\n", child, *(int *)(pid));
    if (WIFEXITED(stat_val))
            printf("Child=%d completed normally with code=%d.\n", child, WEXITSTATUS(stat_val));
    else if (WIFSIGNALED(stat_val))
        printf("Child=%d ended with a non-intercepted signal with code=%d.\n", child, WTERMSIG(stat_val));
    else if (WIFSTOPPED(stat_val))
        printf("Child=%d stopped with %d code.\n", child, WSTOPSIG(stat_val));
}

int main() 
{
    int fd[2];
    if (pipe(fd) == -1)
	{
        perror("pipe");
		exit(1);
	}

    wait_signal();

    pid_t child_1 = fork_child(1, fd);
    pid_t child_2 = fork_child(2, fd);

    printf("parent: pid=%d, group=%d, child_1=%d, child_2=%d\n", getpid(), getpgrp(), child_1, child_2);
    wait_for_childs(fd);
    wait_for_childs(fd);
    
    return 0;
}
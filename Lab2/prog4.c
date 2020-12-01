#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <string.h>

void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) 
    {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

void convert_to_str(int n, char s[])
{
    int i = 0;
    do 
    {
        s[i++] = n % 10 + '0';   
    } while ((n /= 10) > 0);     
    s[i] = '\0';
    reverse(s);
} 


pid_t fork_child(int child_num, int *fd) 
{
    pid_t child = fork();
    char pid[10];
    if (child == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (child == 0) 
    {
        convert_to_str(getpid(), pid);
        close(fd[0]);
        write(fd[1], pid, sizeof(pid));
        exit(0);
    }
    return child;
}

void wait_for_childs(int *fd)
{
    int stat_val;
    char pid[10];
    read(fd[0], pid, 10);
    pid_t child = wait(&stat_val);
    printf("Child %d wrote %s\n", child, pid);
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

    pid_t child_1 = fork_child(1, fd);
    pid_t child_2 = fork_child(2, fd);

    printf("parent: pid=%d, group=%d, child_1=%d, child_2=%d\n", getpid(), getpgrp(), child_1, child_2);
    close(fd[1]);
    wait_for_childs(fd);
    wait_for_childs(fd);
    
    return 0;
}
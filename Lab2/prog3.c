#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>

void print_child(int child_num)
{
    printf("child: number=%d pid=%-5d parent=%-5d group=%-5d\n", child_num, getpid(), getppid(), getpgrp());
}

pid_t fork_child(int child_num, char *path, char *arg0) 
{
    pid_t child = fork();
    if (child == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (child == 0) 
    {
        print_child(child_num);
        if (execl(path, arg0, NULL) == -1)
        {
            perror("exec");
            exit(1);
        }
        
    }
    return child;
}

void wait_for_childs()
{
    int stat_val;
    pid_t child = wait(&stat_val);
    printf("Child has finished: PID=%d\n", child);
    if (WIFEXITED(stat_val))
            printf("Child=%d completed normally with code=%d.\n", child, WEXITSTATUS(stat_val));
    else if (WIFSIGNALED(stat_val))
        printf("Child=%d ended with a non-intercepted signal with code=%d.\n", child, WTERMSIG(stat_val));
    else if (WIFSTOPPED(stat_val))
        printf("Child=%d stopped with %d code.\n", child, WSTOPSIG(stat_val));
}

int main() 
{
    pid_t child_1 = fork_child(1, "/bin/ls", "ls");
    pid_t child_2 = fork_child(2, "/bin/ps", "ps");

    printf("parent: pid=%d, group=%d, child_1=%d, child_2=%d\n", getpid(), getpgrp(), child_1, child_2);

    wait_for_childs();
    wait_for_childs();
    
    return 0;
}
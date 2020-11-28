#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <string.h>

int flag = 0;
int fd[2];

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

void catch_signal(int sig_numb)
{
    printf("\nSignal Cntrl + C\n");
    if (flag)
        flag = 0;
    else 
        flag = 1;
}

void wait_signal(char *operation)
{
    printf("Press Ctrl + C to %s.\n", operation);
	signal(SIGINT, catch_signal);
	sleep(5);
}

pid_t fork_child(int child_num) 
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

void wait_for_childs(pid_t child, int stat_val)
{
    char pid[10];
    read(fd[0], pid, 10);
    printf("Child %d wrote %s\n", child, pid);
    if (WIFEXITED(stat_val))
            printf("Child=%d completed normally with code=%d.\n", child, WEXITSTATUS(stat_val));
    else if (WIFSIGNALED(stat_val))
        printf("Child=%d ended with a non-intercepted signal with code=%d.\n", child, WTERMSIG(stat_val));
    else if (WIFSTOPPED(stat_val))
        printf("Child=%d stopped with %d code.\n", child, WSTOPSIG(stat_val));
}

void parent_read(pid_t child_1, pid_t child_2)
{
    if (child_1 != 0 && child_2 != 0)
    {
        int stat_val_1;
        int stat_val_2;
        pid_t child_1 = wait(&stat_val_1);
        pid_t child_2 = wait(&stat_val_2);

        wait_signal("read");
        if (flag)
            exit(0);

        close(fd[1]);
        wait_for_childs(child_1, stat_val_1);
        wait_for_childs(child_2, stat_val_2);
    }
}

int main() 
{
    if (pipe(fd) == -1)
	{
        perror("pipe");
		exit(1);
	}

    wait_signal("write");
    
    if (!flag)
        exit(0);

    pid_t child_1 = fork_child(1);
    pid_t child_2 = fork_child(2);
    printf("parent: pid=%-5d, child_1=%-5d, child_2=%-5d\n", getpid(), child_1, child_2);
    parent_read(child_1, child_2);
    return 0;
}
/*
Задание: моделирование возникновения процессов сирот.
Везде не менее 2х форков
потомки - sleep()
потомок
    собственный id, id предка, id группы
    принт до sleep, после
предок
    собственный id, id потомка
*/

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>

void print_child(int child_num, char *descr)
{
    printf("child: number=%d pid=%-5d parent=%-5d group=%-5d %s\n", child_num, getpid(), getppid(), getpgrp(), descr);
}

pid_t fork_child(int child_num) 
{
    pid_t child = fork();
    if (child == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (child == 0) 
    {
        print_child(child_num, "before sleep");
        sleep(2);
        print_child(child_num, "after sleep");
        exit(0);
    }
    return child;
}

int main() 
{
    pid_t child_1 = fork_child(1);
    pid_t child_2 = fork_child(2);

    printf("parent: pid=%-5d, child_1=%-5d, child_2=%-5d\n", getpid(), child_1, child_2);

    return 0;
}
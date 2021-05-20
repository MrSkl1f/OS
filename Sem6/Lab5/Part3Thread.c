#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<pthread.h>

void *write(int *fd)
{
    struct stat bufStat;
    
    FILE *file = fopen("result.txt", "w");
    stat("result.txt", &bufStat);
    printf("Opening file\n\tinode\t= %ld\n\tsize\t= %ld\n", bufStat.st_ino, bufStat.st_size);
    
    char needChar = 'a' + *fd - 1;    
    while (needChar <= 'z') 
    {
        fprintf(file, "%c", needChar);
        needChar += 2;
    }

    fclose(file);
    stat("result.txt", &bufStat);
    printf("Closing\n\tinode\t= %ld\n\tsize\t= %ld\n", bufStat.st_ino, bufStat.st_size);
}

int main()
{
    pthread_t thread1, thread2;
    int fd1 = 1,
        fd2 = 2;

    int stat1 = pthread_create(&thread1, NULL, write, &fd1);
    if (stat1 != 0)
    {
        printf("Cannot create thread 1\n");
        return -1;
    }

    int stat2 = pthread_create(&thread2, NULL, write, &fd2);
    if (stat2 != 0)
    {
        printf("Cannot create thread 2\n");
        return -1;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    return 0;
}
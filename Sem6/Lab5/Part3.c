#include <stdio.h>
#include <sys/stat.h>

int main()
{
    struct stat bufStat;

    FILE *file1 = fopen("result.txt", "w");
    stat("result.txt", &bufStat);
    printf("First opening\n\tinode\t= %ld\n\tsize\t= %ld\n", bufStat.st_ino, bufStat.st_size);

    FILE *file2 = fopen("result.txt", "w");
    stat("result.txt", &bufStat);
    printf("Second opening\n\tinode\t= %ld\n\tsize\t= %ld\n", bufStat.st_ino, bufStat.st_size);

    char needChar = 'a';
    while (needChar <= 'z') 
    {
        if (needChar % 2 == 0)
        {
            fprintf(file1, "%c", needChar);
        }
        else
        {
            fprintf(file2, "%c", needChar);
        }
        needChar++;
    }

    fclose(file1);
    stat("result.txt", &bufStat);
    printf("First closing\n\tinode\t= %ld\n\tsize\t= %ld\n", bufStat.st_ino, bufStat.st_size);

    fclose(file2);
    stat("result.txt", &bufStat);
    printf("Second closing\n\tinode\t= %ld\n\tsize\t= %ld\n", bufStat.st_ino, bufStat.st_size);

    return 0;
}
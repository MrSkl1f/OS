#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static int      myFtw(char *);
static int      doPath(char *, int level);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Использование ./lab2.out <каталог>\n");
        return 1;
    }

    return myFtw(argv[1]);
}

/*
    Обойти дерево каталогов, начиная с каталога "pathname".
    Пользовательская функция func() вызывается для каждого встреченного файла.
*/

#define	FTW_F	1   /* файл, не являющийся каталогом */
#define	FTW_D	2	/* каталог */
#define	FTW_DNR	3	/* каталог, недоступный для чтения */
#define	FTW_NS  4	/* файл, информацию о котором */
                    /* невозможно получить с помощью stat */

// Первый вызов для введенного каталога
static int myFtw(char *pathname)
{
    // изменение текущей директории для использования коротких имен
    if (chdir(pathname) == -1) 
    {
        printf("Ошибка вызова функции chdir %s\n", pathname);
        return 1;
    }
    return doPath(".", 0);
}

/*
    Обход дерева каталогов, начиная с "fullpath". Если "fullpath" не является
    каталогом, для него вызывается lstat(), func() и затем выполняется возврат.
    Для директорий производится рекурсивный вызов функции.
*/
static int doPath(char *fullpath, int level)
{
    struct stat statbuf;
    DIR *dp;
    
    if (lstat(fullpath, &statbuf) < 0) // Ошибка вызова stat
    {
        return 1;
    }
    
    /*
        Это каталог. Сначала вызовем func(),
        а затем обработаем все файлы в каталоге.
    */
    for (int i = 0; i < level; i++)
    {
        printf("|    ");
    }
    printf("|- %s\n", fullpath);

    if (S_ISDIR(statbuf.st_mode) == 0) //Если не каталог
    {
        return 1;
    }
    
    if ((dp = opendir(fullpath)) == NULL) // каталог не доступен
    {
        return 1;
    } 

    // Изменение текущей директории для использования коротких имен.
    if (chdir(fullpath) < 0) 
    {
        printf("Ошибка вызова функции chdir %s\n", fullpath);
        return 1;
    }

    level += 1;
    struct dirent *dirp;
    while ((dirp = readdir(dp)) != NULL)
    {
        // Пропуск каталогов . и ..
        if (strcmp(dirp->d_name, ".") != 0 && strcmp(dirp->d_name, "..") != 0 && strcmp(dirp->d_name, ".git") != 0) 
        {
            doPath(dirp->d_name, level);
        }
    }
    if (chdir("..") == -1)
    {
        printf("Ошибка вызова функции chdir ..\n");
    }
    if (closedir(dp) < 0)
    {
        printf("Невозможно закрыть каталог %s\n", fullpath);
    }
    return 0;
}




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
char *statmFields[] = {
	"size",       // общий размер программы
	"resident",   // размер резидентной части
	"share",      // разделяемые страницы
	"trs",        // текст (код)
	"drs",        // данные/стек
	"lrs",        // библиотека
	"dt"         // "дикие" (dirty) страницы
};

char *statFields[] = {
	"pid 		(id процесса)",
	"comm 		(имя исполняемого файла)",
	"state 		(состояние R/S/D/Z/T)",
	"ppid 		(parent pid процесса)",
	"pgrp 		(pgrp процесса)",
	"session 	(id сессии)",
	"tty 		(управляющий терминал)",
	"tpgid 		(pgrp процесса управляющего терминалом)",
	"flags 		(флаги процесса)",
	"min_flt 	(количество малых сбоев работы процесса)",
	"cmin_flt 	(количество малых сбоев в процессе и его сыновьях)",
	"maj_flt 	(количество существенных сбоев в процессе)",
	"cmaj_flt 	(количество существенных сбоев в процессе и его сыновьях)",
	"utime 		(количество тиков после распределения работы в режиме пользователя)",
	"stime 		(количество тиков после распределения работы в режиме ядра)",
	"cutime 	(количество тиков после распределения работы процесса и его сыновей в режиме пользователя)",
	"cstime 	(количество тиков после распределения работы процесса и его сыновей в режиме ядра)",
	"priority 	(приоритет процесса)",
	"nice",
	"num_threads (количество потоков)",
	"it_real_value 	(количество тиков до отправки SIGALARM)",
	"start_time (время от запуска системы до запуска процесса)",
	"vsize 		(размер виртуальной памяти, байт)",
	"rss 		(количество страниц загруженных в память)",
	"rlim 		(предел размера процесса)",
	"start_code (адрес, выше которого может выполняться программа",
	"end_code 	(адрес, ниже которого может выполняться программа",
	"start_stack (адрес начала стека)",
	"kstk_esp 	(текущее значение ESP)",
	"kstk_eip 	(текущее значение EIP)",
	"signal 	(побитовая таблица задержки сигналов)",
	"blocked 	(побитовая таблица блокируемых сигналов)",
	"sigignore 	(побитовая таблица игнорируемых сигналов)",
	"sigcatch 	(побитовая таблица полученных сигналов)",
	"wchan 		(канал, где процесс находится в режиме ожидания)",
	"nswap",
	"cnswap",
	"exit_signal",
	"processor",
	"rt_priority",
	"policy",
	"delayacct_blkio_tics",
	"quest_time",
	"cquest_time",
	"start_data",
	"end_data",
	"start_brk",
	"arg_start",
	"arg_end",
	"env_start",
	"env_end",
	"exit_code"
};

#define BUF_SIZE 0x1000
#define PATH_SIZE 255

int run(pid_t pid, FILE *result);

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("sudo ./main.out <pid>\n");
	}

	FILE *result = fopen("myFile.txt", "w");

	run(atoi(argv[1]), result);
	
	fclose(result);

	return 0;
}

//файл, указывает на директорию процесса
int readCmdline(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/cmdline", pid);
	fprintf(dest, "\n-cmdline:\n");

	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	char buf[BUF_SIZE];
	int len = fread(buf, 1, BUF_SIZE, f);
	buf[len] = 0;

	fprintf(dest, "\tComplete command line - %s\n", buf);
	fclose(f);

	return 0;
}


//символическая ссылка на директорию процесса
int readCwd(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/cwd", pid);

	fprintf(dest, "\n-cwd:\n");

	char buf[BUF_SIZE];
	int len = readlink(path, buf, BUF_SIZE);
	if (!len) {
		fprintf(dest, "Can't read file %s\n", path);
		return -1;
	}
	buf[len] = 0;

	fprintf(dest, "\tPathname of current working directory - %s\n", buf);
	
	return 0;
}


//файл
int readEnviron(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/environ", pid);
	
	fprintf(dest, "\n-environ:\n");

	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	char buf[BUF_SIZE];
	int len = 0;
	while ((len = fread(buf, 1, BUF_SIZE, f)) > 0) {
		for (int i = 0; i < len; i++) {
			if (buf[i] == 0) {
				buf[i] = 10;
			}
		}
		buf[len] = 0; 
		fprintf(dest, "%s\n", buf);
	}

	fclose(f);

	return 0;
}


//символическая ссылка на файл процесса
int readExe(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/exe", pid);

	fprintf(dest, "\n-exe:\n");

	char buf[BUF_SIZE];
	int len = readlink(path, buf, BUF_SIZE);
	if (!len) {
		fprintf(dest, "Can't read file %s\n", path);
		return -1;
	}
	buf[len] = 0;

	fprintf(dest, "\tPathname of the executed programm - %s\n", buf);

	return 0;
}


//файл-директория
int readFd(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/fd", pid);

	fprintf(dest, "\n-fd:\n");


	DIR *dp = opendir(path); // open directory
	if (!dp) {
		fprintf(dest, "Can't open file%s\n", path);
		return -1;
	}
	
	struct dirent *dirp;
	char path_to_file[BUF_SIZE];
	char link_to_file[BUF_SIZE];
	int len;

	while((dirp = readdir(dp)) != NULL) {   
		if((strcmp(dirp->d_name, ".") !=0 ) && (strcmp(dirp->d_name, "..") != 0)) {
			len = sprintf(path_to_file, "%s/%s", path, dirp->d_name);
			path_to_file[len] = 0;

			len = readlink(path_to_file, link_to_file, BUF_SIZE);
			link_to_file[len] = 0;

			fprintf(dest, "\tFile: %s\tInode: %ld\tLink: %s\n", path_to_file, dirp->d_ino, link_to_file);
		}
	}

	closedir(dp);
	return 0;
}


//файл со списком выделенных участков памяти
int readMaps(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/maps", pid);

	fprintf(dest, "\n-maps:\n");

	char buf[BUF_SIZE];
	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	int len = 0;
	while ((len = fread(buf, 1, BUF_SIZE, f)) > 0) {
		buf[len] = 0; 
		fprintf(dest, "%s\n", buf);
	}

	fclose(f);

	return 0;
}


//soft link на корень ФС
int readRoot(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/root", pid);

	fprintf(dest, "\n-root:\n");

	char buf[BUF_SIZE];
	int len = readlink(path, buf, BUF_SIZE);
	if (!len) {
		fprintf(dest, "Can't read file %s\n", path);
		return -1;
	}
	buf[len] = 0;
	
	fprintf(dest, "\tPer-process root of the filesystem: %s\n", buf);

	return 0;
}


int readStat(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/stat", pid);

	fprintf(dest, "\n-stat:\n");

	FILE *f = fopen(path,"r");
	if (!f) {    
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	char buf[BUF_SIZE];
	int len = fread(buf, 1, BUF_SIZE, f);
	buf[len] = '\0';

	int i = 0;
	char *pch = strtok(buf, " ");
	while(pch != NULL) {
		fprintf(dest, "\t%s:\t%s\n", statFields[i], pch);
		pch = strtok(NULL, " ");
		i++;
	}

	fclose(f);

	return 0;
}

лаб 4.1 /proc/statm Информация о памяти совпадает с полем rss в stat

//файл
int readStatm(FILE *dest, pid_t pid)
{
	char path[PATH_SIZE];
	snprintf(path, PATH_SIZE, "/proc/%d/statm", pid);
	
	fprintf(dest, "\n-statm:\n");

	FILE *f = fopen(path, "r");
	if (!f) {
		fprintf(dest, "Can't open file %s\n", path);
		return -1;
	}

	char buf[BUF_SIZE];
	int len = 0;
	while ((len = fread(buf, 1, BUF_SIZE, f)) > 0) {
		for (int i = 0; i < len; i++) {
			if (buf[i] == 0) {
				buf[i] = 10;
			}
		}
		buf[len] = 0; 
		fprintf(dest, "%s\n", buf);
	}

	fclose(f);

	return 0;
}

int run(pid_t pid, FILE *result)
{
	readCmdline(result, pid);
	readCwd(result, pid);
	readEnviron(result, pid);
	readExe(result, pid);
	readFd(result, pid);
	readMaps(result, pid);
	readRoot(result, pid);
	readStat(result, pid);

	return 0;
}
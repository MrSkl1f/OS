#include <syslog.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h> //umask
#include <unistd.h>   //setsid
#include <stdio.h>    //perror
#include <signal.h>   //sidaction
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/file.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

sigset_t mask;

void HandleSIGHUP(int signum)
{
    char buf[100];
    long int ttime = time(NULL);
    sprintf(buf, "SIGHUP received. User: %s. Time: %s", getlogin(), ctime(&ttime));
    syslog(LOG_WARNING, buf);
}

int lockfile(int fd)
{
    struct flock fl; // блокировка на запись
    fl.l_type = F_WRLCK; 
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;
    return (fcntl(fd, F_SETLK, &fl));
}

int already_running(void)
{

    syslog(LOG_ERR, "Проверяем на многократный запуск");

    int fd;
    char buf[16];
    
    // открываем лок файл
    fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);

    if (fd < 0)
    {
        syslog(LOG_ERR, "невозможно открыть %s: %s!", LOCKFILE, strerror(errno));
        exit(1);
    }

    syslog(LOG_WARNING, "Открыт lock-файл");

    // проверка блокировки
    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);
            exit(1);
        }
        syslog(LOG_ERR, "невозможно установить блокировку на %s: %s!\n", LOCKFILE, strerror(errno));
        exit(1);
    }
    // если не заблокирован, то ставим блокировку
    flock(fd, LOCK_EX | LOCK_UN);
    if (errno == EWOULDBLOCK)
    {
        syslog(LOG_ERR, "невозможно установить блокировку на %s: %s!", LOCKFILE, strerror(errno));
        close(fd);
        exit(1);
    }

    syslog(LOG_WARNING, "Записываем PID!");

    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    syslog(LOG_WARNING, "Записали PID!");

    return 0;
}

void daemonize(const char *cmd)
{
    int fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    // 1. Сбрасывание маски режима создания файла
    // чтобы можно было создавать файлы с любыми правами доступа
    umask(0);

    // 2. Получение максимального возможного номера дискриптора
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        perror("Невозможно получить максимальный номер дискриптора\n");
    }

    // 3. Стать лидером новой сессии, чтобы утратить управляющий терминал
    // не общается с пользователем
    if ((pid = fork()) < 0)
    {
        perror("fork\n");
    }
    else if (pid != 0) // родительский процесс
    {
        exit(0);
    }

    // чтобы стать главой сессии 
    setsid();

    // 4. Обеспечение невозможности обретения терминала в будущем
    // мы игнорируем сигнал SIGHUP (вызывается при утрате управляющего терминала) - для уведомления о потере соединения с управляющим терминалом пользователем
    // SIGHUP передается лидеру сессии
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        perror("Невозможно игнорировать сигнал SIGHUP!\n");
    }

    // Не нужен в линукс
    /*
        if ((pid = fork()) < 0)
        {
            perror("Ошибка функции fork!\n");
        }
        else if (pid != 0) //родительский процесс
        {
            exit(0);
        }
    */
    // 5. Назначить корневой каталог текущим рабочим каталогом,
    // чтобы впоследствии можно было отмонтировать файловую систему
    // если демон был записан на флешку
    if (chdir("/") < 0)
    {
        perror("Невозможно назначить корневой каталог текущим рабочим каталогом!\n");
    }

    // 6. Зактрыть все open файловые дескрипторы
    // для того, чтобы мы не держали у себя ресурсы
    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }
    for (int i = 0; i < rl.rlim_max; i++)
    {
        close(i);
    }

    // 7. Присоединить файловые дескрипторы 0, 1, 2 к /dev/null
    // чтобы когда что-то пишется в stderror, stdin, stdout все в нул и игнорило
    // если в демоне функции принтф или сканф, то вся инфа будет отправлена на нул
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0); //копируем файловый дискриптор
    fd2 = dup(0);

    // 8. Инициализировать файл журнала
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d\n", fd0, fd1, fd2);
        exit(1);
    }

    syslog(LOG_WARNING, "Демон запущен!");
}

void thr_fn(void *arg)
{
    int err, signo;
    for (;;) {
        // для ожидания сигнала. Как только приходит
        err = sigwait(&mask, &signo);
        if (err != 0)
        {
            syslog(LOG_ERR, "Error call sigwait");
        }
        switch (signo)
        {
        case SIGTERM:
            exit(1);
            break;
        case SIGHUP:
            HandleSIGHUP(signo);
            break;
        default:
            syslog(LOG_INFO, "GET undefined SIGNAL %d\n", signo);
            break;
        }
    }
}

int main()
{
    struct sigaction sa;
    pthread_t tid;
    daemonize("daemon2");
    
    

    // 9. Блокировка файла для одной существующей копии демона
    // чтоб 1 демон был
    if (already_running() != 0)
    {
        syslog(LOG_ERR, "Демон уже запущен!\n");
        exit(1);
    }

    // Восстановить действие по умолчанию для сигнала сигхап и заблокировать все сигналы
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    int err;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        perror("Cигнал SIGHUP error!\n");
        exit(1);
    }
    // ставлю масочку, так что все 1 и все обрабатываю 

    sigfillset(&mask);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
    {
        perror("Cигнал SIG_BLOCK error!\n");
        exit(1);
    }

    err = pthread_create(&tid, NULL, thr_fn, 0);
    if (err != 0)
    {
        perror("Create thread error!\n");
        exit(1);
    }
    //thr_fn(&mask);

    long int ttime = time(NULL);

    while (1)
    {
        syslog(LOG_INFO, "Time: %s", ctime(&ttime));
        sleep(6);
    }
}

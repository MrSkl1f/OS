#include <signal.h>
#ifndef SOCKETS
#define SOCKETS

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCK_NAME "mysock.soc"
#define MSG_LEN 256

#endif

void close_socket();
void signal_handler(int signum);


static int sock;

void close_socket() 
{
    close(sock);
    // удалить файл сокета
    unlink(SOCK_NAME);
}

void signal_handler(int signum) 
{
    printf("\nCatched signal\n");
    close_socket();
    exit(0);
}


int main() 
{
    // Создаёт конечную точку соединения и возвращает файловый дескриптор.
    // безымянный файл
    // SOCK_DGRAM поддерживает дейтаграммы (ненадежные сообщения с ограниченной длиной без установки соединения).
    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    struct sockaddr addr = {
        .sa_family = AF_UNIX,
        .sa_data = SOCK_NAME
    };

    // привязывает сокет к локальному адресу
    if (bind(sock, &addr, sizeof(addr)) == -1) {
        perror("bind failed");
        return EXIT_FAILURE;
    }

    signal(SIGINT, signal_handler);

    printf("Wait message\n\n");

    char msg[MSG_LEN];
    while (1) {
        // принимает данные из сокета
        ssize_t bytes = recvfrom(sock, msg, sizeof(msg), 0, NULL, NULL);
        if (bytes == -1) {
            close_socket();
            perror("recv error");
            return EXIT_FAILURE;
        }

        msg[bytes] = '\n';
        msg[bytes + 1] = 0;
        printf("Message: %s", msg);
    }
}
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

int main(int argc) 
{
    char msg[MSG_LEN];
    scanf("%s", &msg);
    // Создаёт конечную точку соединения и возвращает файловый дескриптор.
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }

    //связывание безымянного сокета с адресом
    struct sockaddr addr = {
        .sa_family = AF_UNIX,
        .sa_data = SOCK_NAME
    };

    // отправляют данные в сокет
    ssize_t err = sendto(sock, msg, strlen(msg), 0, &addr, sizeof(addr));
    if (err == -1) {
        perror("sendto failed");
        return EXIT_FAILURE;
    }
    // После окончания передачи данных сокет закрывается
    close(sock);
    return 0;
}
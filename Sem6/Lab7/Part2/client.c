#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>

#include <arpa/inet.h>
#include <netdb.h>

#define HOST "localhost"
#define PORT 5000
#define MSG_LEN 256
#define MAX_CLIENTS 10


int main(void)
{
    // создаёт конечную точку соединения и возвращает файловый дескриптор.
    // AF_INET Протоколы Интернет IPv4
    // SOCK_STREAM Обеспечивает создание двусторонних, 
    // надёжных потоков байтов на основе установления соединения. 
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }
    // преобразование  доменного  имени  сервера  в  его сетевой адрес
    struct hostent *server = gethostbyname(HOST); 
    if (server == NULL) {
        perror("gethostbyname failed");
        return EXIT_FAILURE;
    }
    // Для  сетевого  взаимодействия 
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr = *((struct in_addr*) server->h_addr_list[0]),
        // преобразовать число из порядка хоста в сетевой
        .sin_port = htons(PORT)
    };
    
    // Соединение с другим сокетом
    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("connect failed");
        return EXIT_FAILURE;
    }

        
    char msg[MSG_LEN];
    for (size_t i = 0; i < 5; i++) {
        sprintf(msg, "Hello %ld", i+1);
        // передаются данные
        // 1 - дескриптор  сокета, 2,3 - адрес  буфера, 4 - передачи дополнительных флагов, 5,6 - информация об адресе сервера и его длине  
        ssize_t err = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *) &addr, sizeof(addr));
        if (err == -1) {
            // сокет закрывается
            close(sock);
            perror("sendto failed");
            return EXIT_FAILURE;
        }
        printf("Sent num %ld\n", i + 1);
        sleep(rand() % 5+ 1);
    }

    return 0;
}

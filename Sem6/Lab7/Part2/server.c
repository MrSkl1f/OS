#include <signal.h>
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


static int cur_clients = 0;
static int sock = 0;

void signal_handler(int sig) {
    printf("\nCatched signal CTRL+C\n");
    // сокет закрывается
    close(sock);
    exit(0);
}

int new_client_handler(int* const clients)
{
    struct sockaddr_in addr;
    int addr_size = sizeof(addr);

    // Устанавливает соединение в ответ на запрос клиента и создает копию сокета для того, чтобы исходный сокет мог продолжать прослушивание
    int newsock = accept(sock, (struct sockaddr*) &addr, (socklen_t*) &addr_size);
    if (newsock < 0) {
        perror("accept failed");
        return EXIT_FAILURE;
    }

    cur_clients++;
    printf("\nClient %d (fd = %d)\n\n", cur_clients, newsock);

    if (cur_clients < MAX_CLIENTS) {
        clients[cur_clients - 1] = newsock;
    }
    else {
        perror("too many clients");
        return EXIT_FAILURE;   
    }

    return 0;
}

void process_clients(fd_set *set, int* const clients)
{
    char msg[MSG_LEN] = { 0 };
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int fd = clients[i];
        if ((fd > 0) && FD_ISSET(fd, set)) {
            ssize_t read_len = read(fd, msg, MSG_LEN);
            if (read_len == -1) {
                perror("read failed");
            }
            else if (read_len == 0) {
                printf("\nClient %d disconnected\n\n", i+1);
                close(fd);
                clients[i] = 0;
                cur_clients--;
            }
            else {
                msg[read_len] = '\0';
                printf("Message from %d: %s\n", i+1, msg);
            }
        }
    }   
}


int main(void)
{
    // создаёт конечную точку соединения и возвращает файловый дескриптор.
    // AF_INET Протоколы Интернет IPv4
    // SOCK_STREAM Обеспечивает создание двусторонних, 
    // надёжных потоков байтов на основе установления соединения.
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket failed");
        return EXIT_FAILURE;
    }
    // Для  сетевого  взаимодействия 
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        // Linux kernel может быть либо big endian, либо little endian, в зависимости от архитектуры
        // преобразовать число из порядка хоста в сетевой
        .sin_port = htons(PORT),
        // программа сервер зарегистрируется на всех адресах той  машины, на которой она выполняется
        .sin_addr.s_addr = INADDR_ANY    
    };
    // связывание сокета с некоторым адресом
    // 1 - дескриптор сокета, 2 - указатель на структуру struct sockaddr
    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind failed");
        return EXIT_FAILURE;
    }

    signal(SIGINT, signal_handler);
    // для каждого клиента у сервера должен быть открыт отдельный сокет
    // сообщает сокету, что должны приниматься новые соединения
    if (listen(sock, 3) < 0) {
        perror("listen failed");
        return EXIT_FAILURE;
    }
        
    
    printf("Listen port %d.\n", PORT);

    int clients[MAX_CLIENTS] = { 0 };
    while (1) {
        // набор дескрипторов
        fd_set set; 
        FD_ZERO(&set);
        FD_SET(sock, &set);
        int max_fd = sock;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int cur_cl = clients[i]; 
            if (cur_cl > 0) {
                FD_SET(cur_cl, &set);
            }
            
            max_fd = (cur_cl > max_fd) ? (cur_cl) : (max_fd);
        }
        
        // интервал времени, по прошествии которого она вернет управление в любом случае
        struct timeval timeout = {1, 500000};
        // может проверять  состояние нескольких  дескрипторов  сокетов  (или  файлов)  сразу
        // 1 - кол-во проверяемых, 2,3,4 - наборы  дескрипторов,  которые  следует  проверять
        // Вызов select() для проверки наличия входящих данных на сокете
        int activ = select(max_fd + 1, &set, NULL, NULL, &timeout);
        if (activ < 0) {
            perror("select failed");
            return EXIT_FAILURE;
        }
        
        if (FD_ISSET(sock, &set)) {
            int err = new_client_handler(clients);
            if (err != 0) {
                return err;
            }
        }

        process_clients(&set, clients);
    }

    return 0;
}

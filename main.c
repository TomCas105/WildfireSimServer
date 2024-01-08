#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "pos_sockets/active_socket.h"
#include "pos_sockets/passive_socket.h"

typedef struct thread_data {
    short port;
    ACTIVE_SOCKET *my_socket;
    char *clientMap;
};

void thread_data_init(struct thread_data *data, short port, ACTIVE_SOCKET *my_socket) {
    data->port = port;
    data->my_socket = my_socket;
    data->clientMap = "";
}

void thread_data_destroy(struct thread_data *data) {
    data->port = 0;
    data->my_socket = NULL;
    data->clientMap = NULL;
}

void send_map(ACTIVE_SOCKET *socket, char *data) {
    // Write map data to the socket
    struct char_buffer mapBuffer;

    char_buffer_init(&mapBuffer);
    char_buffer_append(&mapBuffer, data, strlen(data));
    active_socket_write_data(socket, &mapBuffer);
    printf("Mapa poslana!\n");
    char_buffer_destroy(&mapBuffer);
}

void *process_client_data(void *thread_data) {
    struct thread_data *data = thread_data;
    PASSIVE_SOCKET p_socket;
    passive_socket_init(&p_socket);
    passive_socket_start_listening(&p_socket, data->port);
    printf("zacina cakat\n");
    passive_socket_wait_for_client(&p_socket, data->my_socket);
    passive_socket_stop_listening(&p_socket);
    passive_socket_destroy(&p_socket);
    printf("Klient bol pripojeny!\n");
    active_socket_start_reading(data->my_socket);
    return NULL;
}

void receiveData(struct thread_data data, struct char_buffer receivedBuffer) {
    data.clientMap = receivedBuffer.data;
}

void sendData(struct thread_data data) {
    struct char_buffer sendBuffer;
    char_buffer_init(&sendBuffer);
    sendBuffer.data = data.clientMap;

    active_socket_write_data(data.my_socket, &sendBuffer);

    char_buffer_destroy(&sendBuffer);
}

void consume(struct thread_data data) {
    while (true) {
        struct char_buffer receivedBuffer;
        char_buffer_init(&receivedBuffer);

        if (active_socket_try_get_read_data(data.my_socket, &receivedBuffer)) {
            if (receivedBuffer.size > 0) {
                printf("%s", receivedBuffer.data);

                char *selector = (char *) malloc(5);
                char *receivedData = (char *) malloc(receivedBuffer.size - 5);

                strncpy(selector, receivedBuffer.data, 4);
                strncpy(receivedData, receivedBuffer.data + 6, receivedBuffer.size - 5);

                if (selector == "save") {
                    printf("%s", "prijimam data");
                } else if (selector == "load") {
                    sendData(data);
                } else if (selector == "exit") {
                    break;
                }

                free(selector);
            }
        }

        char_buffer_destroy(&receivedBuffer);
        sleep(1);
    }
}

int main() {
    pthread_t th_receive;
    struct thread_data data;
    struct active_socket my_socket;
    active_socket_init(&my_socket);
    thread_data_init(&data, 11889, &my_socket);
    pthread_create(&th_receive, NULL, process_client_data, &data);
    consume(data);
    pthread_join(th_receive, NULL);
    thread_data_destroy(&data);
    active_socket_destroy(&my_socket);
    return 0;
}
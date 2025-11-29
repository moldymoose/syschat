#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <getopt.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define VPRINTF(fmt, ...) \
    do { \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
    } while (0)

#define BUF_SIZE 4096 
#define USERNAME_MAX 24
#define MAX_CLIENTS 256

typedef struct {
    char address[INET_ADDRSTRLEN];
    int port;
    bool is_server;
} config_t;

#define PROTO_VERSION 1
typedef enum {
    PROTO_HANDSHAKE,
    PROTO_MESSAGE,
} proto_type_e;

typedef struct {
    proto_type_e type;
    unsigned int length;
} proto_header_t;

int create_socket();
bool prompt(char* buffer, size_t size, char* message);
void flush_input(void);
void start_server(config_t config);
void start_client(config_t config);
void send_handshake(int fd);
void receive_handshake(int fd);
void client_event_loop(int socket_fd);
void initialize_clients();
int find_free_client_slot();
void server_event_loop(int listening_fd);
void send_message(int fd, const char* message);
void receive_message(int fd);

#endif

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define PROTO_HEADER_SIZE 8

// Protocol header types
typedef enum {
    PROTO_HANDSHAKE = 1,
    PROTO_MESSAGE   = 2,
    PROTO_USERNAME  = 3,
} proto_type_e;


// Config for connection
typedef struct {
    char address[INET_ADDRSTRLEN];
    int port;
    int is_server;
} config_t;

int create_socket();

int prompt(char* buffer, size_t size, char* message);
void flush_input(void);

void start_server(config_t config);
void start_client(config_t config);
void client_event_loop(int socket_fd);
void server_event_loop(int listening_fd);

void initialize_clients();
int find_free_client_slot();


ssize_t write_all(int fd, const void *buf, size_t len);
ssize_t read_all(int fd, void *buf, size_t len);
int send_proto_message(int fd, proto_type_e type, const void *payload, uint32_t length);
int recv_proto_message(int fd, proto_type_e *type, void **payload, uint32_t *length);

#endif

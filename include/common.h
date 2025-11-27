#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include <getopt.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define VPRINTF(fmt, ...) \
    do { \
        fprintf(stderr, fmt, ##__VA_ARGS__); \
    } while (0)

#define BUF_SIZE 1024
#define USERNAME_MAX 24

typedef struct {
    char address[INET_ADDRSTRLEN];
    int port;
    bool is_server;
} config_t;

int create_socket();
bool prompt(char* buffer, size_t size, char* message);
void flush_input(void);
void start_server(config_t config);
void start_client(config_t config);

#endif

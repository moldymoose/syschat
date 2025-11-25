#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 7777
#define BUF_SIZE 1024
#define USERNAME_MAX 24

int create_socket();
bool prompt(char* buffer, size_t size, char* message);
void flush_input(void);

#endif

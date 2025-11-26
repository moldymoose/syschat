#include "common.h"

int create_socket() {
    VPRINTF("Creating socket\n");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

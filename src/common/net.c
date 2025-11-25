#include "common.h"

int create_socket() {
    // Tells operating system to get a new ipv4 (AF_INET) TCP (SOCK_STREAM) socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock;
}

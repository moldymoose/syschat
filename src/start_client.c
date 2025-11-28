#include "common.h"

void start_client(config_t config) {
    int socket_fd;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(config.port);
    server_addr.sin_addr.s_addr = inet_addr(config.address);

    // Create socket
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }

    VPRINTF("Connected to server %s:%d\n", config.address, config.port);
    //receive_handshake(socket_fd);
    client_event_loop(socket_fd);

    // Close socket
    close(socket_fd);
}
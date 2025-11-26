#include "common.h"

void start_server(config_t config) {
    VPRINTF("Starting server at %s:%d\n", config.address, config.port);

    int server_fd = create_socket();

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(config.port);
    address.sin_addr.s_addr = INADDR_ANY;

    VPRINTF("Binding socket\n");
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    VPRINTF("Listening on socket\n");
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    VPRINTF("Server listening on %s:%d\n", config.address, config.port);

    close(server_fd);
}
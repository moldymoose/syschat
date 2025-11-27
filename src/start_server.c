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

    while(true) {
        VPRINTF("Waiting for client connections...\n");
        struct sockaddr_in client_address;
        socklen_t client_addr_len = sizeof(client_address);

        int client_fd = accept(server_fd, (struct sockaddr*)&client_address, &client_addr_len);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }
        VPRINTF("Client %d connected: %s:%d\n", client_fd, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        close(client_fd);
    }

    close(server_fd);
}
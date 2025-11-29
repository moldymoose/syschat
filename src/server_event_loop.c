#include "protocol.h"

typedef enum {
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED
} state_e;

typedef struct {
    int fd;
    state_e state;
    char buffer[BUF_SIZE];
    char username[USERNAME_MAX];
} client_state_t;

client_state_t clients[MAX_CLIENTS];

void initialize_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Set all client slots to unused
        clients[i].fd = -1;
        clients[i].state = STATE_NEW;
        memset(clients[i].buffer, 0, BUF_SIZE);
        memset(clients[i].username, 0, USERNAME_MAX);
    }
}

int find_free_client_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd == -1) {
            return i;
        }
    }
    return -1; // No free slot found
}

void server_event_loop(int listening_fd) {
    VPRINTF("Entering server event loop on fd %d\n", listening_fd);

    int connection_fd, num_fds, next_available_slot;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    fd_set read_fds, write_fds;

    initialize_clients();

    while(true) {
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        // Add listening socket to read set
        FD_SET(listening_fd, &read_fds);
        num_fds = listening_fd + 1;

        // Add active client sockets to read set
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd != -1) {
                FD_SET(clients[i].fd, &read_fds);
                if (clients[i].fd >= num_fds) {
                    num_fds = clients[i].fd + 1;
                }
            }
        }

        // Wait for activity on any socket
        int activity = select(num_fds, &read_fds, &write_fds, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }

        if (FD_ISSET(listening_fd, &read_fds)) {
            connection_fd = accept(listening_fd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (connection_fd < 0) {
                perror("Accept failed");
                continue;
            }

            next_available_slot = find_free_client_slot();
            if (next_available_slot != -1) {
                clients[next_available_slot].fd = connection_fd;
                clients[next_available_slot].state = STATE_CONNECTED;
                VPRINTF("New client connected on fd %d, assigned to slot %d\n", connection_fd, next_available_slot);
            } else {
                VPRINTF("Max clients reached. Rejecting connection on fd %d\n", connection_fd);
                close(connection_fd);
            }
        }

        if (FD_ISSET(connection_fd, &read_fds)) {
            proto_type_e msg_type;
            void *payload = NULL;
            uint32_t length = 0;
            if (recv_proto_message(connection_fd, &msg_type, &payload, &length) == 0) {
                if(msg_type == PROTO_MESSAGE) {
                    char* message = (char*)payload;
                    VPRINTF("Received message from client fd %d: %s\n", connection_fd, message);
                } else {
                    VPRINTF("Received unknown message type %d from client fd %d\n", msg_type, connection_fd);
                }
                free(payload);
            } else {
                VPRINTF("Error receiving message from client fd %d\n", connection_fd);
                close(connection_fd);
                // Mark client slot as free
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == connection_fd) {
                        clients[i].fd = -1;
                        clients[i].state = STATE_DISCONNECTED;
                        break;
                    }
                }
            }
        }
    }
    VPRINTF("Exiting server event loop on fd %d\n", listening_fd);
    close(listening_fd);
}
#include "protocol.h"

// States that a client can be in
typedef enum {
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED
} state_e;

// Struct stores client state, fd, and username
typedef struct {
    int fd;
    state_e state;
    char buffer[BUF_SIZE];
    char username[USERNAME_MAX];
} client_state_t;

// Initialize client state array
client_state_t clients[MAX_CLIENTS];

// Lookup table for finding client by fd
static client_state_t *fd_table[FD_SETSIZE];
// Active fds array and count
static int active_fds[MAX_CLIENTS];
static int active_count = 0;

// Set all clients to -1 (unused) and zero buffers
void initialize_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].state = STATE_NEW;
        memset(clients[i].buffer, 0, BUF_SIZE);
        memset(clients[i].username, 0, USERNAME_MAX);
    }

    // Initialize fd lookup table
    for (int i = 0; i < FD_SETSIZE; ++i) {
        fd_table[i] = NULL;
    }
    active_count = 0; // Reset active count
}

// Find the next free client slot, return index or -1 if full
int find_free_client_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd == -1) {
            return i;
        }
    }
    return -1; // No free slot found
}

// listening_fd: socket fd to listen for new connections
void server_event_loop(int listening_fd) {
    VPRINTF("Entering server event loop on fd %d\n", listening_fd);

    // connection_fd: fd for new client connections
    // num_fds: max fd for select
    // next_available_slot: index of next free client slot
    int connection_fd, num_fds, next_available_slot;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    fd_set read_fds, write_fds;

    initialize_clients();

    while(true) {
        // Clear fd sets
        FD_ZERO(&read_fds);
        FD_ZERO(&write_fds);

        // Add listening socket to read set
        FD_SET(listening_fd, &read_fds);
        num_fds = listening_fd + 1; // fds are 0-indexed

        // Add active fds to read set
        for (int i = 0; i < active_count; ++i) {
            int fd = active_fds[i];
            FD_SET(fd, &read_fds);
            if (fd >= num_fds) num_fds = fd + 1;
        }

        // Wait for activity on any socket
        int activity = select(num_fds, &read_fds, &write_fds, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }

        // If listening socket is readable, new connection is incoming
        if (FD_ISSET(listening_fd, &read_fds)) {
            connection_fd = accept(listening_fd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (connection_fd < 0) {
                perror("Accept failed");
                continue;
            }

            // Assign new client to a free slot
            next_available_slot = find_free_client_slot();
            // If a slot is available, initialize client state
            if (next_available_slot != -1) {
                clients[next_available_slot].fd = connection_fd;
                clients[next_available_slot].state = STATE_CONNECTED;
                VPRINTF("New client connected on fd %d, assigned to slot %d\n", connection_fd, next_available_slot);
                // Add to fd lookup table
                if (connection_fd < FD_SETSIZE) {
                    fd_table[connection_fd] = &clients[next_available_slot];
                    if (active_count < MAX_CLIENTS) {
                        active_fds[active_count++] = connection_fd;
                    } else {
                        VPRINTF("Active array overflow, closing fd %d\n", connection_fd);
                        fd_table[connection_fd] = NULL;
                        clients[next_available_slot].fd = -1;
                        clients[next_available_slot].state = STATE_NEW;
                        close(connection_fd);
                    }
                } else {
                    VPRINTF("Received fd %d >= FD_SETSIZE; closing\n", connection_fd);
                    clients[next_available_slot].fd = -1;
                    clients[next_available_slot].state = STATE_NEW;
                    close(connection_fd);
                }
            } else {
                VPRINTF("Max clients reached. Rejecting connection on fd %d\n", connection_fd);
                close(connection_fd);
            }
        }

        // Loop through active fds to check for messages
        for (int i = 0; i < active_count; ++i) {
            int fd = active_fds[i];
            // If fd is not set in read_fds, skip
            if (!FD_ISSET(fd, &read_fds)) continue;

            // Receive protocol message
            proto_type_e msg_type;
            void *payload = NULL;
            uint32_t length = 0;
            if (recv_proto_message(fd, &msg_type, &payload, &length) == 0) {
                switch (msg_type) {
                    case PROTO_MESSAGE: {
                        char *message = (char *)payload;
                        client_state_t* active_client = fd_table[fd];
                        // Log received message (unknown user if no username set --only possible from frontend clients)
                        VPRINTF("Received message from client fd %d: user %s: %s\n", fd, active_client ? active_client->username : "unknown", message);

                        // Broadcast message to all connected clients
                        char broadcast_buffer[BUF_SIZE];
                        // Prepend username to message and get length
                        int broadcast_len = snprintf(broadcast_buffer, BUF_SIZE, "%s: %s", active_client ? active_client->username : "unknown", message);

                        // Send to all active clients
                        if (broadcast_len > 0 && broadcast_len < BUF_SIZE) {
                            for (int j = 0; j < active_count; ++j) {
                                int client_fd = active_fds[j];
                                if (send_proto_message(client_fd, PROTO_MESSAGE, broadcast_buffer, broadcast_len) != 0) {
                                    VPRINTF("Failed to broadcast message to client fd %d\n", client_fd);
                                }
                            }
                        }
                        break;
                    }
                    case PROTO_USERNAME: {
                        // Set username for client
                        char *username = (char *)payload;
                        VPRINTF("Client fd %d set username to: %s\n", fd, username);
                        client_state_t *active_client = fd_table[fd];
                        if (active_client) {
                            strncpy(active_client->username, username, USERNAME_MAX - 1);
                            active_client->username[USERNAME_MAX - 1] = '\0';
                        }
                        break;
                    }
                    default:
                        VPRINTF("Received unknown message type %d from client fd %d\n", msg_type, fd);
                        break;
                }
                free(payload);
            } else {
                VPRINTF("Error or disconnect receiving from client fd %d\n", fd);
                close(fd);

                // Update client state to disconnected and remove from active lists
                client_state_t *active_client = fd_table[fd];
                if (active_client) {
                    active_client->fd = -1;
                    active_client->state = STATE_DISCONNECTED;
                }

                fd_table[fd] = NULL;
                // Remove fd from active_fds by swapping with last and decrementing count
                active_fds[i] = active_fds[active_count - 1];
                active_count--;
                --i;
            }
        }
    }
    // Probably unreachable since server has no exit
    VPRINTF("Exiting server event loop on fd %d\n", listening_fd);
    close(listening_fd);
}
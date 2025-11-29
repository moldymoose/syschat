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

/* O(1) lookup -----------------------------------------------------------------
 * We keep an FD-indexed table so we can find the client_state by socket fd in
 * constant time. select() / FD_SETSIZE typically restricts the maximum fd you
 * can manage — we size the table by FD_SETSIZE and track active_fds to iterate
 * only currently connected sockets.
 */
static client_state_t *fd_table[FD_SETSIZE];
static int active_fds[MAX_CLIENTS];
static int active_count = 0;

void initialize_clients() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Set all client slots to unused
        clients[i].fd = -1;
        clients[i].state = STATE_NEW;
        memset(clients[i].buffer, 0, BUF_SIZE);
        memset(clients[i].username, 0, USERNAME_MAX);
    }

    /* initialize fd lookup table */
    for (int i = 0; i < FD_SETSIZE; ++i) {
        fd_table[i] = NULL;
    }
    active_count = 0;
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

        /* Add only active fds (O(active_count)) and update maxfd */
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
                /* Add O(1) lookup and append to active list */
                if (connection_fd < FD_SETSIZE) {
                    fd_table[connection_fd] = &clients[next_available_slot];
                    /* append fd to active_fds */
                    if (active_count < MAX_CLIENTS) {
                        active_fds[active_count++] = connection_fd;
                    } else {
                        /* shouldn't happen — cleanup and close */
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
        /* Iterate active fds and handle readable clients. Using the fd_table
         * lets us lookup client_state in O(1) time. When a client disconnects
         * we remove from fd_table and shrink active_fds by swapping with last. */
        for (int i = 0; i < active_count; ++i) {
            int fd = active_fds[i];
            if (!FD_ISSET(fd, &read_fds)) continue;

            proto_type_e msg_type;
            void *payload = NULL;
            uint32_t length = 0;
            if (recv_proto_message(fd, &msg_type, &payload, &length) == 0) {
                switch (msg_type) {
                    case PROTO_MESSAGE: {
                        char *message = (char *)payload;
                        client_state_t* st = fd_table[fd];
                        VPRINTF("Received message from client fd %d: user %s: %s\n", fd, st ? st->username : "unknown", message);
                        break;
                    }
                    case PROTO_USERNAME: {
                        char *username = (char *)payload;
                        VPRINTF("Client fd %d set username to: %s\n", fd, username);
                        client_state_t *st = fd_table[fd];
                        if (st) {
                            strncpy(st->username, username, USERNAME_MAX - 1);
                            st->username[USERNAME_MAX - 1] = '\0';
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

                /* cleanup: mark client slot free and remove from fd_table + active_fds */
                client_state_t *st = fd_table[fd];
                if (st) {
                    st->fd = -1;
                    st->state = STATE_DISCONNECTED;
                }

                fd_table[fd] = NULL;
                /* remove fd from active_fds by swapping with last */
                active_fds[i] = active_fds[active_count - 1];
                active_count--;
                --i; /* re-check the swapped-in fd on next loop iteration */
            }
        }
    }
    VPRINTF("Exiting server event loop on fd %d\n", listening_fd);
    close(listening_fd);
}
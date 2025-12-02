#include "protocol.h"

// Circular buffer for storing last few messages (set by MAX_STORED_MESSAGES)
typedef struct {
    char messages[MAX_STORED_MESSAGES][BUF_SIZE];
    int count;  // Total messages recieved by client (not total stored)
    int head;   // Index where next message will be written
} message_buffer_t;

// Sets number of messages to 0, location of Index to beginning, and clears all data in message buffer
static void init_message_buffer(message_buffer_t *buf) {
    buf->count = 0;
    buf->head = 0;
    memset(buf->messages, 0, sizeof(buf->messages));
}


static void add_message(message_buffer_t *buf, const char *message) {
    // Clear the slot first to avoid leftover data
    memset(buf->messages[buf->head], 0, BUF_SIZE);
    // Copy the message at the head location
    strncpy(buf->messages[buf->head], message, BUF_SIZE - 1);
    buf->messages[buf->head][BUF_SIZE - 1] = '\0';
    // Increment the head (but roll over to beginning if max is reached)
    buf->head = (buf->head + 1) % MAX_STORED_MESSAGES;
    buf->count++;
}

static void display_messages(message_buffer_t *buf, const char *username) {
    // Clear screen using ansi escape code macro, and print header
    CLEAR_SCREEN();
    printf("=== Welcome to SysChat (User: %s) ===\n", username);
    printf("Last %d messages:\n\n", buf->count < MAX_STORED_MESSAGES ? buf->count : MAX_STORED_MESSAGES);
    
    // Calculate starting position for display
    int start_index;
    int num_to_display;
    
    // If fewer messages than max stored, start from 0 and end at count
    if (buf->count <= MAX_STORED_MESSAGES) {
        start_index = 0;
        num_to_display = buf->count;
    // Otherwise, start from head and display max stored messages
    } else {
        start_index = buf->head;
        num_to_display = MAX_STORED_MESSAGES;
    }
    
    // Display messages in order
    for (int i = 0; i < num_to_display; i++) {
        // Start from start_index and roll over
        int index = (start_index + i) % MAX_STORED_MESSAGES;
        printf("%s\n", buf->messages[index]);
    }
    
    // Print prompt for user input
    printf("\n> ");
    fflush(stdout);
}

void client_event_loop(int socket_fd) {
    VPRINTF("Entering client event loop on fd %d\n", socket_fd);
    // Prompt for username
    char username[USERNAME_MAX] = {0};
    while(!prompt(username, USERNAME_MAX, "Please Enter Your Name: "));

    send_proto_message(socket_fd, PROTO_USERNAME, username, strlen(username));
    
    // Initialize message buffer
    message_buffer_t msg_buffer;
    init_message_buffer(&msg_buffer);
    
    // Display initial empty chat screen
    display_messages(&msg_buffer, username);

    while(true) {

        // Set up file descriptor to monitor for input
        fd_set read_fds;

        // Clear the set
        FD_ZERO(&read_fds);
        
        // Add stdin and socket_fd to the set
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(socket_fd, &read_fds);

        // Highest file descriptor number
        int max_fd = socket_fd > STDIN_FILENO ? socket_fd : STDIN_FILENO;

        // Wait for activity on either stdin or socket
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("select error");
            break;
        }

        // Check if there's input from stdin
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            char input_buffer[BUF_SIZE] = {0};
            if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
                input_buffer[strcspn(input_buffer, "\n")] = '\0';
                int length = strlen(input_buffer);
                if (length > 0) {
                    send_proto_message(socket_fd, PROTO_MESSAGE, input_buffer, length);
                    // Redisplay after sending --probably unnecessary, since server will echo back
                    display_messages(&msg_buffer, username);
                }
            }
        }

        // Check if there's input from the server
        if (FD_ISSET(socket_fd, &read_fds)) {
            // Message from server
            proto_type_e msg_type;
            void *payload = NULL;
            uint32_t length = 0;
            
            // Receive message (dynamically allocates payload)
            if (recv_proto_message(socket_fd, &msg_type, &payload, &length) == 0) {
                if (msg_type == PROTO_MESSAGE) {
                    char *message = (char *)payload;
                    // Add message to buffer
                    add_message(&msg_buffer, message);
                    // Clear screen and redisplay all messages
                    display_messages(&msg_buffer, username);
                }
                // Free the allocated payload
                free(payload);
            } else {
                // Connection closed or error
                printf("\nDisconnected from server.\n");
                close(socket_fd);
                break;
            }
        }
    }

    // Probably unreachable
    VPRINTF("Exiting client event loop on fd %d\n", socket_fd);
}
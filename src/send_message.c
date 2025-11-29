#include "common.h"

void send_message(int fd, const char* message) {
    VPRINTF("Sending message to fd %d\n", fd);

    if(strlen(message) > BUF_SIZE - sizeof(proto_header_t)) {
        VPRINTF("Message too long to send to fd %d\n", fd);
        return;
    }

    char buffer[BUF_SIZE] = {0};
    proto_header_t* header = (proto_header_t*)buffer;

    header->type = PROTO_MESSAGE;
    int real_length = strlen(message);
    header->length = htonl(real_length);

    char* data = (char*)&header[1];
    strncpy(data, message, real_length);
    write(fd, header, sizeof(proto_header_t) + real_length);

    VPRINTF("Message sent to fd %d: %s\n", fd, message);
}
#include "common.h"

void send_handshake(int fd) {
    VPRINTF("Sending handshake to fd %d\n", fd);

    // Buffer is just raw bytes
    char buffer[BUF_SIZE] = {0};
    // Header points to the start of the buffer
    proto_header_t* header = (proto_header_t*)buffer;

    header->type = PROTO_HANDSHAKE;
    int real_length = sizeof(int);
    header->length = htonl(real_length);

    // Data is integer immediately after header
    int* data = (int*)&header[1];
    *data = htonl(PROTO_VERSION); // Handshake version
    write(fd, header, sizeof(proto_header_t) + real_length);

    VPRINTF("Handshake sent to fd %d\n", fd);
}
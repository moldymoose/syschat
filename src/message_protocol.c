#include "protocol.h"

ssize_t write_all(int fd, const void *buf, size_t len) {
    size_t sent = 0;
    // Loop until all bytes are sent
    while (sent < len) {
        ssize_t n = write(fd, (const char*)buf + sent, len - sent);
        if (n <= 0) return n; // error
        sent += n;
    }
    return sent;
}

ssize_t read_all(int fd, void *buf, size_t len) {
    size_t received = 0;
    // Loop until all bytes are received
    while (received < len) {
        ssize_t n = read(fd, (char*)buf + received, len - received);
        if (n <= 0) return n; // error or EOF
        received += n;
    }
    return received;
}

int send_proto_message(int fd, proto_type_e type, const void *payload, uint32_t length) {
    uint8_t header[PROTO_HEADER_SIZE];

    // Construct and pad header
    header[0] = type;
    header[1] = 0;
    header[2] = 0;
    header[3] = 0;

    // Length is 4 bytes so we use the last 4 bytes of the header
    uint32_t nlen = htonl(length);
    memcpy(&header[4], &nlen, 4);

    // Send header
    if (write_all(fd, header, PROTO_HEADER_SIZE) <= 0)
        return -1;

    // Send payload if present
    if (length > 0 && write_all(fd, payload, length) <= 0)
        return -1;

    return 0;
}

int recv_proto_message(int fd, proto_type_e *type, void **payload, uint32_t *length) {
    uint8_t header[PROTO_HEADER_SIZE];

    // Read header
    if (read_all(fd, header, PROTO_HEADER_SIZE) <= 0)
        return -1;

    // Parse header
    *type = header[0];
    // Length is in last 4 bytes
    *length = ntohl(*(uint32_t*)&header[4]);

    // Allocate one extra byte for null-termination
    *payload = malloc(*length + 1);
    if (*payload == NULL)
        return -1;

    // Read payload if present
    if (*length > 0 && read_all(fd, *payload, *length) <= 0) {
        free(*payload);
        return -1;
    }
    // Null-terminate the payload
    ((char *)(*payload))[*length] = '\0';

    return 0;
}

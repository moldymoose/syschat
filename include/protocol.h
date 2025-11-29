#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "common.h"

#define PROTO_HEADER_SIZE 8

typedef enum {
    PROTO_HANDSHAKE = 1,
    PROTO_MESSAGE   = 2,
} proto_type_e;

ssize_t write_all(int fd, const void *buf, size_t len);
ssize_t read_all(int fd, void *buf, size_t len);
int send_proto_message(int fd, proto_type_e type, const void *payload, uint32_t length);
int recv_proto_message(int fd, proto_type_e *type, void **payload, uint32_t *length);

#endif
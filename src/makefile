CC = xlc
CFLAGS = -qflag=w -qinfo=all -g
BUILD = build

SRC = client_event_loop.c flush_input.c main.c message_protocol.c \
      net.c prompt.c server_event_loop.c start_client.c start_server.c

OBJ = $(SRC:.c=.o)
DBG = $(SRC:.c=.dbg)

syschat: $(OBJ)
    $(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c include/common.h
    $(CC) $(CFLAGS) -c -g $<

clean:
    rm -f $(OBJ) $(DBG) syschat
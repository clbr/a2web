.PHONY: all clean

LIBS = -lxmlrpc_client -lfcgi

FILES = $(wildcard *.c)
OBJS = $(FILES:.c=.o)

CFLAGS += -Wall -Wextra
LDFLAGS += -Wl,-O1

NAME = a2web.cgi

all: $(NAME)

$(NAME): $(OBJS)
	gcc -o $(NAME) $(LDFLAGS) $(CFLAGS) $(LIBS) $(OBJS)


clean:
	rm -f $(NAME) *.o

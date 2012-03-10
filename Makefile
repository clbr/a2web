.PHONY: all clean copy

LIBS = -lxmlrpc_client -lfcgi

FILES = $(wildcard *.c)
OBJS = $(FILES:.c=.o)

CFLAGS += -Wall -Wextra
LDFLAGS += -Wl,-O1

NAME = a2web.cgi

# Dir for testing
COPYDIR ?= /tmp


all: $(NAME)

$(NAME): $(OBJS)
	gcc -o $(NAME) $(LDFLAGS) $(CFLAGS) $(LIBS) $(OBJS)


clean:
	rm -f $(NAME) *.o

copy: all
	mkdir -p $(COPYDIR)/cgi-bin $(COPYDIR)/a2web/themes
	cp $(NAME) $(COPYDIR)/cgi-bin
	cp -a themes/* $(COPYDIR)/a2web/themes

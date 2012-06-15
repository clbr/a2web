.PHONY: all clean copy

LIBS = -lxmlrpc_client -lfcgi

FILES = $(wildcard *.c)
OBJS = $(FILES:.c=.o)

CFLAGS += -Wall -Wextra -ffunction-sections -fdata-sections
LDFLAGS += -Wl,-O1 -Wl,-gc-sections

NAME = a2web.cgi

# Dir for testing
COPYDIR ?= /tmp
THEMEDIR ?= $(COPYDIR)


all: $(NAME)

$(NAME): $(OBJS)
	$(CC) -o $(NAME) $(LDFLAGS) $(CFLAGS) $(LIBS) $(OBJS)


clean:
	rm -f $(NAME) *.o

copy: all
	mkdir -p $(COPYDIR)/cgi-bin $(THEMEDIR)/a2web
	cp $(NAME) $(COPYDIR)/cgi-bin
	cp -a themes/* $(THEMEDIR)/a2web

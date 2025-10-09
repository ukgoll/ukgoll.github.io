CC = gcc
CFLAGS = -Wall -g
UNAME_S := $(shell uname)

ifeq ($(UNAME_S), Darwin)
aco_httpd: ./lib/aco.c ./lib/acosw.S main.c logger.c utils.c unix_kqueue.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o aco_httpd
else
aco_httpd: ./lib/aco.c ./lib/acosw.S main.c logger.c utils.c unix_epoll.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o aco_httpd
endif


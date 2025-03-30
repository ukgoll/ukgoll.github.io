CC = clang
CFLAGS = -Wall -Wextra -O2
SRCS = utils.c no_blocking_tcp_client.c
OBJS = $(SRCS:.c=.o)

nbtc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^


# 让 .o 依赖相应的 .c 和 .h 文件
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 显式指定 .h 依赖关系（如果 utils.h 变了，相关 .o 文件也会重新编译）
utils.o: utils.h
no_blocking_tcp_client.o: utils.h

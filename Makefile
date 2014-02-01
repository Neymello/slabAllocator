OBJS=main.o kmem.o
CFLAGS=-g -I. -o3 -Wall -Wextra -lpthread -ggdb3
#CFLAGS=-g -I. -o3 -lpthread -ggdb3
BIN=main
CC=gcc

%.o:%.c
	$(CC) $(CFLAGS) $(DEFINES) -o $@ -c $< 

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(DEFINES) -o $(BIN) $^

clean:
	rm $(BIN) $(OBJS)

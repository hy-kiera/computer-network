CC = gcc
TARGET = server client
CFLAGS = -W -Wall

all : $(TARGET)

server: server.c
		$(CC) -g -o server server.c

client: client.c
		$(CC) -g -o client client.c

clean:
		rm $(TARGET)	

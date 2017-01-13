CFLAGS = -lm -std=c99 -pedantic-errors -Wall -g
CC = gcc

all:  Client Server

clean:
	-rm utils.o server.o client.o server client

Client: header.h client.o utils.o
	gcc  -o client client.o utils.o $(CFLAGS) 

Server: header.h server.o utils.o
	gcc  -o server server.o utils.o $(CFLAGS) 

Client.o: header.h client.c
	gcc  -c client.c $(CFLAGS)
	
Server.o: header.h server.c
	gcc  -c  server.c $(CFLAGS)
	
Utils.o: header.h utils.c
	gcc  -c  utils.c $(CFLAGS)
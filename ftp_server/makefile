# This is the makefile for the socket_server program
socket_server : main.o my_server.o
	cc -o socket_server main.o my_server.o
main.o : main.c my_server.h
	cc -c main.c
my_server.o : my_server.c my_server.h
	cc -c my_server.c
	
clean:
	rm *.o socket_server

# This is the makefile for socket_client
socket_client : main.o my_client.o
	    cc -o socket_client main.o my_client.o
main.o : main.c my_client.h
	    cc -c main.c
my_client.o : my_client.c my_client.h
	    cc -c my_client.c
    
clean:
	    rm *.o socket_client


# default target
all: client server
    
# target for removing all temporary files
clean:
	rm -vf client server *.o

# programs to build
client: client.o networking.o
    
server: server.o networking.o

# dependencies to header files
client.o: networking.h
    
server.o: networking.h


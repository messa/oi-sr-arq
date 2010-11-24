
# default target
all: client server

# target for removing all temporary files
clean:
	rm -vf client server *.o

# programs to build
client: client.o networking.o window.o

server: server.o networking.o window.o

# dependencies to header files
client.o: networking.h

server.o: networking.h configuration.h window.h

networking.o: networking.h

window.o: window.h configuration.h


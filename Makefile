
# report more warnings
CFLAGS=-Wall -pedantic

# default target
all: client server

check: client server
	./run_tests.py

# target for removing all temporary files
clean:
	rm -vf client server *.o

# programs to build
client: client.o networking.o window.o util.o

server: server.o networking.o window.o util.o

# dependencies to header files
client.o: networking.h util.h

server.o: networking.h configuration.h window.h util.h

networking.o: networking.h

window.o: window.h configuration.h

util.o: util.h


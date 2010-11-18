
all: client server
    
clean:
	rm -vf client server *.o

    
client: client.o networking.o
    
server: server.o networking.o


client.o: networking.h
    
server.o: networking.h

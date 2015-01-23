CFLAGS=-W -Wall -Wextra -O0 -g
LDFLAGS=-lpthread


all:client server

# client
client:client.o
	$(CC) -o $@ $^ $(LDFLAGS)

client.o:client.c
	$(CC) -o $@  -c $< $(CFLAGS)


#server
server:server.o
	$(CC) -o $@ $^ $(LDFLAGS)

server.o:server-thread.c
	$(CC) -o $@  -c $< $(CFLAGS)



.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf client server


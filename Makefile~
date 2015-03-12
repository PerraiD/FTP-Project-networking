CFLAGS=-W -Wall -Wextra -O0 -g
LDFLAGS=-lpthread


all:client server

# client
client:client.o
	$(CC) -o $@ $^ $(LDFLAGS)

#server
server:server.o
	$(CC) -o $@ $^ $(LDFLAGS)


.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf client server


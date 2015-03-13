CFLAGS=-O0 -g
LDFLAGS=-lpthread


all:client server

# client
client:client.c libnet.c
	$(CC) -o $@ $< $(LDFLAGS)

#server
server:server.c libnet.c
	$(CC) -o $@ $< $(LDFLAGS)


.PHONY: clean mrproper

clean:
	rm -rf *.o

mrproper: clean
	rm -rf client server


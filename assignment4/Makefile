OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

server: fs_server.o
	gcc fs_server.o -o server.out

client: client-api.a fs_client.o
	gcc clien_tapp.out -L. -lclient_api.a

client-api.a: client_api.o
	ar r libclient_api.a client_api.o

$(OBJECTS): %.o: %.c ece454_fs.h
	gcc -c $< -o $@

clean:
	rm -rf a.out *.o core *.a fs_client

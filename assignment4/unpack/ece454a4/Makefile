OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))

server: fs_server.o
	gcc fs_server.o -L. simplified_rpc/libstubs.a -o server.out

client-api.a: libstubs_stuff client_api.o
	ar r libclient-api.a client_api.o simplified_rpc/server_stub.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o

fs_client: client-api.a fs_client.o
	gcc fs_client.o libclient-api.a -o fs_client

fs_client2: client-api.a fs_client2.o
	gcc fs_client2.o libclient-api.a -o fs_client2

libstubs_stuff:
	$(MAKE) -C simplified_rpc

$(OBJECTS): %.o: %.c ece454_fs.h
	gcc -c $< -o $@

clean:
	$(MAKE) -C simplified_rpc clean
	rm -rf a.out *.o core *.a fs_client server
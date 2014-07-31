OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
# LIB_DIR = /simplified_rpc 

all: client-api.a server

server: fs_server.o
	gcc fs_server.o -L. simplified_rpc/libstubs.a -o server.out

client: client-api.a fs_client.o
	gcc fs_client.o -L. libclient_api.a simplified_rpc/libstubs.a -o client.out

client-api.a: libstubs_task client_api.o
	ar r libclient-api.a client_api.o simplified_rpc/server_stub.o simplified_rpc/client_stub.o simplified_rpc/helper.o simplified_rpc/mybind.o

libstubs_task:
	$(MAKE) -C simplified_rpc

$(OBJECTS): %.o: %.c ece454_fs.h
	gcc -c $< -o $@

clean:
	$(MAKE) -C simplified_rpc clean
	rm -rf *.out *.o core *.a fs_client

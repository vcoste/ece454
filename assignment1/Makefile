objects := $(patsubst %.c,%.o,$(wildcard *.c))

server: libstubs.a serverapp.o
	gcc serverapp.o -L. -lstubs -o server.out

client: libstubs.a clientapp.o
	gcc clientapp.o -L. -lstubs -o client.out

libstubs.a: server_stub.o client_stub.o
	ar r libstubs.a server_stub.o client_stub.o

$(objects): %.o: %.c ece454rpc_types.h
	gcc -c $< -o $@

clean:
	rm -rf *.out *.o core *.a

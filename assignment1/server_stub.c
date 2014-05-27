#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "ece454rpc_types.h"

#define	PORT_RANGE_LO	10000
#define PORT_RANGE_HI	10100
#define BUF_SIZE		1024

func_node *functions;
int ret_int;
return_type r;

/* The following need to be implemented in the server stub */

/* register_procedure() -- invoked by the app programmer's server code
 * to register a procedure with this server_stub. Note that more than
 * one procedure can be registered */
bool register_procedure(const char *procedure_name, const int nparams, fp_type fnpointer) {
	//check if it already exists
	// if it doesn't add it to a linked list
	bool found = false;
	func_node *itr;
	func_node *newFunc;

	// TODO: compare by procedure names here
	for (itr = functions; itr; itr = itr->next) {
		if (fnpointer == itr->fnpointer) {
			found = true;
			break;
		}
	}

	if(!found) {
		newFunc = malloc(sizeof(*newFunc));
		newFunc->procedure_name = procedure_name;
		newFunc->nparams = nparams;
		newFunc->fnpointer = fnpointer;
		newFunc->next = functions;
		functions = newFunc;
		return true;	
	} else {
		return false;
	}
}

int mybind(int sockfd, struct sockaddr_in *addr) {
    if(sockfd < 1) {
		fprintf(stderr, "mybind(): sockfd has invalid value %d\n", sockfd);
		return -1;
    }

    if(addr == NULL) {
		fprintf(stderr, "mybind(): addr is NULL\n");
		return -1;
    }

    if(addr->sin_port != 0) {
		fprintf(stderr, "mybind(): addr->sin_port is non-zero. Perhaps you want bind() instead?\n");
		return -1;
    }

    unsigned short p;
    for(p = PORT_RANGE_LO; p <= PORT_RANGE_HI; p++) {
		addr->sin_port = htons(p);
		int b = bind(sockfd, (const struct sockaddr *)addr, sizeof(struct sockaddr_in));
		if(b < 0) {
		    continue;
		}
		else {
		    break;
		}
    }

    if(p > PORT_RANGE_HI) {
		fprintf(stderr, "mybind(): all bind() attempts failed. No port available...?\n");
		return -1;
    }

    /* Note: upon successful return, addr->sin_port contains, in network byte order, the
     * port to which we successfully bound. */
    return 0;
}

bool procedureExists(char *fn_name, fp_type *fp) {
	func_node *itr;

	for (itr = functions; itr; itr = itr->next) {
		// printf("name: %s, lenght:%lu\n", fn_name, strlen(fn_name));
		// printf("name: %s, lenght:%lu\n", itr->procedure_name, strlen(itr->procedure_name));
		if (strcmp(itr->procedure_name, fn_name) == 0) {
			*fp = itr->fnpointer;
			return true;
		}
	}
	return false;
}

bool parseBuffer(const void *buffer, arg_type **args, fp_type *fp, int *n_params) {
	char func_name[100];
	int i;
	void *ptrIdx = buffer;
	if (isalpha(*(char *)ptrIdx)) {
		strcpy(func_name, buffer); // get function name
		ptrIdx += strlen(func_name)+1;
		// printf("func_name: %s\n", func_name);
		if (!procedureExists(func_name, fp)) {
			printf("procedure does not exist: %s\n", func_name);
			return false;
		}
		printf("Parsing arguments for: %s\n", func_name);
		*n_params = *(int*)ptrIdx;
		ptrIdx += sizeof(int);

		for (i = 0; i < *n_params; ++i) {
			arg_type *temp = malloc(sizeof(*temp));

			temp->arg_size = *(int *)ptrIdx;
			ptrIdx += sizeof(int);

			temp->arg_val = malloc(temp->arg_size);
			memcpy(temp->arg_val, ptrIdx, temp->arg_size);
			ptrIdx += temp->arg_size;

			temp->next = *args;
			*args = temp;
		}

		return true;
	}
	printf("Not alpha\n");
	return false;
}

/* launch_server() -- used by the app programmer's server code to indicate that
	* it wants start receiving rpc invocations for functions that it registered
	* with the server stub.
	*
	* IMPORTANT: the first thing that should happen when launch_server() is invoked
	* is that it should print out, to stdout, the IP address/domain name and
	* port number on which it listens.
	*
	* launch_server() simply runs forever. That is, it waits for a client request.
	* When it receives a request, it services it by invoking the appropriate 
	* application procedure. It returns the result to the client, and goes back
	* to listening for more requests. 
*/
void launch_server() {
	int s, n, len = sizeof(struct sockaddr_in);
	char buf[BUF_SIZE];
	char *received_data = (char *)malloc(BUF_SIZE);
	struct sockaddr_in server, client;

	memset((char *) &server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = 0;
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	/* initialize socket */
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    	perror("socket");
    	return;
    }
    // bind socket
    if (mybind(s, &server) < 0) {
    	perror("Unable to bind to socket");
    }

    printf("Connected to port: %d\n", ntohs(server.sin_port));

    while ((n = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &client, &len)) != -1) {
    	printf("received a request\n");
    	int n_params;
    	arg_type *args;
    	fp_type fn_pointer;
    	return_type *result = malloc(sizeof(result));
    	char ret_buf[BUF_SIZE];

    	if (parseBuffer(buf, &args, &fn_pointer, &n_params)) {
    		printf("Buffer parsed\n");
    		
    		*result = fn_pointer(n_params, args);
    		memcpy(ret_buf, &result->return_size, sizeof(int));
    		memcpy((ret_buf + sizeof(int)), result->return_val, result->return_size);

    		printf("ret_buf.size: %d\n", result->return_size);
    		printf("ret_buf.val: %d\n", *(int *)(result->return_val));
    		sendto(s, ret_buf, sizeof(ret_buf), 0, (struct sockaddr *) &client, len);
    	} else {
    		printf("not parsed\n");
    		char * error_msg = "Error, function not found";
    		memcpy(ret_buf, error_msg, strlen(error_msg));
    		sendto(s, ret_buf, strlen((char*)ret_buf), 0, (struct sockaddr *) &client, len);
    	}
    }

	//ouput server and port
	// printf("ecelinux3.uwaterloo.ca 5764"); //this is just an example

	//start server and wait for procedure calls
	//when client request arrives, invoke it locally and return response to client

	//should not return
}
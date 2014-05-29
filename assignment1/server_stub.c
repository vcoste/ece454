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

/**
 * Invoked by the app programmer's server code to register a procedure
 * @param  procedure_name	name of the procedure to register
 * @param  nparams 			number of parameters in the procedure to register
 * @param  fnpointer 		function pointer to the procedure implementation
 * @return                  true if the procedure was registered successfully
 * 							false otherwise
 */
bool register_procedure(const char *procedure_name, 
						const int nparams, 
						fp_type fnpointer) {
	// Check if the procedure already exists
	// if it doesn't add it to a linked list
	bool found = false;
	func_node *itr;
	func_node *newFunc;

	for(itr = functions; itr; itr = itr->next) {
		if (strcmp(itr->procedure_name, procedure_name) == 0) {
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
		fprintf(stderr, 
				"mybind(): addr->sin_port is non-zero. Perhaps you want bind() instead?\n");
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
		fprintf(stderr, 
				"mybind(): all bind() attempts failed. No port available...?\n");
		return -1;
    }

    /* Note: upon successful return, addr->sin_port contains, in network byte order, the
     * port to which we successfully bound. */
    return 0;
}

/**
 * Checks the linked list of functions to see if the procedure already has been
 * added. Procedures can reference the same function as long as they have 
 * different names
 * @param  fn_name name of the procedure
 * @param  fp      pointer to the desired function
 * @return         true if function has been added to list, false if not
 */
bool procedureExists(char *fn_name, fp_type *fp) {
	func_node *itr;

	for(itr = functions; itr; itr = itr->next) {
		if (strcmp(itr->procedure_name, fn_name) == 0) {
			*fp = itr->fnpointer;
			return true;
		}
	}
	return false;
}

/**
 * Parses the buffer received from the client after a recvfrom has returned. 
 * The buffer expects the following format:
 * String[varible number of bits, null-terminated],
 * Number of params[Sizeof(int)], 
 * size of parameter[sizeof(int)], 
 * first parameter data[size is given at the previous point in the buffer]
 * @param  buffer   buffer received from recvfrom
 * @param  args     a pointer to an args struct pointer, this will be 
 * 					overwritten and populated with the values in the buffer
 * @param  fp       pointer to a function pointer, desired function is found in
 * 					the pointer list and this value is updated to point to the
 * 					correct function
 * @param  n_params number of parameters sent by the client, this value is overwritten
 * @return          returns true if parsed correctly, false otherwise
 */
bool parseBuffer(const void *buffer, arg_type **args, fp_type *fp, int *n_params) {
	char func_name[100];
	int i;
	void *ptrIdx = buffer;
	if(isalpha(*(char *)ptrIdx)) {
		// get function name
		strcpy(func_name, buffer); 
		ptrIdx += strlen(func_name)+1;
		if(!procedureExists(func_name, fp)) {
			return false;
		}
		*n_params = *(int*)ptrIdx;
		ptrIdx += sizeof(int);
		
		arg_type *temp, *tail;
		for(i = 0; i < *n_params; ++i) {
			temp = malloc(sizeof(*temp));
			temp->arg_size = *(int *)ptrIdx;
			ptrIdx += sizeof(int);

			temp->arg_val = malloc(temp->arg_size);
			memcpy(temp->arg_val, ptrIdx, temp->arg_size);
			ptrIdx += temp->arg_size;

			// append to list, create if list is empty
			if(*args) {
				tail->next = temp;
				tail = tail->next;
			} else {
				*args = temp;
				tail = temp;
			}
		}

		return true;
	}
	// printf("Not alpha\n");
	return false;
}

/**
 * Used by the app programmer's server code to indicate that it wants start 
 * receiving rpc invocations for functions that it registered with the 
 * server stub. This method runs forever waiting for client request and 
 * servicing them by sending the return value back to the client.
 */
void launch_server() {
	int s, n;
	socklen_t len = sizeof(struct sockaddr_in);
	char buf[BUF_SIZE];
	char *received_data = (char *)malloc(BUF_SIZE);
	struct sockaddr_in server, client;

	memset((char *) &server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = 0;
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	// initialize socket
    if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    	perror("socket");
    	return;
    }

    // bind socket
    if(mybind(s, &server) < 0) {
    	perror("Unable to bind to socket");
    }

    // printing IPaddress and port number
    printf(	"%s %d\n",	
    		inet_ntoa(server.sin_addr), 
			ntohs(server.sin_port)); 
	fflush(stdout);

    while((n = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &client, &len)) != -1) {
    	int n_params;
    	arg_type *args;
    	fp_type fn_pointer;
    	return_type *result = (return_type *)malloc(sizeof(result));
    	char ret_buf[BUF_SIZE];

    	if(parseBuffer(buf, &args, &fn_pointer, &n_params)) {
    		*result = fn_pointer(n_params, args);
    		memcpy(ret_buf, &result->return_size, sizeof(int));
    		memcpy((ret_buf + sizeof(int)), result->return_val, result->return_size);

    		sendto(s, ret_buf, sizeof(ret_buf), 0, (struct sockaddr *) &client, len);

    		free(args);
    		args = 0;
    	} else {
    		perror("not parsed\n");
    		char *error_msg = "Error, function not found";
    		memcpy(ret_buf, error_msg, strlen(error_msg));
    		sendto(s, ret_buf, strlen((char*)ret_buf), 0, (struct sockaddr *) &client, len);
    	}
    }
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdarg.h>

#include "ece454rpc_types.h"

#define BUF_SIZE 1024

//  The following needs to be implemented in the client stub. This is a
//  * procedure with a variable number of arguments that the app programmer's
//  * client code uses to invoke. The arguments should be self-explanatory.
//  *
//  * For each of the nparams parameters, we have two arguments: size of the
//  * argument, and a (void *) to the argument.

/**
 * [make_remote_call description]
 * @param  servernameorip
 * @param  serverportnumber
 * @param  procedure_name
 * @param  nparams
 * @return
 */
return_type make_remote_call(	const char *servernameorip,
	                            const int serverportnumber,
	                            const char *procedure_name,
	                            const int nparams,
			    				...) {
	// setup UDP connection here
	struct sockaddr_in server;
    socklen_t len = sizeof(struct sockaddr_in);
    char buf[BUF_SIZE];
    struct hostent *host;
    int n, s;

    host = gethostbyname(servernameorip);
    if(host == NULL) {
		perror("gethostbyname");
	    // showing error for now
		printf("host is NULL\n");
		return_type return_error;
		char* error_msg = "host is null";
		return_error.return_val = error_msg;
		return_error.return_size = sizeof(error_msg);
		return return_error;
    }

    // initialize socket
    if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		//should throw an error
		printf("socket error\n");
		return_type return_error;
		char* error_msg = "socket error";
		return_error.return_val = error_msg;
		return_error.return_size = sizeof(error_msg);
		close(s);
    }

    //initialize server addr
    memset((char *) &server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(serverportnumber);
    server.sin_addr = *((struct in_addr*) host->h_addr);

    // construct procedure call buffer
    char procedure_call[BUF_SIZE];
    void * index = procedure_call;

    // copy in name of the procedure
    strcpy(index, procedure_name);
    index += strlen(procedure_name)+1;

    // copy in number of params
    memcpy((void *)(index), (void *)&nparams, sizeof(int));
    index += sizeof(int);

    // populating list of arguments in the procedure call
    va_list arguments;
    va_start(arguments, nparams);
    int i;
    for (i = 0; i < nparams; ++i) {
    	int arg_size = va_arg(arguments, int); 
    	memcpy((void *)(index), (void *)&arg_size, sizeof(int));
	    index += sizeof(int);

	    void * arg = va_arg(arguments, void *); 
	    memcpy((void *)(index), (void *)arg, arg_size);
	    index += arg_size;
    }
    
    // send message
    if(sendto(s, procedure_call, sizeof(procedure_call), 0, (struct sockaddr *) &server, len) == -1) {
		perror("sendto()");
    }

    // receive response.
	if((n = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &server, &len)) != -1) {
    	//received something
    	printf( "Received from %s:%d\n",	
    			inet_ntoa(server.sin_addr), 
				ntohs(server.sin_port)); 
    	fflush(stdout);

    	if (len>BUF_SIZE) {
    		// showing error for now if BUF_SIZE is too small
    		printf("response is bigger than BUF_SIZE\n");
    		return_type return_error;
    		char* error_msg = "buf too small";

    		return_error.return_val = error_msg;
    		return_error.return_size = sizeof(error_msg);
    		close(s);
    		return return_error;
    	} else {
            // parsing response and creating return_type response object
            return_type *response = malloc(sizeof(*response));;
            memcpy(&(response->return_size), buf, sizeof(int));

            response->return_val = malloc(response->return_size);
            memcpy(response->return_val, (buf + sizeof(int)), response->return_size);

            close(s);
			return *response;
    	}
    } else {
	    // nothing received - showing error for now
		printf("nothing received\n");
		return_type return_error;
		char* error_msg = "nothing received";
		return_error.return_val = error_msg;
		return_error.return_size = sizeof(error_msg);
		close(s);
		return return_error;
    }    
}
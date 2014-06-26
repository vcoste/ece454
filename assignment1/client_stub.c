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

#define BUF_SIZE 65507

/**
 * This is used by the client application program to invoke a remote method. 
 * @param  char *   name or IP address of the server to connect to
 * @param  int      port number to connect to on the server
 * @param  char *   name of the procedure to call
 * @param  int      number of parameters sent to the remote method
 * @param  ...      For each of the nparams parameters, we have two arguments:
 *                  <size of the argument,(void *) to the argument>
 * @return          the return value of the remote procedure call with the 
 *                  correct type
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
        // Hostname not found
		perror("gethostbyname");
        return_type *return_error = malloc(sizeof(*return_error));
        int zero_size = 0;
        memcpy(&(return_error->return_size), &zero_size, sizeof(zero_size));
        return_error->return_val = malloc(return_error->return_size);
        memcpy( return_error->return_val, 
                NULL, 
                return_error->return_size);
		return *return_error;
    }

    // initialize socket
    if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		// Error with socket
        perror("socket");
        return_type *return_error = malloc(sizeof(*return_error));
        int zero_size = 0;
        memcpy(&(return_error->return_size), &zero_size, sizeof(zero_size));
        return_error->return_val = malloc(return_error->return_size);
        memcpy( return_error->return_val, 
                NULL, 
                return_error->return_size);
		close(s);
        return *return_error;
    }

    //initialize server addr
    memset((char *) &server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(serverportnumber);
    server.sin_addr = *((struct in_addr*) host->h_addr);

    // construct procedure_call buffer
    char procedure_call[BUF_SIZE];
    void * index = procedure_call;

    // copy name of the procedure into procedure_call
    strcpy(index, procedure_name);
    index += strlen(procedure_name)+1;

    // copy number of params into procedure_call
    memcpy((void *)(index), (void *)&nparams, sizeof(int));
    index += sizeof(int);

    // populating list of arguments into procedure_call
    va_list arguments;
    va_start(arguments, nparams);
    int i;
    for(i = 0; i < nparams; ++i) {
    	int arg_size = va_arg(arguments, int); 
    	memcpy((void *)(index), (void *)&arg_size, sizeof(int));
	    index += sizeof(int);

	    void * arg = va_arg(arguments, void *); 
	    memcpy((void *)(index), (void *)arg, arg_size);
	    index += arg_size;
    }
    
    // send message
    if(sendto(  s,
                procedure_call,
                sizeof(procedure_call),
                0,
                (struct sockaddr *) &server,
                len) == -1) {
		perror("sendto()");
        return_type *return_error = malloc(sizeof(*return_error));
        int zero_size = 0;
        memcpy(&(return_error->return_size), &zero_size, sizeof(zero_size));
        return_error->return_val = malloc(return_error->return_size);
        memcpy( return_error->return_val, 
                NULL, 
                return_error->return_size);
        close(s);
        return *return_error;
    }

    // receive response.
	if((n = recvfrom(  s,
                       buf,
                       BUF_SIZE,
                       0,
                       (struct sockaddr *) &server,
                       &len)) != -1) {
    	// received something
    	fflush(stdout);

    	if(len>BUF_SIZE) {
    		// Error, BUF_SIZE is too small
    		return_type *return_error = malloc(sizeof(*return_error));
            int zero_size = 0;
            memcpy(&(return_error->return_size), &zero_size, sizeof(zero_size));
            return_error->return_val = malloc(return_error->return_size);
            memcpy( return_error->return_val, 
                    NULL, 
                    return_error->return_size);
    		close(s);
    		return *return_error;
    	} else {
            // parsing response and creating return_type response object
            return_type *response = malloc(sizeof(*response));;
            memcpy(&(response->return_size), buf, sizeof(int));
            response->return_val = malloc(response->return_size);
            memcpy( response->return_val, 
                    (buf + sizeof(int)), 
                    response->return_size);
            close(s);
			return *response;
    	}
    } else {
	    // nothing received from server

        return_type *return_error = malloc(sizeof(*return_error));
        int zero_size = 0;
        memcpy(&(return_error->return_size), &zero_size, sizeof(zero_size));
        return_error->return_val = malloc(return_error->return_size);
        memcpy( return_error->return_val, 
                NULL, 
                return_error->return_size);
		close(s);
		return *return_error;
    }    
}
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
    int n, s/*socket*/;

    host = gethostbyname(servernameorip);
    if (host == NULL) {
	perror("gethostbyname");
	//should throw an error
	// return 1;
    }

    /* initialize socket */
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
	perror("socket");
	//should throw an error
	// return 1;
    }

    /* initialize server addr */
    memset((char *) &server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(serverportnumber);
    server.sin_addr = *((struct in_addr*) host->h_addr);

    //construct procedure call
    char *procedure_call;
	char *comma = ",";
    strcpy(procedure_call, procedure_name);
    strcat(procedure_call, comma);
    char *num_params;
    sprintf(num_params, "%d", nparams);
    strcat(procedure_call,num_params);
    strcat(procedure_call, comma);
    va_list arguments;
    va_start(arguments, nparams);
    for (int i = 0; i < nparams; ++i) {
    	// if (i == 0)
    	strcat(procedure_call, va_arg(arguments, void *));
    	if (i < nparams-1) {
    		strcat(procedure_call, comma);
    	}
    }

    /* send message */
    if (sendto(s, procedure_call, sizeof(procedure_call), 0, (struct sockaddr *) &server, len) == -1) {
	perror("sendto()");
    }

    /* receive echo.
    ** for single message, "while" is not necessary. But it allows the client 
    ** to stay in listen mode and thus function as a "server" - allowing it to 
    ** receive message sent from any endpoint.
    */
    bool more_to_come = 1;
    int num_parts = 0;
    int num_iterations = 0;
   
    char* response_string;
	
    while (more_to_come) {
    	if ((n = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &server, &len)) != -1) {
	    	//received something
	    	//return from here
	    	printf(	"Received from %s:%d: ",	
	    			inet_ntoa(server.sin_addr), 
					ntohs(server.sin_port)); 
	    	fflush(stdout);
			write(1, buf, n);
			write(1, "\n", 1);

	    	if (len>BUF_SIZE) {
	    		// make recvfrom call again with bigger BU_SIZE ?? (not sure)
	    		// showing error for now
	    		printf("response is bigger than BUF_SIZE");
	    		return_type return_error;
	    		char* error_msg = "buf too small";
	    		return_error.return_val = error_msg;
	    		return_error.return_size = sizeof(error_msg);
	    		close(s);
	    		return return_error;
	    	} else {
	    		char * comparator = "$";
	    		int result = strncmp(&buf[1], comparator, 1);
	    		if(result == 0) {
	    			num_parts = atoi(&buf[0]);
				}
				// take result and add to 
				strcat(response_string, buf);
				num_iterations++;
				if (num_iterations >= num_parts) {
					more_to_come = 0;
				}
	    	}
	    } else {
	    	//might have to close the socket before this
		    //probabaly won't reach this point
		    close(s);
		    // showing error for now
			printf("nothing received");
			return_type return_error;
			char* error_msg = "nothing received";
			return_error.return_val = error_msg;
			return_error.return_size = sizeof(error_msg);
			close(s);
			return return_error;
	    }
    }     

    //make return_type from reponse buf
    return_type response;
	response.return_val = response_string;
	response.return_size = sizeof(response_string);
	close(s);
	return response;
}
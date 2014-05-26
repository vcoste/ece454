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
    char *open_paranthesis = '(';
    char *close_paranthesis = ')';
    strcpy (procedure_call, procedure_name);
    strcat(procedure_call, open_paranthesis);
    va_list arguments;
    va_start(arguments, nparams);
    for (int i = 0; i < nparams; ++i) {
    	// strcat(procedure_call, va_arg(arguments, (void *)));
    }
    strcat(procedure_call, close_paranthesis);

    /* send message */
    if (sendto(s, procedure_call, sizeof(procedure_call), 0, (struct sockaddr *) &server, len) == -1) {
	perror("sendto()");
	// return 1;
    }

    /* receive echo.
    ** for single message, "while" is not necessary. But it allows the client 
    ** to stay in listen mode and thus function as a "server" - allowing it to 
    ** receive message sent from any endpoint.
    */
    if ((n = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &server, &len)) != -1) {
    	//received something
    	//return something from here
    	printf(	"Received from %s:%d: ",	
    			inet_ntoa(server.sin_addr), 
				ntohs(server.sin_port)); 
    	fflush(stdout);
		write(1, buf, n);
		write(1, "\n", 1);

    	if (len>BUF_SIZE) {
    		// make recvfrom call again with bigger BU_SIZE ?? (not sure)
    		printf("response is bigger than BUF_SIZE");
    	} else {
    		//make return_type from reponse buf
    		return_type response;
    		response.return_val = buf;
    		response.return_size = sizeof(buf);
    		return response;
    	}
    }
    //might have to close the socket before this
    //probabaly won't reach this point
    close(s);
    // return 0;
}
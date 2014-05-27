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

    /* initialize socket */
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
    /* initialize server addr */
    memset((char *) &server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(serverportnumber);
    server.sin_addr = *((struct in_addr*) host->h_addr);

    ////////////////////////////////////////////////////////////////////
    //construct procedure call
    char procedure_call[BUF_SIZE];
    void * index = procedure_call;
    printf("%s\n", procedure_name);
    printf("%d\n", strlen(procedure_name));
    strcpy(procedure_call, procedure_name);
    index += strlen(procedure_name);

    memcpy((void *)(index), (void *)&nparams, sizeof(int)); // copy in number of params
    index += sizeof(int);

    va_list arguments;
    va_start(arguments, nparams);
    printf("before for loop\n");
    for (int i = 0; i < nparams; ++i) {
    	int arg_size = va_arg(arguments, int); 
    	printf("i=%d, arg_size=%d\n", i, arg_size);
    	memcpy((void *)(index), (void *)&arg_size, sizeof(int));
	    index += sizeof(int);

	    void * arg = va_arg(arguments, void *); 
    	printf("i=%d, arg=%s\n", i, (char *)arg);
	    memcpy((void *)(index), (void *)arg, arg_size);
	    index += arg_size;
    }
    printf("after for loop\n");
    
    // printf("%d\n", isalpha(buf[0]));
    printf("procedure name: %s\n", (char *)(procedure_call));
    printf("number of params: %d\n", *(int *)(procedure_call+strlen(procedure_name)));
    printf("size of first param: %d\n", *(int *)(procedure_call+strlen(procedure_name)+sizeof(int)));
    printf("val  of first param: %d\n", *(int *)(procedure_call+strlen(procedure_name)+sizeof(int)+sizeof(int)));
    printf("size of secon param: %d\n", *(int *)(procedure_call+strlen(procedure_name)+sizeof(int)+sizeof(int)+sizeof(int)));
    printf("val  of secon param: %d\n", *(int *)(procedure_call+strlen(procedure_name)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)));
    /* send message */
    if(sendto(s, procedure_call, sizeof(procedure_call), 0, (struct sockaddr *) &server, len) == -1) {
		perror("sendto()");
    }
    printf("after sendto\n");

    /* receive echo.
    ** for single message, "while" is not necessary. But it allows the client 
    ** to stay in listen mode and thus function as a "server" - allowing it to 
    ** receive message sent from any endpoint.
    */
	
	if((n = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &server, &len)) != -1) {
		printf("in first if\n");
    	//received something
    	//return from here
    	printf(	"Received from %s:%d: ",	
    			inet_ntoa(server.sin_addr), 
				ntohs(server.sin_port)); 
    	fflush(stdout);
		write(1, buf, n);
		write(1, "\n", 1);

    	if (len>BUF_SIZE) {
    		printf("in second if\n");
    		// make recvfrom call again with bigger BUF_SIZE ?? (not sure)
    		// showing error for now
    		printf("response is bigger than BUF_SIZE\n");
    		return_type return_error;
    		char* error_msg = "buf too small";
    		return_error.return_val = error_msg;
    		return_error.return_size = sizeof(error_msg);
    		close(s);
    		return return_error;
    	} else {
    		printf("in second else\n");
    		return_type response;
    		memcpy(&(response.return_size), buf, sizeof(int));
    		memcpy(&(response.return_val), (buf+sizeof(int)), response.return_size);
			close(s);
			return response;
    	}
    } else {
    	printf("in first else\n");
    	//might have to close the socket before this
	    //probabaly won't reach this point
	    // showing error for now
		printf("nothing received\n");
		return_type return_error;
		char* error_msg = "nothing received";
		return_error.return_val = error_msg;
		return_error.return_size = sizeof(error_msg);
		close(s);
		return return_error;
    } 
    printf("after recvfrom\n");
    //make return_type from reponse buf
    
}

int main() {
	
	int a = -10, b = 20, c=30, d=60;

	char *e = "e";
	char *f = "f";
	char *g = "g";
	char *h = "h";
	char *j = "j";

	return_type ans = make_remote_call( "ecelinux3.uwaterloo.ca",
										5673,
										"addtwo", 2,
										sizeof(int), (void *)(&a),
										sizeof(int), (void *)(&b));

	// return_type ans = make_remote_call("ecelinux3.uwaterloo.ca",
	// 								5673,
	// 								"mult", 4,
	// 								sizeof(int), (void *)(&a),
	// 								sizeof(int), (void *)(&b),
	// 								sizeof(int), (void *)(&c),
	// 								sizeof(int), (void *)(&d));

	// return_type ans = make_remote_call("ecelinux3.uwaterloo.ca",
	// 								5673,
	// 								"concatinate", 5,
	// 								sizeof(e), (void *)(e),
	// 								sizeof(f), (void *)(f),
	// 								sizeof(g), (void *)(g),
	// 								sizeof(h), (void *)(h),
	// 								sizeof(j), (void *)(j));
	int i = *(int *)(ans.return_val);
	printf("client, got result: %d\n", i);

	return 0;
}
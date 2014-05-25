#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ece454rpc_types.h"

struct function_node *functions;

/* The following need to be implemented in the server stub */

/* register_procedure() -- invoked by the app programmer's server code
 * to register a procedure with this server_stub. Note that more than
 * one procedure can be registered */
bool register_procedure(const char *procedure_name,
	                    const int nparams,
			       		fp_type fnpointer) {
	//check if it already exists
	//if it doesn't add it to a linked list or something like that
	// if it worked
	bool found = false;
	struct function_node *itr = malloc(sizeof(*itr));
	struct function_node *newFunc;

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

 // launch_server() -- used by the app programmer's server code to indicate that
 // * it wants start receiving rpc invocations for functions that it registered
 // * with the server stub.
 // *
 // * IMPORTANT: the first thing that should happen when launch_server() is invoked
 // * is that it should print out, to stdout, the IP address/domain name and
 // * port number on which it listens.
 // *
 // * launch_server() simply runs forever. That is, it waits for a client request.
 // * When it receives a request, it services it by invoking the appropriate 
 // * application procedure. It returns the result to the client, and goes back
 // * to listening for more requests.
 
void launch_server() {
	//ouput server and port
	printf("ecelinux3.uwaterloo.ca 5764"); //this is just an example

	//start server and wait for procedure calls
	//when client request arrives, invoke it locally and return response to client

	//should not return
}

int main(int argc, char const *argv[])
{
	const char *nm = "asdf";
	int nParams = 1;
	fp_type ptr = (fp_type)0x1233123;
	register_procedure(nm, nParams,ptr);

	const char *nm1 = "asdfP";
	int nParams1 = 2;
	fp_type ptr1 = (fp_type)0x1233223;
	register_procedure(nm1, nParams1,ptr1);

	struct function_node *itr = malloc(sizeof(*itr));
	for (itr = functions; itr; itr = itr->next) {
		printf("%s\n", itr->procedure_name);
	}

	return 0;
}
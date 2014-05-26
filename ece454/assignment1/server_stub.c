#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ece454rpc_types.h"

func_node *functions;
int ret_int;
return_type r;

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
	func_node *itr = malloc(sizeof(*itr));
	func_node *newFunc;

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

return_type add(const int nparams, arg_type* a) {
	 if(nparams != 2) {
		/* Error! */
		r.return_val = NULL;
		r.return_size = 0;
		return r;
    }

    if(a->arg_size != sizeof(int) ||
       a->next->arg_size != sizeof(int)) {
		/* Error! */
		r.return_val = NULL;
		r.return_size = 0;
		return r;
    }

    int i = *(int *)(a->arg_val);
    int j = *(int *)(a->next->arg_val);

    ret_int = i+j;
    r.return_val = (void *)(&ret_int);
    r.return_size = sizeof(int);

    return r;
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

int main(int argc, char const *argv[]) {

	register_procedure("addtwo", 2, add);

	
	arg_type *args = malloc(sizeof(*args));
	int arg1 = 1, arg2 = 2;

	args->arg_val = &arg1;
	args->arg_size = sizeof(int);

	args->next = malloc(sizeof(*args));
	args->next->arg_val = &arg2;
	args->next->arg_size = sizeof(int);

	func_node *itr = malloc(sizeof(*itr));
	for (itr = functions; itr; itr = itr->next) {
		return_type retVal = itr->fnpointer(2, args);
		printf("Shit returned! %d\n", *((int *)retVal.return_val));
	}

	return 0;
}
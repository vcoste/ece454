#include <stdio.h>
#include "ece454rpc_types.h"

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
	
}
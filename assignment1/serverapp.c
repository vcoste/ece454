#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ece454rpc_types.h"

int ret_int;
return_type r;

return_type add(const int nparams, arg_type* a)
{
	printf("In RPC add\n");
	if(nparams != 2) {
		/* Error! */
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	if(a->arg_size != sizeof(int) || a->next->arg_size != sizeof(int)) {
		/* Error! */
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	int i = *(int *)(a->arg_val);
	printf("p1 %d\n", i);
	int j = *(int *)(a->next->arg_val);
	printf("p2 %d\n", j);

	ret_int = i+j;
	r.return_val = (void *)(&ret_int);
	r.return_size = sizeof(int);

	return r;
}

return_type addThree(const int nparams, arg_type* a)
{
	printf("In RPC add\n");
	if(nparams != 3) {
		/* Error! */
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	if(a->arg_size != sizeof(int) || a->next->arg_size != sizeof(int) || a->next->next->arg_size != sizeof(int)) {
		/* Error! */
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	int i = *(int *)(a->arg_val);
	printf("p1 %d\n", i);
	int j = *(int *)(a->next->arg_val);
	printf("p2 %d\n", j);
	int k = *(int *)(a->next->next->arg_val);
	printf("p3 %d\n", k);

	ret_int = i+j+k;
	r.return_val = (void *)(&ret_int);
	r.return_size = sizeof(int);

	return r;
}

return_type concat(const int nparams, arg_type* a) {
	if (nparams != 2) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	char *arg1 = (char *)a->arg_val, *arg2 = (char *)a->next->arg_val;
	char *result = malloc(a->arg_size + a->next->arg_size);

	printf("Successfully received params. arg1: %s, arg2: %s\n", arg1, arg2);

	strcpy(result, arg1);
	strcat(result, arg2);

	printf("Concatenated strings: %s\n", result);

	r.return_val = result;
	r.return_size = a->arg_size + a->next->arg_size;

	return r;
}

int main() {
	printf("Registering addtwo\n");
	register_procedure("addtwo", 2, add);
	printf("Registering addThree\n");
	register_procedure("addthree", 3, addThree);
	printf("Registering concattwo\n");
	register_procedure("concattwo", 2, concat);

	launch_server();

	// arg_type *args = malloc(sizeof(*args));
	// args->arg_size = 2;
	// args->arg_val = malloc(2);
	// strcpy(args->arg_val, arg1Str);

	// printf("Value in arg1: %s\n", args->arg_val);

	// args->next = malloc(sizeof(arg_type));
	// args->next->arg_size = 2;
	// args->next->arg_val = malloc(2);
	// strcpy(args->next->arg_val, arg2Str);
	// printf("Value in arg2: %s\n", args->next->arg_val);

	// printf("Calling concat\n");

	// concat(2, args);

	/* should never get here, because
	   launch_server(); runs forever. */

	return 0;
}

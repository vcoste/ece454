#include <stdio.h>
#include "ece454rpc_types.h"

int main()
{
	char *arg1Str = "ab";
	char *arg2Str = "cd";

	return_type ans = make_remote_call( "localhost",
										10000,
										"concattwo", 2,
										sizeof(arg1Str), (void *)(arg1Str),
										sizeof(arg2Str), (void *)(arg2Str));
	char *result = (char *)ans.return_val;
	printf("client, got result: %s\n", result);

	return 0;
}
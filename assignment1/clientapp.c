#include <stdio.h>
#include <stdlib.h>
#include "ece454rpc_types.h"

int main(int argc, char **argv)
{
	char *arg1Str = "ab";
	char *arg2Str = "cd";
	char *arg3Str = "ef";

	int arg1Int = 4;
	int arg2Int = 8;
	int arg3Int = 5;

	if (argc == 3){

		printf("Sending arg1: %s, arg2: %s\n", arg1Str, arg2Str);
		return_type ans = make_remote_call( argv[1],
											atoi(argv[2]),
											"concattwo", 2,
											sizeof(arg1Str), (void *)(arg1Str),
											sizeof(arg2Str), (void *)(arg2Str));
		char *result = (char *)ans.return_val;
		printf("Concat, client, got result: %s\n", result);

		return_type ans2 = make_remote_call(	argv[1],
												atoi(argv[2]),
												"addtwo", 2,
												sizeof(arg1Int), (void *)(&arg1Int),
												sizeof(arg2Int), (void *)(&arg2Int));
		int result2 = *(int*)ans2.return_val;
		printf("Add, client got result: %d\n", result2);

		return_type ans3 = make_remote_call(	argv[1],
												atoi(argv[2]),
												"addtwo", 3,
												sizeof(arg1Int), (void *)(&arg1Int),
												sizeof(arg2Int), (void *)(&arg2Int),
												sizeof(arg3Int), (void *)(&arg3Int));
		int result3 = *(int*)ans3.return_val;
		printf("Add, client got result: %d\n", result3);
	} else {
		printf("Not enough args\nUseage: <ip address> <port>\n");
	}
	return 0;
}
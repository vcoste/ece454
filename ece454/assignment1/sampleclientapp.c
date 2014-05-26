#include <stdio.h>
#include "ece454rpc_types.h"

int main()
{
	int a = -10, b = 20;
	return_type ans = make_remote_call("exelinux3.uwaterloo.ca",
									5673,
									"addtwo", 2,
									sizeof(int), (void *)(&a),
									sizeof(int), (void *)(&b));
	int i = *(int *)(ans.return_val);
	printf("client, got result: %d\n", i);

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsOtherIncludes.h"

// linked list of users that have been mounted
struct mounted_user {
	int *id;
	struct mounted_user *next;
};

int id_counter = 0; // used to give a unique ID to each user
struct mounted_user *users = NULL;

////////////////////////////////////////////////////////////////////////////////
///                          Helper Functions Definitions
////////////////////////////////////////////////////////////////////////////////
/**
 * Allocates a unique id for the current user
 * @return [int] id for current user
 */
int giveID();

////////////////////////////////////////////////////////////////////////////////
/// Start of implementation
////////////////////////////////////////////////////////////////////////////////
return_type r;

return_type fsMount(const int nparams, arg_type* a) {
	
	struct mounted_user *newmounted_user = (struct mounted_user *)malloc(sizeof(struct mounted_user));
	newmounted_user->id = (int *)malloc(sizeof(int));
	*newmounted_user->id = giveID();
	newmounted_user->next = users;
	users = newmounted_user;

	r.return_val = newmounted_user->id;
	r.return_size = sizeof(int);

	return r;
}

int giveID() {
	int newID = id_counter;
	id_counter++;
	return newID;
}

int main(int argc, char const *argv[]) {
	register_procedure("fsMount", 0, fsMount);

    launch_server();
    return 0;
}
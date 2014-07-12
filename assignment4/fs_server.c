#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsOtherIncludes.h"

// linked list of users that have been mounted
struct mounted_user {
	int *id;
	struct mounted_user *next;
	// DIR *dirStream - might be good to have a stream per client, might not be good...
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
/**
 * Removes client reference from the users linked list
 * @param  int id - ID of the client to be deleted
 * @return     1 if success 0 if failure
 */
int removeClientByID(int);

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

return_type fsUnmount(const int nparams, arg_type* a) {

	if (nparams != 1) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	int clientID = *(int*)a->arg_val;
	removeClientByID(clientID);

	int zero = 0;
	r.return_val = (void *)&zero;
	r.return_size = sizeof(int);
	return r;
}

return_type fsOpenDir(const int nparams, arg_type* a) {

	if (nparams != 1) {
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	DIR *desiredDir;
	desiredDir = opendir(a->arg_val);

	r.return_val = desiredDir;
	r.return_size = sizeof(desiredDir);

	return r;
}

int giveID() {
	int newID = id_counter;
	id_counter++;
	return newID;
}

int removeClientByID(int id) {
	
	struct mounted_user *itr = users;
	for (; itr != NULL; itr = itr->next) {
		if (itr->next == NULL) {
			free(itr);
			itr = NULL;
		} else if(*itr->next->id == id) {
			if (itr->next->next == NULL) {
				free(itr->next);
				itr->next = NULL;
			} else {
				struct mounted_user *temp = itr->next;
				itr->next = itr->next->next;
				free(temp);
				temp = NULL;
			}
		}
	}
}

int main(int argc, char const *argv[]) {

	DIR *workingDir;
	struct dirent *directoryEntry;

	if (argc != 2) {
		perror("Server requires local folder to be served as argument only"); exit(1);
	} else {
		if ((workingDir = opendir(argv[1])) == NULL) {
			perror("Cannot open directory"); exit(1);
		}
		printf("Opened directory\nPrinting directory\n");
	}

	while ((directoryEntry = readdir(workingDir)) != NULL) {
		printf("\t%s\n", directoryEntry->d_name);
	}
	printf("\n");

	register_procedure("fsMount", 0, fsMount);
	printRegisteredProcedures();

    launch_server();
    return 0;
}
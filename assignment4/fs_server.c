#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fsOtherIncludes.h"

#if 1
#define _DEBUG_1_
#endif

// linked list of users that have been mounted
typedef struct MountedUser {
	int *id;
	struct MountedUser *next;
	DIR *dirStream;
} mounted_user;

int id_counter = 0; // used to give a unique ID to each user
mounted_user *users = NULL;

////////////////////////////////////////////////////////////////////////////////
///                          Helper Functions Definitions
////////////////////////////////////////////////////////////////////////////////
/**
 * Allocates a unique id for the current user
 * @return [int] id for current user
 */
int giveID();
/**
 * Adds a client to the users linked list of mounted users
 * @return     0 on success, -1 on failure
 */
int* addNewClient();
/**
 * Removes client reference from the users linked list
 * @param  int id - ID of the client to be deleted
 * @return     1 if success 0 if failure
 */
int removeClientByID(int);
/**
 * Finds the client in the users linked list that matches the id provided
 * @param  int [id] id of the desired client
 * @return     [mounted_user] NULL if not found
 */
mounted_user* findClientById(int);
/**
 * Prints the ids in the mounted_users linked list
 */
void printMountedUsers();

////////////////////////////////////////////////////////////////////////////////
/// Start of implementation
////////////////////////////////////////////////////////////////////////////////
return_type r;

return_type fsMount(const int nparams, arg_type* a) {
	#ifdef _DEBUG_1_
	printf("in fsMount\n");
	#endif
	int *clientID = addNewClient();

	#ifdef _DEBUG_1_
	printMountedUsers();
	#endif

	if (*clientID >= 0) {
		r.return_val  = clientID;
		r.return_size = sizeof(int);
	} else {
		r.return_val  = NULL;
		r.return_size = 0;
	}

	#ifdef _DEBUG_1_
	printf("leaving fsMount\n");
	#endif
	return r;
}

return_type fsUnmount(const int nparams, arg_type* a) {
	#ifdef _DEBUG_1_
	printf("in fsUnmount (server side)\n");
	#endif

	if (nparams != 1) {
		r.return_val  = NULL;
		r.return_size = 0;
		return r;
	}

	int clientID = *(int*)a->arg_val;
	removeClientByID(clientID);

	int zero = 0;
	r.return_val  = (void *)&zero;
	r.return_size = sizeof(int);
	return r;
}

return_type fsOpenDir(const int nparams, arg_type* a) {
	int retVal = 0;

	if (nparams != 2) {
		retVal = -1;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {

		retVal = -1;
	} else if ((user->dirStream = opendir((char *)a->next->arg_val)) == NULL) {

		perror("fsOpenDir()"); 
		retVal = -1;
	}

	r.return_size = sizeof(int);
	return r;
}

return_type fsCloseDir(const int nparams, arg_type* a) {

	int retVal = 0;

	if (nparams != 2) {
		retVal = -1;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {

		retVal = -1;
	} else if (user->dirStream != NULL || closedir(user->dirStream) == -1) {

		perror("fsCloseDir()"); 
		retVal = -1;
	}

	r.return_size = sizeof(int);
	return r;
}

int giveID() {
	int newID = id_counter;
	id_counter++;
	return newID;
}

int* addNewClient() {
	
	mounted_user *newmounted_user = malloc(sizeof(mounted_user));
	newmounted_user->id  = malloc(sizeof(int));
	*newmounted_user->id = giveID();
	newmounted_user->next = NULL;

	#ifdef _DEBUG_1_
	printf("New user created with id: %d\nAdding to linked list\n", *newmounted_user->id);
	#endif

	if (users == NULL) {
		users = newmounted_user;
		#ifdef _DEBUG_1_
		printf("List was empty. New user added to head of list\n");
		#endif
		return newmounted_user->id;
	}

	mounted_user *itr = users;

	for(; itr != NULL; itr = itr->next) {
		if (itr->next == NULL) { // at tail so add new user
			itr->next = newmounted_user;
			break;
		}
	}

	#ifdef _DEBUG_1_
	printf("Added new client to end of list\n");
	#endif

	return newmounted_user->id;
}

int removeClientByID(int id) {
	
	mounted_user *itr = users;
	for (; itr != NULL; itr = itr->next) {
		if (*itr->id == id && itr->next == NULL) {
			free(itr);
			itr = NULL;
		} else if(*itr->next->id == id) {
			if (itr->next->next == NULL) {
				free(itr->next);
				itr->next = NULL;
			} else {
				mounted_user *temp = itr->next;
				itr->next = itr->next->next;
				free(temp);
				temp = NULL;
			}
		}
	}

	return 0;
}

mounted_user* findClientById(int id) {

	mounted_user *user = users;
	for (; user != NULL; user = user->next) {
		if (*user->id == id) {
			return user;
		}
	}

	return NULL;
}

void printMountedUsers() {
	printf("Printing mounted user id's\n");
	mounted_user *user = users;
	for (; user != NULL; user = user->next) {
		printf("\t%d\n", *user->id);
	}
	printf("\tEND\n\n");
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
	register_procedure("fsUnmount", 1, fsUnmount);
	register_procedure("fsOpenDir", 1, fsOpenDir);
	printRegisteredProcedures();

    launch_server();
    return 0;
}
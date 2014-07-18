#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "fsOtherIncludes.h"

#if 1
#define _DEBUG_1_
#endif

// linked list of users that have been mounted
typedef struct MountedUser {
	int *id;
	char *folderAilias;
	DIR *dirStream;
	struct MountedUser *next;
} mounted_user;


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
 * @param      folderAilias   - local folder of client to be ailiased to server (mounted) directory
 * @param      folderNameSize - size in bytes of the folder name
 * @return     0 on success, -1 on failure
 */
int addNewClient(char*, int);
/**
 * Removes client reference from the users linked list
 * @param      id           - ID of the client to be deleted
 * @param      folderAilias - folder ailias paried with given id
 * @return     0 on success -1 on failure
 */
int removeClient(char*, int);
/**
 * Finds the client in the users linked list that matches the id provided
 * @param  int [id] id of the desired client
 * @return     [mounted_user] NULL if not found
 */
mounted_user* findClientById(int);
char* transformPath(char*, char*);
/**
 * Prints the ids in the mounted_users linked list
 */
void printMountedUsers();

////////////////////////////////////////////////////////////////////////////////
/// Start of implementation
////////////////////////////////////////////////////////////////////////////////
return_type r;
int id_counter = 0; // used to give a unique ID to each user
char* workingDirectoryName;
mounted_user *users = NULL;

return_type fsMount(const int nparams, arg_type* a) {
	#ifdef _DEBUG_1_
	printf("in fsMount\n");
	#endif

	if (nparams != 1) {
		r.return_val  = NULL;
		r.return_size = 0;
		return r;
	}

	int *clientID = malloc(sizeof(int));
	*clientID = addNewClient(a->arg_val, a->arg_size);

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

	if (nparams != 2) {
		r.return_val  = NULL;
		r.return_size = 0;
		return r;
	}

	if (removeClient((char*)a->arg_val, *(int*)a->next->arg_val) == -1) {
		printf("\tremoveClient failed\n");
	}

	#ifdef _DEBUG_1_
	printMountedUsers();
	#endif

	int *ret_int = (int *)malloc(sizeof(int));
	*ret_int = 0;
	r.return_val  = ret_int;
	r.return_size = sizeof(int);
	return r;
}

return_type fsOpenDir(const int nparams, arg_type* a) {
	int *retVal = malloc(sizeof(int));
	r.return_size = sizeof(int);

	#ifdef _DEBUG_1_
	printf("In fsOpenDir, %d arugments:\n", nparams);
	if (nparams > 0) {
		printf("\tSize of arg1: %d\n", a->arg_size);
		printf("\tValue arg1: %d\n", *(int*)(a->arg_val));
	}
	if (nparams > 1) {
		printf("\tSize of arg2: %d\n", a->next->arg_size);
		printf("\tValue arg2: %s\n", a->next->arg_val);
	}
	#endif

	if (nparams != 2 || a->arg_size != sizeof(int)) {
		printf("\tError in fsOpenDir, incorrect arguments reveived\n");
		*retVal = EINVAL;
		r.return_val = retVal;
		return r;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {
		printf("\tError in fsOpenDir, client not found\nID: %d\n", *(int*)a->arg_val);
		*retVal = EACCES;
		r.return_val = retVal;
		return r;
	}
	printf("\ttransform: %s\n", transformPath(user->folderAilias, (char *)a->next->arg_val));
	if ((user->dirStream = opendir(transformPath(user->folderAilias, (char *)a->next->arg_val))) == NULL) {
		perror("\tfsOpenDir()");
		*retVal = errno;
		r.return_val = retVal;
		return r;
	}

	#ifdef _DEBUG_1_
	printf("\tsuccessfully opened directory %s\n", a->next->arg_val);
	#endif

	*retVal = 0;
	r.return_val = retVal;
	return r;
}

return_type fsCloseDir(const int nparams, arg_type* a) {

	int *retVal = malloc(sizeof(int));
	r.return_size = sizeof(int);

	if (nparams != 1 || a->arg_size != sizeof(int)) {
		printf("\tError in fsCLoseDir, incorrect arguments reveived\n");
		*retVal = EINVAL;
		r.return_val = retVal;
		return r;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {

		*retVal = EACCES;
		r.return_val = retVal;
		return r;
	}

	if (user->dirStream != NULL || closedir(user->dirStream) == -1) {

		perror("\tfsCloseDir()"); 
		*retVal = errno;
		r.return_val = retVal;
		return r;
	}

	*retVal = 0;
	r.return_val = retVal;
	return r;
}

return_type fsReadDir(const int nparams, arg_type* a) {
	char *retVal;
	int fileType;
	struct stat st;

	if (nparams != 1 || a->arg_size != sizeof(int)) {
		printf("\tError in fsReadDir, incorrect arguments reveived\n");
		retVal = malloc(sizeof(int));
		*retVal = EINVAL;
		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {
		printf("\tError in fsReadDir, clientID not found: %d\n", *(int*)a->arg_val);
		retVal = malloc(sizeof(int));
		*retVal = EACCES;
		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}

	errno = 0;
	struct dirent *currentDirent;
	if ((currentDirent = readdir(user->dirStream)) == NULL) {
		// either at end of entries or error
		if (errno != 0) {
			perror("\tfsReadDir");
			// error
			retVal = malloc(sizeof(int));
			*retVal = errno;
			r.return_val = retVal;
			r.return_size = sizeof(int);
			return r;
		}
		printf("\tAt end of folder\n");
		
		r.return_val = NULL;
		r.return_size = 0;
		return r;
	}

	retVal = malloc(sizeof(int)+strlen(currentDirent->d_name));

	lstat(currentDirent->d_name, &st);
	if (S_ISDIR(st.st_mode)) {
		fileType = 1;
	} else if (S_ISREG(st.st_mode)) {
		fileType = 0;
	} else {
		fileType = -1;
	}

	memcpy(retVal, &fileType, sizeof(int));
	memcpy(retVal+sizeof(int), currentDirent->d_name, strlen(currentDirent->d_name));

	r.return_val = retVal;
	r.return_size = sizeof(int)+strlen(currentDirent->d_name);

	#ifdef _DEBUG_1_
	printf("\tRead fileName: %s, with strlen: %lu\n", currentDirent->d_name, strlen(currentDirent->d_name));
	printf("\tReturning buffer of size: %d\n", r.return_size);
	#endif

	return r;
}

return_type fsOpen(const int nparams, arg_type* a) {

	char *retBuffer = malloc(2*sizeof(int));
	int fd;
	int errorDescriptor = 0;
	int returnValue = 0;
	int openFlags;

	if (nparams != 3 || a->arg_size != sizeof(int)) {
		printf("\tError in fsOpen, incorrect arguments reveived\n");
		errorDescriptor = -1;
		returnValue = EINVAL;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {
		printf("\tError in fsOpen, clientID not found: %d\n", *(int*)a->arg_val);
		errorDescriptor = -1;
		returnValue = EACCES;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}

	if (*(int*)a->next->next->arg_val == 0) {
		#ifdef _DEBUG_1_
		printf("\tIn fsOpen, read call\n");
		#endif
		openFlags = O_RDONLY | O_NONBLOCK;
	} else if (*(int*)a->next->next->arg_val == 1) {
		#ifdef _DEBUG_1_
		printf("\tIn fsOpen, write call\n");
		#endif
		openFlags = O_WRONLY | O_CREAT | O_NONBLOCK;
	} else {
		printf("\tUnrecognized value for open mode\n");
		errorDescriptor = -1;
		returnValue = EINVAL;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}

	#ifdef _DEBUG_1_
	printf("\tcalling system open call, file name: %s, mode: %d, flags generated: %d\n", a->next->arg_val, *(int*)a->next->next->arg_val, openFlags);
	#endif

	if ((fd = open(transformPath(user->folderAilias, a->next->arg_val), openFlags)) == -1) {
		perror("\tfsOpen");
		// error
		errorDescriptor = -1;
		returnValue = errno;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}
	if (fchmod(fd, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
		perror("\tfsOpen");
		// error
		errorDescriptor = -1;
		returnValue = errno;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}

	#ifdef _DEBUG_1_
	printf("\tsuccess from open, fd: %d\n", fd);
	#endif

	
	returnValue = fd;
	memcpy(retBuffer, &errorDescriptor, sizeof(int));
	memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

	r.return_val = retBuffer;
	r.return_size = 2*sizeof(int);
	return r;
}

return_type fsClose(const int nparams, arg_type* a) {

	int *retVal = malloc(sizeof(int));

	if (nparams != 2 || a->arg_size != sizeof(int)) {
		printf("\tError in fsClose, incorrect arguments reveived\n");
		*retVal = EINVAL;

		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {
		printf("\tError in fsClose, clientID not found: %d\n", *(int*)a->arg_val);
		*retVal = EACCES;

		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}

	if (close(*(int*)a->next->arg_val) == -1) {
		*retVal = errno;

		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}

	return r;
}

int giveID() {
	int newID = id_counter;
	id_counter++;
	return newID;
}

int addNewClient(char* folderAilias, int folderNameSize) {
	
	mounted_user *newmounted_user = malloc(sizeof(mounted_user));
	newmounted_user->id           = malloc(sizeof(int));

	#ifdef _DEBUG_1_
	printf("\tAdding new client with folder ailias: %s, string length: %d\n", folderAilias, folderNameSize-1);
	#endif

	if (folderAilias[folderNameSize-1] == '/') {
		#ifdef _DEBUG_1_
		printf("\tSlash present in name, removing\n");
		#endif
		folderNameSize--;
	}
	newmounted_user->folderAilias = malloc(folderNameSize);

	*newmounted_user->id = giveID();
	memcpy(newmounted_user->folderAilias, folderAilias, folderNameSize);
	newmounted_user->folderAilias[folderNameSize] = '\0';

	newmounted_user->next = NULL;

	#ifdef _DEBUG_1_
	printf("\tNew user created with id: %d and folder ailias: %s\n\tAdding to linked list...\n", *newmounted_user->id, newmounted_user->folderAilias);
	#endif

	if (users == NULL) {
		users = newmounted_user;
		#ifdef _DEBUG_1_
		printf("\tList was empty. New user added to head of list\n");
		#endif
		return *newmounted_user->id;
	}

	mounted_user *itr = users;

	for(; itr != NULL; itr = itr->next) {
		if (itr->next == NULL) { // at tail so add new user
			itr->next = newmounted_user;
			break;
		}
	}

	#ifdef _DEBUG_1_
	printf("\tAdded new client to end of list\n");
	#endif

	return *newmounted_user->id;
}

int removeClient(char* folderAilias, int id) {
	
	mounted_user **itr = &users;
	for (; *itr != NULL; *itr = (*itr)->next) {
		// check if at tail and desred user
		if ((*itr)->next == NULL && *(*itr)->id == id && strcmp((*itr)->folderAilias, folderAilias) == 0) {
			free((*itr)->id);
			free((*itr)->folderAilias);
			if ((*itr)->dirStream != NULL) {
				free((*itr)->dirStream);
			}
			free(*itr);
			(*itr) = NULL;
			return 0;
		} else if (*(*itr)->next->id == id && strcmp((*itr)->folderAilias, folderAilias) == 0) {
			if ((*itr)->next->next == NULL) {

				free((*itr)->next);
				free((*itr)->next->id);
				free((*itr)->next->folderAilias);
				if ((*itr)->next->dirStream != NULL) {
					free((*itr)->next->dirStream);
				}
				(*itr)->next = NULL;
				return 0;
			} else {
				mounted_user **temp = &(*itr)->next;
				(*itr)->next = (*itr)->next->next;
				free((*temp)->id);
				free((*temp)->folderAilias);
				if ((*temp)->dirStream != NULL) {
					free((*temp)->dirStream);
				}
				free(*temp);
				temp = NULL;
				return 0;
			}
		}
	}
	return -1;
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

char* transformPath(char* folderAilias, char* pathGiven) {
	int foundFolderAilias = 0;
	int indexOfFileName = 0; // looks for the start of alpha character, the path can potentially start with slashes and/or periods
	char* transformedPath;

	#ifdef _DEBUG_1_
	printf("\tIn transformPath, folderAilias: %s, pathGiven: %s\n", folderAilias, pathGiven);
	#endif

	while (foundFolderAilias == 0) {
		if (pathGiven[indexOfFileName] == '/' || pathGiven[indexOfFileName] == '.') {
			indexOfFileName++;
		} else {
			foundFolderAilias = 1;
		}
	}
	// indexOfFileName now starts at where the filename acutally starts
	// compare with folderAilias to see if it needs to be replaced
	if (strncmp(&pathGiven[indexOfFileName], folderAilias, strlen(folderAilias)) == 0 && pathGiven[indexOfFileName+strlen(folderAilias)] == '/') {

		#ifdef _DEBUG_1_
		printf("\tFolder ailias in path given\n");
		#endif

		transformedPath = (char*)malloc(strlen(workingDirectoryName)+strlen(pathGiven)-strlen(folderAilias)-1);

		memcpy(transformedPath, pathGiven, indexOfFileName);
		memcpy(transformedPath+indexOfFileName, workingDirectoryName, strlen(workingDirectoryName)-1);
		strcpy(transformedPath+indexOfFileName+strlen(workingDirectoryName)-1, &pathGiven[indexOfFileName+strlen(folderAilias)]);
	} else {
		#ifdef _DEBUG_1_
		printf("\tNo folder ailias in path given, just appending workingDirectoryName\n");
		#endif
		transformedPath = (char*)malloc(strlen(workingDirectoryName)+strlen(pathGiven));
		memcpy(transformedPath, workingDirectoryName, strlen(workingDirectoryName));
		memcpy(transformedPath+strlen(workingDirectoryName), pathGiven, strlen(pathGiven));
	}

	#ifdef _DEBUG_1_
	printf("\tPath transformed: %s\n", transformedPath);
	#endif

	return transformedPath;
}

void printMountedUsers() {
	printf("Printing mounted users\n");
	mounted_user *user = users;
	for (; user != NULL; user = user->next) {
		printf("\tID: %d, Folder ailias: %s\n", *user->id, user->folderAilias);
	}
	printf("\tEND\n\n");
}

int main(int argc, char const *argv[]) {

	DIR *workingDir;

	if (argc != 2) {
		perror("Server requires local folder to be served as argument only"); exit(1);
	} else {
		if ((workingDir = opendir(argv[1])) == NULL) {
			perror("Cannot open directory"); exit(1);
		}
		closedir(workingDir);
		printf("Opened directory successfully\n");

		workingDirectoryName = malloc(strlen(argv[1]));
		strcpy(workingDirectoryName, argv[1]);

		if (workingDirectoryName[strlen(workingDirectoryName)-1] != '/') {
			workingDirectoryName = malloc(strlen(workingDirectoryName)+1);
			strcpy(workingDirectoryName, argv[1]);
			workingDirectoryName[strlen(argv[1])] = '/';
		}
	}

	printf("Server running in directory: %s\n", workingDirectoryName);

	register_procedure("fsMount",   1, fsMount);
	register_procedure("fsUnmount", 2, fsUnmount);
	register_procedure("fsOpenDir", 2, fsOpenDir);
	register_procedure("fsCloseDir", 1, fsCloseDir);
	register_procedure("fsReadDir", 1, fsReadDir);
	register_procedure("fsOpen", 3, fsOpen);
	register_procedure("fsClose", 2, fsClose);
	printRegisteredProcedures();

    launch_server();
    return 0;
}
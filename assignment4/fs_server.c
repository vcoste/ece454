#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "fsOtherIncludes.h"

#if 0
#define _DEBUG_1_
#endif

// linked list of users that have been mounted
typedef struct MountedUser {
	int *id;
	char *folderAilias;
	DIR *dirStream;
	struct MountedUser *next;
} mounted_user;

typedef struct OpenedFiles {
	char *fileName;
	int fd;
	int mode;
	struct OpenedFiles *next;
} opened_file;

// when opening a file --> add to openedFiles list if it is not already there, otherewise just add fd to that list
// we should keep track of rights
// when closing search 


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
int removeOpenedFile(int);
////////////////////////////////////////////////////////////////////////////////
/// Start of implementation
////////////////////////////////////////////////////////////////////////////////
return_type r;
int id_counter = 0; // used to give a unique ID to each user
char* workingDirectoryName;
mounted_user *users = NULL;
opened_file *openedFiles = NULL;

/**
 * Invoked by the client app to mount a file system
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          0 on success, -1 with error code otherwise
 */
return_type fsMount(const int nparams, arg_type* a) {
	#ifdef _DEBUG_1_
	printf("in fsMount\n");
	#endif

	char *retBuffer = malloc(2*sizeof(int));
	int errorDescriptor;
	int returnValue;

	if (nparams != 1) {
		errorDescriptor = -1;
		returnValue = EINVAL;

		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val  = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}

	int clientID = addNewClient(a->arg_val, a->arg_size);

	#ifdef _DEBUG_1_
	printMountedUsers();
	#endif

	if (clientID >= 0) {
		errorDescriptor = 0;
		returnValue = clientID;

		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val  = retBuffer;
		r.return_size = 2*sizeof(int);
	} else {
		errorDescriptor = -1;
		returnValue = EUSERS;

		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val  = retBuffer;
		r.return_size = 2*sizeof(int);
	}

	#ifdef _DEBUG_1_
	printf("leaving fsMount\n");
	#endif
	return r;
}

/**
 * Invoked by the client app to unmount a file system
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          0 on success, -1 with error code otherwise
 */
return_type fsUnmount(const int nparams, arg_type* a) {
	#ifdef _DEBUG_1_
	printf("in fsUnmount (server side)\n");
	#endif

	int *retVal = malloc(sizeof(int));
	r.return_size = sizeof(int);

	if (nparams != 2) {
		*retVal  = EINVAL;
		r.return_val = retVal;
		return r;
	}

	if (removeClient((char*)a->arg_val, *(int*)a->next->arg_val) == -1) {
		#ifdef _DEBUG_1_
		printf("\tremoveClient failed\n");
		#endif
		*retVal = ENOENT;
		r.return_val = retVal;
		return r;
	}

	#ifdef _DEBUG_1_
	printMountedUsers();
	#endif

	*retVal = 0;
	r.return_val  = retVal;
	r.return_size = sizeof(int);
	return r;
}

/**
 * Invoked by the client app to open a directory
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          0 on success, -1 with error code otherwise
 */
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
		#ifdef _DEBUG_1_
		printf("\tError in fsOpenDir, incorrect arguments reveived\n");
		#endif
		*retVal = EINVAL;
		r.return_val = retVal;
		return r;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {
		#ifdef _DEBUG_1_
		printf("\tError in fsOpenDir, client not found\nID: %d\n", *(int*)a->arg_val);
		#endif
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

/**
 * Invoked by the client app to close a directory
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          0 on success, -1 with error code otherwise
 */
return_type fsCloseDir(const int nparams, arg_type* a) {

	int *retVal = malloc(sizeof(int));
	r.return_size = sizeof(int);

	if (nparams != 1 || a->arg_size != sizeof(int)) {
		#ifdef _DEBUG_1_
		printf("\tError in fsCLoseDir, incorrect arguments reveived\n");
		#endif
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

/**
 * Invoked by the client app to read a directory
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          0 with a file discriptor on success, -1 with error code otherwise
 */
return_type fsReadDir(const int nparams, arg_type* a) {
	char *retVal;
	int fileType;
	struct stat st;

	if (nparams != 1 || a->arg_size != sizeof(int)) {
		#ifdef _DEBUG_1_
		printf("\tError in fsReadDir, incorrect arguments reveived\n");
		#endif
		retVal = malloc(sizeof(int));
		*retVal = EINVAL;
		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {
		#ifdef _DEBUG_1_
		printf("\tError in fsReadDir, clientID not found: %d\n", *(int*)a->arg_val);
		#endif
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
		#ifdef _DEBUG_1_
		printf("\tAt end of folder\n");
		#endif
		
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

/**
 * Invoked by the client app to open a file
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          0 with a file descriptor on success, -1 with error code otherwise
 */
return_type fsOpen(const int nparams, arg_type* a) {
	#ifdef _DEBUG_1_
	printf("in fsOpen\n");
	#endif
	char *retBuffer = malloc(2*sizeof(int));
	int fd;
	int errorDescriptor = 0;
	int returnValue = 0;
	int openFlags;

	if (nparams != 3 || a->arg_size != sizeof(int)) {
		#ifdef _DEBUG_1_
		printf("\tError in fsOpen, incorrect arguments reveived\n");
		#endif
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
		#ifdef _DEBUG_1_
		printf("\tError in fsOpen, clientID not found: %d\n", *(int*)a->arg_val);
		#endif
		errorDescriptor = -1;
		returnValue = EACCES;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}

	// printf("user->folderAilias: %s, a->next->arg_val:%s\n", user->folderAilias, a->next->arg_val);
	char* fullFileName = transformPath(user->folderAilias, a->next->arg_val);

	// check if there any other writes for the corresponding file
	// if there is another write, ask the client to try again soon
	// otherwise add it to the list if all other checks do not fail
	
	if (openedFiles != NULL) {
		// make iterator
		opened_file *itr1 = openedFiles;
		for (; itr1 != NULL; itr1=itr1->next) {
			if(strcmp(itr1->fileName, fullFileName) == 0 && itr1->mode == 1) {
				#ifdef _DEBUG_1_
				printf("ask client to wait\n");
				#endif
				errorDescriptor = -2;
				returnValue = -2;
				memcpy(retBuffer, &errorDescriptor, sizeof(int));
				memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

				r.return_val = retBuffer;
				r.return_size = 2*sizeof(int);
				return r;
			}
		}
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
		#ifdef _DEBUG_1_
		printf("\tUnrecognized value for open mode\n");
		#endif
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
	
	if ((fd = open(fullFileName, openFlags)) == -1) {
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

	opened_file *newFileNode = (opened_file*)malloc(sizeof(opened_file));
	newFileNode->fd = fd;
	newFileNode->fileName = (char*)malloc(strlen(fullFileName));
	strcpy(newFileNode->fileName, fullFileName);
	newFileNode->mode = *(int*)a->next->next->arg_val;
	newFileNode->next = NULL;

	if(openedFiles == NULL) {
		openedFiles = newFileNode;
	} else {
		opened_file *itr = openedFiles;
		for (; itr != NULL; itr=itr->next) {
			if(itr->next == NULL) {
				itr->next = newFileNode;
				break;
			}
		}
	}

	returnValue = fd;
	memcpy(retBuffer, &errorDescriptor, sizeof(int));
	memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));
	r.return_val = retBuffer;
	r.return_size = 2*sizeof(int);
	return r;
}

/**
 * Invoked by the client app to close a file
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          0 on success, -1 with error code otherwise
 */
return_type fsClose(const int nparams, arg_type* a) {
	int *retVal = malloc(sizeof(int));

	if (nparams != 2 || a->arg_size != sizeof(int)) {
		#ifdef _DEBUG_1_
		printf("\tError in fsClose, incorrect arguments reveived\n");
		#endif
		*retVal = EINVAL;

		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}

	mounted_user *user;
	if ((user = findClientById(*(int*)a->arg_val)) == NULL) {
		#ifdef _DEBUG_1_
		printf("\tError in fsClose, clientID not found: %d\n", *(int*)a->arg_val);
		#endif
		*retVal = EACCES;

		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}


	if (close(*(int*)a->next->arg_val) == -1) {
		perror("fsClose()");
		*retVal = errno;

		r.return_val = retVal;
		r.return_size = sizeof(int);
		return r;
	}
	// when close is successful remove opened_file node
	int fd = *(int*)a->next->arg_val;
	#ifdef _DEBUG_1_
	printf("\nin fsClose, looking for fd: %d\n", fd);
	#endif
	
	if (removeOpenedFile(fd) == 0) {
		// success
	} else {
		// failure
	}

	*retVal = 0;
	r.return_val = retVal;
	r.return_size = sizeof(int);

	return r;
}

/**
 * Invoked by the client app to write to a file
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          the size of what was written to file, -1 with error code otherwise
 */
return_type fsWrite(const int nparams, arg_type* a) {
	// should get 2 params
	char *retBuffer = (char *)malloc(2*sizeof(int));
	int returnValue = 0;
	int errorDescriptor = 0;

	if (nparams != 2 || a->arg_size != sizeof(int)) {
		#ifdef _DEBUG_1_
		printf("\tError in fsWrite, incorrect arguments reveived\n");
		#endif
		errorDescriptor = -1;
		returnValue = EINVAL;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}
	#ifdef _DEBUG_1_
	printf("\tIn fsRead, parameter: fd: %d\n", *(int*)a->arg_val);
	#endif

	int fd = 0;
	fd = *(int*)a->arg_val;
	
	if (a->next->arg_size != 0) {
		returnValue = write(fd, a->next->arg_val, a->next->arg_size);
	}
	if (returnValue == -1) {
		errorDescriptor = -1;
		returnValue = errno;
	} 

	memcpy(retBuffer, &errorDescriptor, sizeof(int));
	memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

	r.return_val = retBuffer;
	r.return_size = 2*sizeof(int);
	return r;	
}

/**
 * Invoked by the client app to read from a file
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          the size of what was read from a file, -1 with error code otherwise
 */
return_type fsRead(const int nparams, arg_type* a) {
	// should get 2 params: fd and count
	
	int returnValue = 0;
	int errorDescriptor = 0;

	if (nparams != 2 || a->arg_size != sizeof(int) || a->next->arg_size != sizeof(int)) {
		#ifdef _DEBUG_1_
		printf("\tError in fsRead, incorrect arguments reveived\n");
		#endif
		char *retBuffer = (char *)malloc(2*sizeof(int));
		errorDescriptor = -1;
		returnValue = EINVAL;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	}

	#ifdef _DEBUG_1_
	printf("\tIn fsRead, parameters: fd: %d, count: %d\n", *(int*)a->arg_val, *(int*)a->next->arg_val);
	#endif

	int fd = 0;
	if (a->arg_size != 0) {
		fd = *(int*)a->arg_val;
	}
	int count = 0;
	void *buf;

	count = *(int*)a->next->arg_val;
	buf = (void*)malloc(count);
	
	returnValue = read(fd, buf, count);
	
	if (returnValue == -1) {
		perror("fsRead()");
		char *retBuffer = (char *)malloc(2*sizeof(int));
		errorDescriptor = -1;
		returnValue = errno;
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int);
		return r;
	} else {
		#ifdef _DEBUG_1_
		printf("\tSuccessfully read fd: %d, %s\n", fd, buf);
		#endif
		char *retBuffer = (char *)malloc(2*sizeof(int)+returnValue);
		memcpy(retBuffer, &errorDescriptor, sizeof(int));
		memcpy(retBuffer+sizeof(int), &returnValue, sizeof(int));
		memcpy(retBuffer+2*sizeof(int), buf, returnValue);

		r.return_val = retBuffer;
		r.return_size = 2*sizeof(int)+returnValue;
		return r;
	}
}

/**
 * Invoked by the client app to remove to a file
 * @param  nparams  number of parameters sent to the function
 * @param  a        linked list of parameters sent to the function
 * @return          0 on success, -1 with error code otherwise
 */
return_type fsRemove(const int nparams, arg_type* a) {

	int *retVal = malloc(sizeof(int));

	*retVal = 0;
	r.return_size = sizeof(int);

	if (nparams != 2 || a->arg_size != sizeof(int)) {
		#ifdef _DEBUG_1_
		printf("\tError in fsRemove, incorrect arguments reveived\n");
		#endif
		*retVal = EINVAL;

		r.return_val = retVal;
		return r;
	}
	mounted_user *user;
	user = findClientById(*(int*)a->arg_val);

	char* fullPathName = transformPath(user->folderAilias, (char*)a->next->arg_val);

	// TODO:
	// check for other file name in opened_file linked list, 
	// if it is not there remove file, 
	// otherwise ask client to try again later

	if (openedFiles != NULL) {
		// make iterator
		#ifdef _DEBUG_1_
		printf("searching thru openedFiles");
		printf("looking for :%s\n", fullPathName);
		#endif
		opened_file *itr1 = openedFiles;
		
		for (; itr1 != NULL; itr1=itr1->next) {
			#ifdef _DEBUG_1_
			printf("\tcomparing with :%s\n", itr1->fileName);
			#endif
			if(strcmp(itr1->fileName, fullPathName) == 0) {
				#ifdef _DEBUG_1_
				printf("ask client to wait\n");
				#endif
				*retVal = -2;
				r.return_val = retVal;
				return r;
			}
		}
	}
	#ifdef _DEBUG_1_
	printf("did not return from fsRemove yet, did not ask the client to wait \n");
	#endif
	
	if (user == NULL) {
		#ifdef _DEBUG_1_
		printf("\tError in fsRemove, clientID not found: %d\n", *(int*)a->arg_val);
		#endif
		*retVal = EACCES;

		r.return_val = retVal;
		return r;
	}

	#ifdef _DEBUG_1_
	printf("\tIn fsRemove, parameter:clientID: %d fname: %s|\n",*(int*)a->arg_val, (char*)a->next->arg_val);
	#endif

	
	if ((*retVal = remove(fullPathName) != 0)) {
		#ifdef _DEBUG_1_
		printf("\tError when removing file: %s\n", fullPathName);
		#endif
		perror("fsRemove()");
		*retVal = errno;
	}

	r.return_val = retVal;
	return r;
}

/**
 * Allocates a unique id for the current user
 * @return [int] id for current user
 */
int giveID() {
	int newID = id_counter;
	id_counter++;
	return newID;
}

/**
 * Adds a client to the users linked list of mounted users
 * @param      folderAilias   - local folder of client to be ailiased to server (mounted) directory
 * @param      folderNameSize - size in bytes of the folder name
 * @return     0 on success, -1 on failure
 */
int addNewClient(char* folderAilias, int folderNameSize) {
	
	mounted_user *newmounted_user = malloc(sizeof(mounted_user));
	newmounted_user->id           = malloc(sizeof(int));
	char* slash;

	#ifdef _DEBUG_1_
	printf("\tAdding new client with folder ailias: %s, string length: %d\n", folderAilias, folderNameSize);
	#endif

	if ((slash = strrchr(folderAilias, '/')) != NULL) {
		#ifdef _DEBUG_1_
		printf("\tSlash present in name, removing\n");
		#endif
		
		*slash = '\0';
		
		#ifdef _DEBUG_1_
		printf("\tUpdated name: %s\n", folderAilias);
		#endif
	}
	newmounted_user->folderAilias = malloc(strlen(folderAilias));

	*newmounted_user->id = giveID();
	strcpy(newmounted_user->folderAilias, folderAilias);
	newmounted_user->folderAilias[folderNameSize] = '\0';

	newmounted_user->next = NULL;

	#ifdef _DEBUG_1_
	printf("\tNew user created with id: %d and folder ailias: %s\n", *newmounted_user->id, newmounted_user->folderAilias);
	printf("Adding to linked list...\n");
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

/**
 * Finds the client in the users linked list that matches the id provided
 * @param  int [id] id of the desired client
 * @return     [mounted_user] NULL if not found
 */
int removeClient(char* folderAilias, int id) {
	
	mounted_user *prev = NULL;
	mounted_user *itr  = users;
	mounted_user *temp;

	for(; itr != NULL; prev = itr, itr = itr->next) {
		if (*itr->id == id && strcmp(itr->folderAilias, folderAilias) == 0) {
			
			temp = itr;
			if (prev == NULL) {
				itr = itr->next;
				users = itr;
			} else {
				prev->next = itr->next;
			}

			free(temp->id);
			free(temp->folderAilias);
			if (temp->dirStream != NULL) {
				free(temp->dirStream);
			}
			free(temp);
			temp = NULL;
			return 0;
		}
	}

	return -1;
}

/**
 * Finds the client in the users linked list that matches the id provided
 * @param  int [id] id of the desired client
 * @return     [mounted_user] NULL if not found
 */
mounted_user* findClientById(int id) {

	mounted_user *user = users;
	for (; user != NULL; user = user->next) {
		if (*user->id == id) {
			return user;
		}
	}

	return NULL;
}

/**
 * Helper function to remove a file that is open
 * @param  fd file descriptor of the file to remove
 * @return    0 on success, -1 otherwise
 */
int removeOpenedFile(int fd) {

	opened_file *prev = NULL;
	opened_file *itr  = openedFiles;
	opened_file *temp;

	for(; itr != NULL; prev = itr, itr = itr->next) {
		if (itr->fd == fd) {
			
			temp = itr;
			if (prev == NULL) {
				itr = itr->next;
			} else {
				prev->next = itr->next;
			}

			free(temp->fileName);
			free(temp);
			temp = NULL;
			return 0;
		}
	}
	return -1;
}

/**
 * Helper function to transform the name of the path to the server specific path
 * @param  folderAilias Alias used on the server for a file
 * @param  pathGiven    Path used by the client to reference file on server
 * @return              Transformed path name
 */
char* transformPath(char* folderAilias, char* pathGiven) {
	#ifdef _DEBUG_1_
	printf("\tIn transformPath, folderAilias: %s, pathGiven: %s\n", folderAilias, pathGiven);
	#endif
	int foundFolderAilias = 0;
	int indexOfFileName = 0; // looks for the start of alpha character, the path can potentially start with slashes and/or periods
	char* transformedPath;


	while (foundFolderAilias == 0) {
		if (pathGiven[indexOfFileName] == '/' || pathGiven[indexOfFileName] == '.') {
			indexOfFileName++;
		} else {
			foundFolderAilias = 1;
		}
	}
	// indexOfFileName now starts at where the filename acutally starts
	// compare with folderAilias to see if it needs to be replaced
	if (strncmp(&pathGiven[indexOfFileName], folderAilias, strlen(folderAilias)) == 0) {

		int slashOffset = 0;
		if (pathGiven[indexOfFileName+strlen(folderAilias)] == '/') {
			slashOffset = 1;
		}

		#ifdef _DEBUG_1_
		printf("\tFolder ailias in path given\n");
		#endif

		transformedPath = (char*)malloc(strlen(workingDirectoryName)+strlen(pathGiven)-strlen(folderAilias)-slashOffset);

		memcpy(transformedPath, pathGiven, indexOfFileName);
		memcpy(transformedPath+indexOfFileName, workingDirectoryName, strlen(workingDirectoryName)-slashOffset);
		strcpy(transformedPath+indexOfFileName+strlen(workingDirectoryName)-slashOffset, &pathGiven[indexOfFileName+strlen(folderAilias)]);
	} else {
		#ifdef _DEBUG_1_
		printf("\tNo folder ailias in path given, just appending workingDirectoryName\n");
		#endif
		transformedPath = (char*)malloc(strlen(workingDirectoryName)+strlen(pathGiven));
		memcpy(transformedPath, workingDirectoryName, strlen(workingDirectoryName));
		strcpy(transformedPath+strlen(workingDirectoryName), pathGiven);
	}

	#ifdef _DEBUG_1_
	printf("\tPath transformed: %s\n", transformedPath);
	#endif

	return transformedPath;
}

/**
 * Prints the ids in the mounted_users linked list
 */
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
		// printf("Opened directory successfully\n");

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
	register_procedure("fsWrite", 2, fsWrite);
	register_procedure("fsRead", 2, fsRead);
	register_procedure("fsRemove", 2, fsRemove);
	
	printRegisteredProcedures();

    launch_server();
    return 0;
}
#include "ece454_fs.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#if 0
#define _DEBUG_CLI_
#endif

typedef struct RemoteFolderServer {
	char *srvIpOrDomName;
	char *localFolderName;
	int *clientId;
	unsigned int srvPort;
	struct RemoteFolderServer *next;
} remote_folder_server;

typedef struct ClientFolderDescriptor {
	char *localFolderName;
	int clientFd;
	int serverFd;
	struct ClientFolderDescriptor *next;
} client_fd;

////////////////////////////////////////////////////////////////////////////////
///                          Helper Functions Definitions
////////////////////////////////////////////////////////////////////////////////
int addNewServer(remote_folder_server*);
remote_folder_server* findServerByFolderName(const char*);
int removeServerByFolderName(const char*);
int createClientFd(char*, int);
client_fd* getNodeFromClientFd(int);
int removeClientFd(int);
int removeClientFdByName(char*);
void printRemoteServers();

remote_folder_server *remoteFolderServers = NULL;
client_fd *clientServerFdMap = NULL;
int uniqueClientFdCounter = 0;

/**
 * This is used by the client application program to mount a remote server.
 * @param  srvIpOrDomName  the name or the IP address of the server to mount
 * @param  srvPort         port number in the server to mount
 * @param  localFolderName local name of the folder given to the mounted files
 * @return                 0 on success and -1 otherwise
 */
int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
	// do similar stuff as ass1 client app
	// save ip address and port number for subsequent remote calls

	#ifdef _DEBUG_CLI_
	printf("Calling fsMount to server\n");
	#endif

	remote_folder_server *newServer;
	if ((newServer = findServerByFolderName(localFolderName)) != NULL) {
		// printf("Server %s already exists as %s\n", localFolderName, newServer->localFolderName);
		errno = EEXIST;
		return -1;
	}

	return_type ans = make_remote_call( srvIpOrDomName,
										(int)srvPort,
										"fsMount", 1,
										strlen(localFolderName), (void *)(localFolderName));
	int containsError;
	int returnedValue;

	memcpy(&containsError, ans.return_val, sizeof(int));
	memcpy(&returnedValue, ans.return_val+sizeof(int), sizeof(int));
	
	if (containsError == 0) {
		//save id
		#ifdef _DEBUG_CLI_
		printf("Successfully returned, given clientID: %d\n", returnedValue);
		#endif
		
		newServer = (remote_folder_server*)malloc(sizeof(remote_folder_server));
		newServer->localFolderName = (char*)malloc(strlen(localFolderName));
		newServer->srvIpOrDomName  = (char*)malloc(strlen(srvIpOrDomName));
		newServer->clientId        = (int*) malloc(sizeof(int));

		*newServer->clientId = returnedValue;
		newServer->srvPort   = srvPort;
		newServer->next = NULL;

		strcpy(newServer->localFolderName, localFolderName);
		strcpy(newServer->srvIpOrDomName, srvIpOrDomName);

		addNewServer(newServer);

		return 0;
	}

	errno = returnedValue;

	return -1;
}

/**
 * This is used by the client application program to unmount a previously mounted file system
 * @param  localFolderName local name of the folder given to the mounted files
 * @return                 0 on success and -1 otherwise
 */
int fsUnmount(const char *localFolderName) {
	// 	the counterpart of fsMount() 
	// 	to unmount a remote ﬁlesystem that is referred to locally by localFolderName. 
	// 	Returns 0 on success, −1 on failure with errno set appropriately
	remote_folder_server *serverToUnmount = findServerByFolderName(localFolderName);
	// printf("serverName: %s, serverPort: %i, clientID: %i\n", serverToUnmount->srvIpOrDomName, serverToUnmount->srvPort, *serverToUnmount->clientId);
	return_type ans = make_remote_call( serverToUnmount->srvIpOrDomName,
										serverToUnmount->srvPort,
										"fsUnmount", 2,
										strlen(localFolderName), (void *)(localFolderName),
										sizeof(int), (void *)(serverToUnmount->clientId));
	
	int result = *(int*)(ans.return_val);

	if (result != 0) {
		errno = result;
		return -1;
	}

	if (removeServerByFolderName(localFolderName) == -1) {
		return -1;
	}

	return result;
}

/**
 * This is used by the client application program to open 
 * a directory in the remote file system
 * @param  folderName Local name of the mounted folder 
 * @return            Pointer to the file system directory
 */
FSDIR* fsOpenDir(const char *folderName) {
	
	remote_folder_server *server;
	if ((server = findServerByFolderName(folderName)) == NULL) {
		errno = ENOENT;
		return NULL;
	}
	// printf("in fsOpenDir, clientId: %d, folderName: %s\n", *server->clientId, folderName);

	return_type ans = make_remote_call( server->srvIpOrDomName,
										server->srvPort,
										"fsOpenDir", 2,
										sizeof(int), (void *)(server->clientId),
										strlen(folderName), (void *)(folderName));
	// printf("return_size: %d\n", ans.return_size);
	// printf("return_val: %d\n", *(int*)(ans.return_val));
	if (ans.return_size == 0) {
		#ifdef _DEBUG_CLI_
		// printf("return_size zero: %d\n", ans.return_size);
		#endif
		FSDIR *nice = NULL;
		//set errno before returning using a generic error
		errno = EBADMSG;
		return nice;
	} else if (*(int*)(ans.return_val) != 0) {
		#ifdef _DEBUG_CLI_
		// printf("return_val not zero: %s\n", strerror(*(int*)(ans.return_val)));
		#endif
		//set errno before returning using return_val as errno
		FSDIR *nice = NULL;
		errno = *(int*)(ans.return_val);
		return nice;
	} else {

		FSDIR* result = malloc(sizeof(FSDIR));
		result->folderName = (char*)malloc(strlen(folderName));

		strcpy(result->folderName, folderName);
		result->id     = *server->clientId;
		result->status = *(int*)(ans.return_val);
    	return result;
	}
}

/**
 * This is used by the client application program to close
 * a directory in the remote file system
 * @param  folder Pointer to the remote folder to close
 * @return        0 on success -1 otherwise
 */
int fsCloseDir(FSDIR *folder) {
	remote_folder_server *server = findServerByFolderName(folder->folderName);
	
	// printf("in fsCloseDir, clientId: %d\n", *server->clientId);
	
	return_type ans = make_remote_call( server->srvIpOrDomName,
										server->srvPort ,
										"fsCloseDir", 1,
										sizeof(int), (void *)(server->clientId));

	// printf("return_size: %d\n", ans.return_size);
	// printf("return_val: %d\n", *(int*)(ans.return_val));

	if (ans.return_size == 0) {
		#ifdef _DEBUG_CLI_
		printf("return_size zero: %d\n", ans.return_size);
		#endif
	
		//set errno before returning using a generic error
		errno = EBADMSG;
		return -1;
	
	} else if (*(int*)(ans.return_val) != 0) {
		#ifdef _DEBUG_CLI_
		printf("return_val not zero: %s\n", strerror(*(int*)(ans.return_val)));
		#endif
	
		errno = *(int*)(ans.return_val);
	
		//set errno before returning using return_val as errno
		return -1;
	} else {
		
		#ifdef _DEBUG_CLI_
		printf("return_val should be zero: %d\n", *(int*)(ans.return_val));
		#endif

		free(folder);
		folder = NULL;

    	return 0;
	}
}

/**
 * This is used by the client application program to get the next entry in the directory
 * @param  folder Pointer to the remote folder to close
 * @return        Pointer to the next entry in the directory
 */
struct fsDirent *fsReadDir(FSDIR *folder) {

	#ifdef _DEBUG_CLI_
	printf("in fsReadDir\n");
	#endif
	
	remote_folder_server *server = findServerByFolderName(folder->folderName);
	return_type ans = make_remote_call( server->srvIpOrDomName,
										server->srvPort ,
										"fsReadDir", 1,
										sizeof(int), (void *)(server->clientId));
	
	if (ans.return_size == 0) {
		return NULL;
	} else if (ans.return_size == sizeof(int)) {
		//set errno and return null
		errno = ans.return_size;
		return NULL;
	} else {
		//read entType and entName
		#ifdef _DEBUG_CLI_
			printf("return_size: %d\n", ans.return_size);
		#endif
	
		struct fsDirent *dir = (struct fsDirent *)malloc(sizeof(struct fsDirent));
		int *type = (int*)malloc(sizeof(int));
		memcpy( type, 
                ans.return_val, 
                sizeof(int));
		dir->entType = (unsigned char)(*type);
		char * index = (char*)(ans.return_val);
		index += sizeof(int);
		// printf("return_size: %d, size of unsigned char: %d\n", ans.return_size, sizeof(unsigned char));
		strcpy( dir->entName, (char*)index);
		return dir;
	}
}

/**
 * This is used by the client application program to open a remote file
 * @param  fname Name of the remote file to open
 * @param  mode  Mode in which to open the remote file, 
 * 1 for write permission and 0 for read permissions
 * @return       File descriptor number, positive integer, -1 on error
 */
int fsOpen(const char *fname, int mode) {
	#ifdef _DEBUG_CLI_
	printf("in fsOpen\n");
	#endif

	remote_folder_server *server;
	if ((server = findServerByFolderName(fname)) == NULL) {
		errno = ENOENT;
		return -1;
	}
	
	while(1) {
		return_type ans = make_remote_call( server->srvIpOrDomName,
											server->srvPort,
											"fsOpen", 3,
											sizeof(int), (void *)(server->clientId),
											strlen(fname), (void *)(fname),
											sizeof(int), (void *)(&mode));
		if (ans.return_size == 0) {
			//error set errno
			#ifdef _DEBUG_CLI_
			printf("return_size zero: %d\n", ans.return_size);
			#endif
			errno = EBADMSG;
			return -1;
		} else {
			int *status = (int*)malloc(sizeof(int));
			memcpy( status, 
	                ans.return_val, 
	                sizeof(int));
			char * index = (char*)(ans.return_val);
			index += sizeof(int);
			// printf("status: %d\n", *status);
			int *val = (int*)malloc(sizeof(int));
			memcpy( val, 
	        	    index, 
	            	sizeof(int));
			if (*status == 0){
				#ifdef _DEBUG_CLI_
				printf("positive val: %d\n", *val);
				#endif
				int clientFd = createClientFd(fname, *val);
				return clientFd;
			} else if(*status == -2) {
				// printf("fsOpen client will start to sleep\n");
				usleep(100000);
			} else {
				errno = *val;
				#ifdef _DEBUG_CLI_
				printf("negative val: %d, errno: %s\n", *val, strerror(errno));
				#endif
				return -1;
			}
		}
	}
}

/**
 * This is used by the client application program to close a remote file
 * @param  fd File descriptor of the file to close
 * @return    0 on success, -1 otherwise
 */
int fsClose(int fd) {
	#ifdef _DEBUG_CLI_
	printf("in fsClose\n");
	#endif

	client_fd *clientFdNode;
	if ((clientFdNode = getNodeFromClientFd(fd)) == NULL) {
		errno = EBADF;
		return -1;
	}

	remote_folder_server *server;
	if ((server = findServerByFolderName(clientFdNode->localFolderName)) == NULL) {
		errno = ENOENT;
		return -1;
	}
	return_type ans = make_remote_call( server->srvIpOrDomName,
										server->srvPort,
										"fsClose", 2,
										sizeof(int), (void *)(server->clientId),
										sizeof(int), (void *)(&clientFdNode->serverFd));
	if (ans.return_size == 0) {
		//error set errno
		#ifdef _DEBUG_CLI_
		printf("return_size zero: %d\n", ans.return_size);
		#endif
		errno = EBADMSG;
		return -1;
	} else if (*(int*)(ans.return_val) != 0) {
		#ifdef _DEBUG_CLI_
		printf("return_val not zero: %s\n", strerror(*(int*)(ans.return_val)));
		#endif
		errno = *(int*)(ans.return_val);
		//set errno before returning using return_val as errno
		return -1;
	} else {
		#ifdef _DEBUG_CLI_
		printf("return_val should be zero: %d\n", *(int*)(ans.return_val));
		#endif
		if (removeClientFd(clientFdNode->clientFd) == 0) {
    		return 0;
		} else {
			#ifdef _DEBUG_CLI_
			printf("did not removeClientFd, return_size zero: %d\n", ans.return_size);
			#endif
			errno = EBADMSG;
			return -1;
		}
	}
}

/**
 * This is used by the client application program to read from a remote file
 * @param  fd    File descriptor of the file to read from
 * @param  buf   Pointer to a buffer to hold the content of the read file
 * @param  count Size of the buffer to read
 * @return       Number of bytes actually read, -1 on error
 */
int fsRead(int fd, void *buf, const unsigned int count) {
	#ifdef _DEBUG_CLI_
	printf("in fsRead\n");
	#endif
	client_fd *clientFdNode = getNodeFromClientFd(fd);
	remote_folder_server *server = findServerByFolderName(clientFdNode->localFolderName);
	return_type ans = make_remote_call( server->srvIpOrDomName,
										server->srvPort,
										"fsRead", 2,
										sizeof(int), (void *)(&(clientFdNode->serverFd)),
										sizeof(unsigned int), (void *)(&count));
    
    int errorDescriptor;
	int returnedValue;

	if (ans.return_size == 0) {
		//error set errno
		#ifdef _DEBUG_CLI_
		printf("return_size zero: %d\n", ans.return_size);
		#endif
		errno = EBADMSG;
		return -1;
	} else if (ans.return_size == 2*sizeof(int)) {
		// there is an error
		memcpy(&errorDescriptor, ans.return_val, sizeof(int));
		memcpy(&returnedValue, ans.return_val+sizeof(int), sizeof(int));
		if (errorDescriptor == 0) {
			#ifdef _DEBUG_CLI_
			printf("return_size =  2*sizeof(int) but errorDescriptor = 0\n");
			#endif
			// errno = returnedValue;
			return -1;
		} else {
			#ifdef _DEBUG_CLI_
			printf("there is an error: %d\n", strerror(returnedValue));
			#endif
			errno = returnedValue;
			return -1;
		}
	} else if(ans.return_size > 2*sizeof(int)) {
		memcpy(&errorDescriptor, ans.return_val, sizeof(int));
		memcpy(&returnedValue, ans.return_val+sizeof(int), sizeof(int));
		if (errorDescriptor == 0) {
			memcpy(buf, ans.return_val+2*sizeof(int), returnedValue);
			return returnedValue;
		} else {
			errno = returnedValue;
			return -1;
		}
	} else {
		#ifdef _DEBUG_CLI_
		printf("Unrecognized return value from server\n");
		#endif
		return -1;
	}

	// if (errorDescriptor == -1) {
	// 	errno = returnedValue;
	// 	return -1;
	// }

	// return returnedValue;
}

/**
 * This is used by the client application program to write to a remote file
 * @param  fd    File descriptor of the file to read from
 * @param  buf   Pointer to a buffer to hold the content to write to the file
 * @param  count Size of the buffer to read
 * @return       Number of bytes actually written, -1 on error
 */
int fsWrite(int fd, const void *buf, const unsigned int count) {
	#ifdef _DEBUG_CLI_
	printf("in fsWrite\n");
	#endif
	client_fd *clientFdNode = getNodeFromClientFd(fd);
	remote_folder_server *server = findServerByFolderName(clientFdNode->localFolderName);
	return_type ans = make_remote_call( server->srvIpOrDomName,
										server->srvPort,
										"fsWrite", 2,
										sizeof(int), (void*)(&(clientFdNode->serverFd)),
										count, buf);
	int errorDescriptor;
	int returnedValue;

	if (ans.return_size == 0) {
		//error set errno
		#ifdef _DEBUG_CLI_
		printf("return_size zero: %d\n", ans.return_size);
		#endif
		errno = EBADMSG;
		return -1;
	} else if (ans.return_size == 2*sizeof(int)) {
		
		memcpy(&errorDescriptor, ans.return_val, sizeof(int));
		memcpy(&returnedValue, ans.return_val+sizeof(int), sizeof(int));
	} else {
		#ifdef _DEBUG_CLI_
		printf("\tUnrecognized return value from server\n");
		#endif
		return -1;
	}

	if (errorDescriptor == -1) {
		errno = returnedValue;
		return -1;
	}

	return returnedValue;
}

/**
 * This is used by the client application program to remove a remote file
 * @param  name Name of the file to remove
 * @return      0 on success, -1 otherwise
 */
int fsRemove(const char *name) {
    #ifdef _DEBUG_CLI_
	printf("in fsRemove\n");
	#endif
	remote_folder_server *server = findServerByFolderName(name);
	while(1) {
		return_type ans = make_remote_call( server->srvIpOrDomName,
											server->srvPort,
											"fsRemove", 2,
											sizeof(int), (void *)(server->clientId),
											strlen(name), name);

		if (ans.return_size != sizeof(int)) {
			#ifdef _DEBUG_CLI_
			printf("\treturn_size bad: %d\n", ans.return_size);
			#endif
			errno = EBADMSG;
			return -1;
		} else if (*(int*)ans.return_val == -2) {
			#ifdef _DEBUG_CLI_
			printf("client will start to sleep\n");
			#endif
			usleep(100000);
		} else if (*(int*)ans.return_val != 0) {
			#ifdef _DEBUG_CLI_
			printf("\tError when deleting file\n");
			#endif

			errno = *(int*)ans.return_val;
			perror("fsRemove");
			return -1;
		} else if (*(int*)ans.return_val == 0) {
			removeClientFdByName(name);
			return 0;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
/// Helper functions
////////////////////////////////////////////////////////////////////////////////////

/**
 * Helper function to add a new server to mount
 * @param  newServer Pointer to the new Server data to add
 * @return           0 on success, -1 otherwise
 */
int addNewServer(remote_folder_server *newServer) {
	if (remoteFolderServers == NULL) {
		remoteFolderServers = newServer;
		return 0;
	}

	remote_folder_server *itr = remoteFolderServers;
	for (; itr != NULL; itr = itr->next) {
		if (itr->next == NULL) {
			itr->next = newServer;
			return 0;
		}
	}
	#ifdef _DEBUG_CLI_
	printf("\tUnable to add new server: %s, %d, %s\n", newServer->srvIpOrDomName, newServer->srvPort, newServer->localFolderName);
	#endif

	return -1;
}

/**
 * Helper function to search up the server by folder name
 * @param  folderName Name of the folder to look up by
 * @return            Pointer to the server found
 */
remote_folder_server* findServerByFolderName(const char* folderName) {
	int lengthUntilSlash = 0;
	char* slash = "/";

	lengthUntilSlash = strcspn(folderName, slash);
	if (lengthUntilSlash == strlen(folderName)) { // didnt find any slashes
		#ifdef _DEBUG_CLI_
		printf("\tNo slashes in folderName\n");
		#endif
	}

	remote_folder_server *server = remoteFolderServers;
	for (; server != NULL; server = server->next) {
		if (strncmp(server->localFolderName, folderName, lengthUntilSlash) == 0) {
			#ifdef _DEBUG_CLI_
			printf("\tFound server:\tServerAddress: %s, Port: %d, Folder: %s, clientID: %d\n", server->srvIpOrDomName, server->srvPort, server->localFolderName, *server->clientId);
			#endif

			return server;
		}
	}
	#ifdef _DEBUG_CLI_
	printf("\tNo server found with foldername: %s\n", folderName);
	#endif

	return NULL;
}

/**
 * Helper function to remove a server by folder name
 * @param  folderName Name of the folder to remove the server
 * @return            0 on success, -1 otherwise
 */
int removeServerByFolderName(const char* folderName) {
	remote_folder_server *prev = NULL;
	remote_folder_server *itr  = remoteFolderServers;
	remote_folder_server *temp;

	for(; itr != NULL; prev = itr, itr = itr->next) {
		if (strcmp(itr->localFolderName, folderName) == 0) {
			
			temp = itr;
			if (prev == NULL) {
				itr = itr->next;
				remoteFolderServers = itr;
			} else {
				prev->next = itr->next;
			}

			free(temp->srvIpOrDomName);
			free(temp->localFolderName);
			free(temp->clientId);
			free(temp);
			temp = NULL;
			return 0;
		}
	}
	return -1;
}

/**
 * Helper function to create a client File Descriptor
 * @param  mountedFolderName Name of the remote mounted folder
 * @param  serverFd          File descriptor of the remote file 
 * @return                   Client file descriptor integer
 */
int createClientFd(char* mountedFolderName, int serverFd) {
	// TODO:
	// make unique fd for the client
	// map serverFd to clientFd
	
	client_fd *newClientFd;

	newClientFd = (client_fd*)malloc(sizeof(client_fd));
	newClientFd->localFolderName = (char*)malloc(strlen(mountedFolderName));

	strcpy(newClientFd->localFolderName, mountedFolderName);
	newClientFd->clientFd = uniqueClientFdCounter;
	newClientFd->serverFd = serverFd;
	newClientFd->next = NULL;

	uniqueClientFdCounter++;

	if (clientServerFdMap == NULL) {
		clientServerFdMap = newClientFd;
		#ifdef _DEBUG_CLI_
		printf("client fd linked list initalized\n");
		#endif
	} else {
		client_fd *itr = clientServerFdMap;
		for (; itr != NULL; itr = itr->next) {
			if (itr->next == NULL) {
				itr->next = newClientFd;
				#ifdef _DEBUG_CLI_
				printf("node added to client fd linked list\n");
				#endif
				break;
			}
		}
	}
	#ifdef _DEBUG_CLI_
	printf("new node:\n\tclientFd: %d\n\tserverFd: %d\n\tfolderName: %s\n", 
			newClientFd->clientFd, 
			newClientFd->serverFd, 
			newClientFd->localFolderName);
	#endif
	return newClientFd->clientFd;
}

/**
 * Helper funciton to find the mapping from the client file descriptor
 * @param  clientFd Client file descriptor
 * @return          Map between server and client file descriptor
 */
client_fd* getNodeFromClientFd(int clientFd) {
	#ifdef _DEBUG_CLI_
	printf("in getNodeFromClientFd, looking for clientFd: %d\n\n", clientFd);
	#endif
	client_fd *itr = clientServerFdMap;
	for (; itr != NULL; itr = itr->next) {
		#ifdef _DEBUG_CLI_
		printf("\tcomparing to :%d\n", itr->clientFd);
		#endif
		if (itr->clientFd == clientFd) {
			#ifdef _DEBUG_CLI_
			printf("returned node:\n\tclientFd: %d\n\tserverFd: %d\n\tfolderName: %s\n", 
			itr->clientFd, 
			itr->serverFd, 
			itr->localFolderName);
			#endif
			return itr;
		}
	}
	return NULL;
}

/**
 * Helper funciton to remove the mapping from the client file descriptor
 * @param  clientFd Client file descriptor
 * @return          0 on success, -1 otherwise
 */
int removeClientFd(int clientFd) {
	#ifdef _DEBUG_CLI_
	printf("removing clientFd: %d\n", clientFd);
	#endif
	client_fd *prev = NULL;
	client_fd *itr  = clientServerFdMap;
	client_fd *temp;

	for(; itr != NULL; prev = itr, itr = itr->next) {
		if (itr->clientFd == clientFd) {
			
			temp = itr;
			if (prev == NULL) {
				itr = itr->next;
				clientServerFdMap = itr;
			} else {
				prev->next = itr->next;
			}

			free(temp->localFolderName);
			free(temp);
			temp = NULL;
			return 0;
		}
	}
	return -1;
}

/**
 * Helper funciton to remove the mapping from the client file name
 * @param  name Name of the file to remove
 * @return      0 on success, -1 otherwise
 */
int removeClientFdByName(char *name) {
	#ifdef _DEBUG_CLI_
	printf("removing clientFd from linked list with name: %s\n", name);
	#endif
	client_fd *prev = NULL;
	client_fd *itr  = clientServerFdMap;
	client_fd *temp;

	for(; itr != NULL; prev = itr, itr = itr->next) {
		if (strcmp(itr->localFolderName, name) == 0) {
			
			temp = itr;
			if (prev == NULL) {
				itr = itr->next;
				clientServerFdMap = itr;
			} else {
				prev->next = itr->next;
			}

			free(temp->localFolderName);
			free(temp);
			temp = NULL;
			return 0;
		}
	}
	return -1;
}

/**
 * Helper funciton to print the list of remote servers
 */
void printRemoteServers() {
	#ifdef _DEBUG_CLI_
	printf("Printing remote servers\n");
	#endif
	remote_folder_server *server = remoteFolderServers;
	for (; server != NULL; server = server->next) {
		printf("\tServerAddress: %s, Port: %d, Folder: %s, clientID: %d\n", server->srvIpOrDomName, server->srvPort, server->localFolderName, *server->clientId);
	}
	printf("\tEND\n\n");
}

/**
 * Helper funciton to print the list of client file descriptors
 */
void printClientFds() {
	printf("Printing File descriptors\n");
	client_fd *itr = clientServerFdMap;
	for (; itr != NULL; itr = itr->next) {
		printf("\tFolder name: %s, clientFd: %d, serverFd: %d\n", itr->localFolderName, itr->clientFd, itr->serverFd);
	}
	printf("\tEND\n\n");
}
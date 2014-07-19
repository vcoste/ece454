#include "ece454_fs.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#if 1
#define _DEBUG_CLI_
#endif

struct remoteFolderServer {
	char *name;
	unsigned int port;
};

struct remoteFolderServer server;
// remoteFolderServer *server = malloc(sizeof(struct remoteFolderServer));
struct fsDirent dent;
int clientId = -1;

int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
	// do similar stuff as ass1 client app
	// save ip address and port number for subsequent remote calls
	server.name = (char *)malloc(strlen(localFolderName));
	strcpy(server.name, srvIpOrDomName);	
	server.port = srvPort;
	#ifdef _DEBUG_CLI_
	printf("Calling fsMount to server\n");
	#endif
	return_type ans = make_remote_call( srvIpOrDomName,
										(int)srvPort,
										"fsMount", 1,
										strlen(localFolderName), (void *)(localFolderName));
	int result = *(int*)(ans.return_val);
	
	if (result >= 0) {
		//save id
		#ifdef _DEBUG_CLI_
		printf("Successfully returned, given clientID: %d\n", result);
		#endif
		clientId = result;
		return 0;
	}

	return -1;
}

int fsUnmount(const char *localFolderName) {
	// 	the counterpart of fsMount() 
	// 	to unmount a remote ﬁlesystem that is referred to locally by localFolderName. 
	// 	Returns 0 on success, −1 on failure with errno set appropriately
	printf("serverName: %s, serverPort: %i, clientID: %i\n", server.name, server.port, clientId);
	return_type ans = make_remote_call( server.name,
										server.port ,
										"fsUnmount", 2,
										strlen(localFolderName), (void *)(localFolderName),
										sizeof(int), (void *)(&clientId));
	int result = *(int*)(ans.return_val);
	return result;
}

FSDIR* fsOpenDir(const char *folderName) {
	printf("in fsOpenDir, clientId: %d, folderName: %s\n", clientId, folderName);
	return_type ans = make_remote_call( server.name,
										server.port,
										"fsOpenDir", 2,
										sizeof(int), (void *)(&clientId),
										strlen(folderName), (void *)(folderName));
	printf("return_size: %d\n", ans.return_size);
	printf("return_val: %d\n", *(int*)(ans.return_val));
	if (ans.return_size == 0) {
		#ifdef _DEBUG_CLI_
		printf("return_size zero: %d\n", ans.return_size);
		#endif
		FSDIR *nice = NULL;
		//set errno before returning using a generic error
		errno = EBADMSG;
		return nice;
	} else if (*(int*)(ans.return_val) != 0) {
		#ifdef _DEBUG_CLI_
		printf("return_val not zero: %d\n", strerror(*(int*)(ans.return_val)));
		#endif
		//set errno before returning using return_val as errno
		FSDIR *nice = NULL;
		errno = *(int*)(ans.return_val);
		return nice;
	} else {
		#ifdef _DEBUG_CLI_
		printf("return_val should be zero: %d\n", (int*)(ans.return_val));
		#endif
		FSDIR* result;
		result->id = clientId;
		result->status = *(int*)(ans.return_val);
    	return result;
	}
}

int fsCloseDir(FSDIR *folder) {
	printf("in fsCloseDir, clientId: %d\n", clientId);
	return_type ans = make_remote_call( server.name,
										server.port ,
										"fsCloseDir", 1,
										sizeof(int), (void *)(&clientId));
	printf("return_size: %d\n", ans.return_size);
	printf("return_val: %d\n", *(int*)(ans.return_val));
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
    	return 0;
	}
}

struct fsDirent *fsReadDir(FSDIR *folder) {
	#ifdef _DEBUG_CLI_
	printf("in fsReadDir\n");
	#endif
	return_type ans = make_remote_call( server.name,
										server.port ,
										"fsReadDir", 1,
										sizeof(int), (void *)(&clientId));
	
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
		printf("return_size: %d, size of unsigned char: %d\n", ans.return_size, sizeof(unsigned char));
		strcpy( dir->entName, (char*)index);
		return dir;
	}
}

int fsOpen(const char *fname, int mode) {
	#ifdef _DEBUG_CLI_
	printf("in fsOpen\n");
	#endif
	return_type ans = make_remote_call( server.name,
										server.port ,
										"fsOpen", 3,
										sizeof(int), (void *)(&clientId),
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
		printf("status: %d\n", *status);
		int *val = (int*)malloc(sizeof(int));
		memcpy( val, 
        	    index, 
            	sizeof(int));
		if (*status == 0){
			#ifdef _DEBUG_CLI_
			printf("positive val: %d\n", *val);
			#endif
			return *val;
		} else {
			errno = *val;
			#ifdef _DEBUG_CLI_
			printf("negative val: %d, errno: %s\n", *val, strerror(errno));
			#endif
			return -1;
		}
	}
}

int fsClose(int fd) {
	#ifdef _DEBUG_CLI_
	printf("in fsClose\n");
	#endif
	return_type ans = make_remote_call( server.name,
										server.port ,
										"fsClose", 2,
										sizeof(int), (void *)(&clientId),
										sizeof(int), (void *)(&fd));
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
    	return 0;
	}
}

int fsRead(int fd, void *buf, const unsigned int count) {
	#ifdef _DEBUG_CLI_
	printf("in fsRead\n");
	#endif
	return_type ans = make_remote_call( server.name,
										server.port ,
										"fsRead", 2,
										sizeof(int), (void *)(&fd),
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
			printf("return_size =  2*sizeof(int) but errorDescriptor = 0\n");
			// errno = returnedValue;
			return -1;
		} else {
			printf("there is an error: %d\n", strerror(returnedValue));
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
		printf("Unrecognized return value from server\n");
		return -1;
	}

	// if (errorDescriptor == -1) {
	// 	errno = returnedValue;
	// 	return -1;
	// }

	// return returnedValue;
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
	#ifdef _DEBUG_CLI_
	printf("in fsWrite\n");
	#endif
	return_type ans = make_remote_call( server.name,
										server.port,
										"fsWrite", 2,
										sizeof(int), (void*)(&fd),
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
		printf("Unrecognized return value from server\n");
		return -1;
	}

	if (errorDescriptor == -1) {
		errno = returnedValue;
		return -1;
	}

	return returnedValue;
}

int fsRemove(const char *name) {
    return(remove(name));
}
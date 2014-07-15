#include "ece454_fs.h"
#include <string.h>
#include <stdlib.h>

#if 1
#define _DEBUG_CLI_
#endif

struct remoteFolderServer {
	char *name[20];
	unsigned int port;
};

struct remoteFolderServer server;
// remoteFolderServer *server = malloc(sizeof(struct remoteFolderServer));
struct fsDirent dent;
int clientId = -1;

int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
	// do similar stuff as ass1 client app
	// save ip address and port number for subsequent remote calls
	strcpy(server.name, srvIpOrDomName);	
	server.port = srvPort;
	#ifdef _DEBUG_CLI_
	printf("Calling fsMount to server\n");
	#endif
	return_type ans = make_remote_call( srvIpOrDomName,
										(int)srvPort,
										"fsMount", 0,
										sizeof(localFolderName), (void *)(localFolderName));
	int result = *(int*)(ans.return_val);
	if (result == -1) {
		return result;
	} else if (result >= 0) {
		//save id
		#ifdef _DEBUG_CLI_
		printf("Successfully returned, given clientID: %d\n", result);
		#endif
		clientId = result;
		return 0;
	}
}

int fsUnmount(const char *localFolderName) {
	// 	the counterpart of fsMount() 
	// 	to unmount a remote ﬁlesystem that is referred to locally by localFolderName. 
	// 	Returns 0 on success, −1 on failure with errno set appropriately
	printf("serverName: %s, serverPort: %i, clientID: %i\n", server.name, server.port, clientId);
	return_type ans = make_remote_call( server.name,
										server.port ,
										"fsUnmount", 2,
										sizeof(localFolderName), (void *)(localFolderName),
										sizeof(int), (void *)(&clientId));
	int result = *(int*)(ans.return_val);
	return result;
}

FSDIR* fsOpenDir(const char *folderName) {
	return_type ans = make_remote_call( server.name,
										server.port ,
										"fsOpenDir", 1,
										sizeof(folderName), (void *)(folderName));
	if (ans.return_size) {
		FSDIR *nice = NULL;
		return nice;
	}
    return NULL; // TODO return FSDIR new implementation
}

int fsCloseDir(FSDIR *folder) {
    return(closedir(folder));
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;
    struct dirent *d = readdir(folder);

    if(d == NULL) {
	if(errno == initErrno) errno = 0;
	return NULL;
    }

    if(d->d_type == DT_DIR) {
	dent.entType = 1;
    }
    else if(d->d_type == DT_REG) {
	dent.entType = 0;
    }
    else {
	dent.entType = -1;
    }

    memcpy(&(dent.entName), &(d->d_name), 256);
    return &dent;
}

int fsOpen(const char *fname, int mode) {
    int flags = -1;

    if(mode == 0) {
	flags = O_RDONLY;
    }
    else if(mode == 1) {
	flags = O_WRONLY | O_CREAT;
    }

    return(open(fname, flags, S_IRWXU));
}

int fsClose(int fd) {
    return(close(fd));
}

int fsRead(int fd, void *buf, const unsigned int count) {
    return(read(fd, buf, (size_t)count));
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    return(write(fd, buf, (size_t)count)); 
}

int fsRemove(const char *name) {
    return(remove(name));
}
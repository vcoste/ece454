#include "ece454_fs.h"
#include <string.h>

struct remoteFolderServer {
	char *name;
	unsigned int port;
};

struct remoteFolderServer server;

int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
	// do similar stuff as ass1 client app
	// save ip address and port number for subsequent remote calls
	
	strcpy (server.name,srvIpOrDomName);
	server.port = srvPort;
	return_type ans = make_remote_call( srvIpOrDomName,
										srvPort,
										"fsMount", 1,
										sizeof(localFolderName), (void *)(localFolderName));
	int result = *(int*)ans.return_val;

	return result;
}

// extern int fsUnmount(const char *localFolderName);
// extern FSDIR* fsOpenDir(const char *folderName);
// extern int fsCloseDir(FSDIR *);
// extern struct fsDirent *fsReadDir(FSDIR *);
// extern int fsOpen(const char *fname, int mode);
// extern int fsClose(int);
// extern int fsRead(int fd, void *buf, const unsigned int count);
// extern int fsWrite(int fd, const void *buf, const unsigned int count);
// extern int fsRemove(const char *name);

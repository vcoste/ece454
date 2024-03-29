/* 
 * Mahesh V. Tripunitara
 * University of Waterloo
 * You specify what goes in this file. I just have a "dummy"
 * specification of the FSDIR type.
 */

#ifndef _ECE_FS_OTHER_INCLUDES_
#define _ECE_FS_OTHER_INCLUDES_
#include <sys/types.h>
#include <dirent.h>
#include "simplified_rpc/ece454rpc_types.h"

typedef struct fs_dir {
	int id;
	int status;
	char* folderName;
} FSDIR;

#endif

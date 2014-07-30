#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 
int main(int argc, char *argv[]) {
    if (argc < 5) {
        printf("Need: <ip1> <port1> <ip2> <port2> \n");
        exit(1);
    }
    const char *ip = argv[1];
    const int port = atoi(argv[2]);
 
    const char *ip2 = argv[3];
    const int port2 = atoi(argv[4]);

    printf("1\n");
    if (fsMount(ip, port, "foo") < 0) {
        perror("fsMount"); exit(1);
    }
 
    printf("2\n");
    if (fsMount(ip, port, "foo") >= 0) {
        printf("ERROR: Should not be able to mount the same folder twice\n");
        exit(1);
    }
 
    printf("3\n");
    if (fsMount(ip2, port2, "foo2") < 0) {
        perror("fsMount"); exit(1);
    }
 
    int fd;
    printf("4\n");
    if ((fd = fsOpen("foo/file1.txt", 0)) < 0) {
        perror("fsOpen"); exit(1);
    }
 
    int fd2;
    printf("5\n");
    if ((fd2 = fsOpen("foo2/file2.txt", 0)) < 0) {
        perror("fsOpen"); exit(1);
    }
 
    printf("6\n");
    if (fsOpen("foo/file2.txt", 0) >= 0) {
        printf("ERROR: There should be no file2.txt in server#1\n");
        exit(1);
    }
 
    printf("7\n");
    if (fsOpen("foo2/file1.txt", 0) >= 0) {
        printf("ERROR: There should be no file1.txt in server#2\n");
        exit(1);
    }
 
    printf("8\n");
    if (fsOpen("foo3/file1.txt", 0) >= 0) {
        printf("ERROR: There is no mounted folder called foo3\n");
        exit(1);
    }
 
    char buf[3001];
    int bytesread;
    int i = 0;
    printf("9\n");
    bytesread = fsRead(fd, (void *)buf, 3000);
    *((char *) buf + bytesread) = '\0';
    if (strncmp(buf, "Hello World1", 12) != 0) {
        printf("ERROR: Read on file1.txt should return 'Hello World1' \n");
        exit(1);
    }
 
    printf("10\n");
    bytesread = fsRead(fd2, (void *)buf, 3000);
    *((char *) buf + bytesread) = '\0';
    if (strncmp(buf, "Hello World2", 12) != 0) {
        printf("ERROR: Read on file2.txt should return 'Hello World2' \n");
        exit(1);
    }
 
    printf("11\n");
    while ((bytesread = fsRead(fd2, (void *)buf, 3000)) > 0) {
        *((char *) buf + bytesread) = '\0';
        printf("%s", (char *) buf);
        i += 1;
    }
    printf("\n");
 
    printf("12\n");
    if (fsRemove("foo/file1.txt") >= 0) {
        printf("ERROR: Should not be able to delete file1.txt while its open\n"); exit(1);
    }
 
    printf("13\n");
    if (fsRemove("foo") >= 0 || fsRemove("foo2") >= 0) {
        printf("ERROR: Should not be able to delete mounted folder\n"); exit(1);
    }
 
    printf("14\n");
    if (fsClose(fd) < 0) {
        perror("fsClose"); exit(1);
    }
 
    printf("15\n");
    if (fsClose(fd2) < 0) {
        perror("fsClose"); exit(1);
    }
 
    printf("16\n");
    if (fsUnmount("foo") < 0) {
        perror("fsUnmount"); exit(1);
    }
 
    printf("17\n");
    if (fsUnmount("foo2") < 0) {
        perror("fsUnmount"); exit(1);
    }
 
    printf("All tests passed!\n");
    return 0;
}

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>

// #include "ece454_fs.h"

// void printBuf(char *buf, int size) {
//     /* Should match the output from od -x */
//     int i;
//     for(i = 0; i < size; ) {
//     if(i%16 == 0) {
//         printf("%08o ", i);
//     }

//     int j;
//     for(j = 0; j < 16;) {
//         int k;
//         for(k = 0; k < 2; k++) {
//         if(i+j+(1-k) < size) {
//             printf("%02x", (unsigned char)(buf[i+j+(1-k)]));
//         }
//         }

//         printf(" ");
//         j += k;
//     }

//     printf("\n");
//     i += j;
//     }
// }

// void test1(int argc, char *argv[]) {
//     char *dirname = NULL;

//     if(argc > 3) 
//         dirname = argv[3];
//     else {
//         dirname = (char *)calloc(strlen(".")+1, sizeof(char));
//         strcpy(dirname, ".");
//     }
//     printf("dirname: %s\n", dirname);
//     printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
//     // printf("fsUnmount(): %d\n", fsUnmount(dirname));
//     FSDIR *fd = fsOpenDir(dirname);
//     if(fd == NULL) {
//         perror("fsOpenDir"); exit(1);
//     } else {
//         printf("no errors printed so far\n");
//     }
//     struct fsDirent *fdent = NULL;
//     fdent = fsReadDir(fd);
//     if (fdent != NULL) {
//         printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
//     } else {
//         printf("\tReached end of folder \n");
//     }
//     fdent = fsReadDir(fd);
//     if (fdent != NULL) {
//         printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
//     } else {
//         printf("\tReached end of folder \n");
//     }
//     fdent = fsReadDir(fd);
//     if (fdent != NULL) {
//         printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
//     } else {
//         printf("\tReached end of folder \n");
//     }
    
//     char fname[256];
//     sprintf(fname, "%s", "asdf");

//     int ff = fsOpen(fname, 1);
//     if(ff < 0) {
//         perror("fsOpen(write)"); exit(1);
//     }

//     char buf[256];
//     sprintf(buf, "Wrote this sweet deal with more text");
//     if(fsWrite(ff, buf, 256) < 256) {
//         fprintf(stderr, "fsWrite() wrote fewer than 256\n");
//     }

//     printf("fsClose(ff): %d\n", fsClose(ff));

//     ff = fsOpen(fname, 0);
//     if(ff < 0) {
//         perror("fsOpen(read)"); exit(1);
//     }

//     if (fsRemove(fname) != 0) {
//         perror("tried to fsRemove");
//         printf("tried to fsRemove\n");
//     } else {
//         printf("\tSuccessfully removed %s\n", fname);
//     }
//     fd = fsOpenDir(dirname);
//     fdent = fsReadDir(fd);
//     if (fdent != NULL) {
//         printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
//     } else {
//         printf("\tReached end of folder \n");
//     }
//     fdent = fsReadDir(fd);
//     if (fdent != NULL) {
//         printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
//     } else {
//         printf("\tReached end of folder \n");
//     }
//     fdent = fsReadDir(fd);
//     if (fdent != NULL) {
//         printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
//     } else {
//         printf("\tReached end of folder \n");
//     }
//     fdent = fsReadDir(fd);
//     if (fdent != NULL) {
//         printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
//     } else {
//         printf("\tReached end of folder \n");
//     }

//     char rdBuf[256];
//     if(fsRead(ff, rdBuf, 256) < 256) {
//         fprintf(stderr, "fsRead() read fewer than 256: %s\n", rdBuf);
//     } else {
//         printf("Successfully read: %s\n", rdBuf);
//     }


//     // printf("fsWrite(ff, buf, 256): %d\n", fsWrite(ff, buf, 256));
    
//     printf("fsClose(ff): %d\n", fsClose(ff));
//     // if(fsClose(ff) < 0) {
//     //     perror("fsClose"); exit(1);
//     // }

//     printf("Reached the end without errors\n");
// }

// void testMounting(char* ip, int port, char* dirname) {
//     printf("Test for mounting\n");
//     printf("\tdirname: %s\n", dirname);
//     printf("\tfsMount(): %d\n", fsMount(ip, port, dirname));
// }

// void testUnmounting(char* path) {
//     printf("Test for unmounting\n");
//     printf("\tfsUnmount(): %d\n", fsUnmount(path));
// }

// void testOpenDir(char* path) {
//     printf("Test for openDir\n");

//     FSDIR* dir;
//     if ((dir = fsOpenDir(path)) == NULL) {
//         printf("Call failed\n");
//         return;
//     }
//     printf("dir->folderName: %s, id: %d, status: %d\n", dir->folderName, dir->id, dir->status);

//     struct fsDirent* entry = fsReadDir(dir);

//     while(entry != NULL) {
//         printf("Entry read: Name: %s, type: %d\n", entry->entName, entry->entType);
//         entry = fsReadDir(dir);
//     }
//     printf("entries now null\n");
// }

// void testRemove(int argc, char *argv[]) {
//     char *dirname = NULL;

//     if(argc > 3) 
//         dirname = argv[3];
//     else {
//         dirname = (char *)calloc(strlen(".")+1, sizeof(char));
//         strcpy(dirname, ".");
//     }
//     printf("dirname: %s\n", dirname);
//     printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
//     // printf("fsUnmount(): %d\n", fsUnmount(dirname));
//     FSDIR *fd = fsOpenDir(dirname);
//     if(fd == NULL) {
//         perror("fsOpenDir"); exit(1);
//     } else {
//         printf("no errors printed so far\n");
//     }

//     char fname[256];
//     sprintf(fname, "%s", "asdf");

//     int ff = fsOpen(fname, 1);
//     if(ff < 0) {
//         perror("fsOpen(write)"); exit(1);
//     }

//     if (fsRemove(fname) != 0) {
//         perror("tried to fsRemove");
//         printf("tried to fsRemove\n");
//     } else {
//         printf("\tSuccessfully removed %s\n", fname);
//     }
// }

// void testOpen(char* fname, int mode) {

//     int ff = fsOpen(fname, mode);
//     if(ff < 0) {
//         perror("fsOpen() error");
//     } else {
//         printf("\tfd received: %d\n", ff);
//     }
// }

// void testClose(int fd) {
//     if (fsClose(fd) == -1) {
//         perror("fsClose() error");
//     } else {
//         printf("\tSuccessfully closed file\n");
//     }
// }

// void testWrite(int fd, const void* buf) {
//     int count = strlen(buf);
//     int result;
//     printf("\tabout to write %d bytes\n", count);
//     result = fsWrite(fd, buf, count);
    
//     if (result == -1) {
//         perror("Write failed");
//         return;
//     }
//     printf("\twrote: %d bytes\n", result);
// }

// int main(int argc, char *argv[]) {
    
//     int option;
//     char ip[16];
//     int port;
//     int mode;
//     int fd;
//     char path[80];
//     char buf[256];
//     size_t ln;

//     int keep_running = 1;
//     int requireServerInfo = 1;

//     if (argc == 3) {
//         strcpy(ip, argv[1]);
//         port = atoi(argv[2]);
//         printf("Running tests with IP: %s, port: %d\n", ip, port);
//         requireServerInfo = 0;
//     }

//     while (keep_running) {
            
//         printf("\nTest cases, yay\n");
//         printf("#\tTestName\t\t(Test args)\n");
//         printf("1\ttestMounting\t\t(ip, port#, dirname)\n");
//         printf("2\ttestUnmounting\t\t(dirname)\n");
//         printf("3\ttestOpenDir\t\t(dirname)\n");
//         printf("4\ttestOpen\t\t(dirname)\n");
//         printf("5\ttestClose\t\t(fd)\n");
//         printf("6\ttestWrite\t\t(fd, buffer, count)\n");
//         printf("98\tPrintClientFds\n");
//         printf("99\tPrintMountedServers\n");
//         printf("0\tEnd Testing Situation\n");

//         fscanf(stdin, "%d", &option);

//         switch (option) {
//             case 1:
//                 if (requireServerInfo) {
//                     printf("Enter server address:");
//                     fscanf(stdin, "%s", ip);
//                     printf("\tIP received: %s\nEnter port number: ", ip);
//                     fscanf(stdin, "%d", &port);
//                     printf("\tPort number: %d\n", port);
//                 }
                
//                 printf("Enter folder to mount: ");
//                 fscanf(stdin, "%s", path);
//                 printf("Calling testMounting(ip=%s,port=%d,dirname=%s)\n\n", ip, port, path);
//                 testMounting(ip,port,path);
//                 break;
//             case 2:
//                 printf("Enter folder to unmount:");
//                 fscanf(stdin, "%s", path);
//                 printf("Calling testUnmounting(dirname=%s)\n", path);
//                 testUnmounting(path);
//                 break;
//             case 3:
//                 printf("Enter folder to open:\n");
//                 fscanf(stdin, "%s", path);
//                 testOpenDir(path);
//             case 4:
//                 printf("Enter file name to open: ");
//                 fscanf(stdin, "%s", path);
//                 printf("File name entered: %s\n", path);
//                 printf("Enter mode to open: ");
//                 fscanf(stdin, "%d", &mode);
//                 testOpen(path, mode);
//                 break;
//             case 5:
//                 printf("Enter file descriptor to close: ");
//                 fscanf(stdin, "%d", &fd);
//                 testClose(fd);
//                 break;
//             case 6:
//                 printf("Enter file descriptor to write to: ");
//                 fscanf(stdin, "%d", &fd);
//                 printf("Enter text to write to file: ");
//                 fscanf(stdin, "%s", buf);
//                 testWrite(fd, buf);
//                 break;
//             case 98:
//                 printClientFds();
//                 break;
//             case 99:
//                 printRemoteServers();
//                 break;
//             default:
//                 keep_running = 0;
//                 printf("Ending test situation\n");
//                 break;
//         }
//     }
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

#define MAX_LINE_SIZE 4096
#define MAX_WORD_SIZE 512

void printBuf(char *buf, int size) {
    /* Should match the output from od -x */
    int i;
    for(i = 0; i < size; ) {
    if(i%16 == 0) {
        printf("%08o ", i);
    }

    int j;
    for(j = 0; j < 16;) {
        int k;
        for(k = 0; k < 2; k++) {
        if(i+j+(1-k) < size) {
            printf("%02x", (unsigned char)(buf[i+j+(1-k)]));
        }
        }

        printf(" ");
        j += k;
    }

    printf("\n");
    i += j;
    }
}

void test1(int argc, char *argv[]) {
    char *dirname = NULL;

    if(argc > 3) 
        dirname = argv[3];
    else {
        dirname = (char *)calloc(strlen(".")+1, sizeof(char));
        strcpy(dirname, ".");
    }
    printf("dirname: %s\n", dirname);
    printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
    // printf("fsUnmount(): %d\n", fsUnmount(dirname));
    FSDIR *fd = fsOpenDir(dirname);
    if(fd == NULL) {
        perror("fsOpenDir"); exit(1);
    } else {
        printf("no errors printed so far\n");
    }
    struct fsDirent *fdent = NULL;
    fdent = fsReadDir(fd);
    if (fdent != NULL) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    } else {
        printf("\tReached end of folder \n");
    }
    fdent = fsReadDir(fd);
    if (fdent != NULL) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    } else {
        printf("\tReached end of folder \n");
    }
    fdent = fsReadDir(fd);
    if (fdent != NULL) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    } else {
        printf("\tReached end of folder \n");
    }
    
    char fname[256];
    sprintf(fname, "%s", "asdf");

    int ff = fsOpen(fname, 1);
    if(ff < 0) {
        perror("fsOpen(write)"); exit(1);
    }

    char buf[256];
    sprintf(buf, "Wrote this sweet deal with more text");
    if(fsWrite(ff, buf, 256) < 256) {
        fprintf(stderr, "fsWrite() wrote fewer than 256\n");
    }

    printf("fsClose(ff): %d\n", fsClose(ff));

    ff = fsOpen(fname, 0);
    if(ff < 0) {
        perror("fsOpen(read)"); exit(1);
    }

    if (fsRemove(fname) != 0) {
        perror("tried to fsRemove");
        printf("tried to fsRemove\n");
    } else {
        printf("\tSuccessfully removed %s\n", fname);
    }
    fd = fsOpenDir(dirname);
    fdent = fsReadDir(fd);
    if (fdent != NULL) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    } else {
        printf("\tReached end of folder \n");
    }
    fdent = fsReadDir(fd);
    if (fdent != NULL) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    } else {
        printf("\tReached end of folder \n");
    }
    fdent = fsReadDir(fd);
    if (fdent != NULL) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    } else {
        printf("\tReached end of folder \n");
    }
    fdent = fsReadDir(fd);
    if (fdent != NULL) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    } else {
        printf("\tReached end of folder \n");
    }

    char rdBuf[256];
    if(fsRead(ff, rdBuf, 256) < 256) {
        fprintf(stderr, "fsRead() read fewer than 256: %s\n", rdBuf);
    } else {
        printf("Successfully read: %s\n", rdBuf);
    }


    // printf("fsWrite(ff, buf, 256): %d\n", fsWrite(ff, buf, 256));
    
    printf("fsClose(ff): %d\n", fsClose(ff));
    // if(fsClose(ff) < 0) {
    //     perror("fsClose"); exit(1);
    // }

    printf("Reached the end without errors\n");
}

void testMounting(char* ip, int port, char* dirname) {
    printf("Test for mounting\n");
    printf("\tdirname: %s\n", dirname);
    printf("\tfsMount(): %d\n", fsMount(ip, port, dirname));
}

void testUnmounting(char* path) {
    printf("Test for unmounting\n");
    printf("\tfsUnmount(): %d\n", fsUnmount(path));
}

void testRemove(int argc, char *argv[]) {
    char *dirname = NULL;

    if(argc > 3) 
        dirname = argv[3];
    else {
        dirname = (char *)calloc(strlen(".")+1, sizeof(char));
        strcpy(dirname, ".");
    }
    printf("dirname: %s\n", dirname);
    printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
    // printf("fsUnmount(): %d\n", fsUnmount(dirname));
    FSDIR *fd = fsOpenDir(dirname);
    if(fd == NULL) {
        perror("fsOpenDir"); exit(1);
    } else {
        printf("no errors printed so far\n");
    }

    char fname[256];
    sprintf(fname, "%s", "asdf");

    int ff = fsOpen(fname, 1);
    if(ff < 0) {
        perror("fsOpen(write)"); exit(1);
    }

    if (fsRemove(fname) != 0) {
        perror("tried to fsRemove");
        printf("tried to fsRemove\n");
    } else {
        printf("\tSuccessfully removed %s\n", fname);
    }
}

void testOpen(int argc, char *argv[]) {
    char *dirname = NULL;

    if(argc > 3) 
        dirname = argv[3];
    else {
        dirname = (char *)calloc(strlen(".")+1, sizeof(char));
        strcpy(dirname, ".");
    }
    printf("dirname: %s\n", dirname);
    printf("fsMount(): %d\n", fsMount(argv[1], atoi(argv[2]), dirname));
    // printf("fsUnmount(): %d\n", fsUnmount(dirname));
    FSDIR *fd = fsOpenDir(dirname);
    if(fd == NULL) {
        perror("fsOpenDir"); exit(1);
    } else {
        printf("no errors printed so far\n");
    }

    char fname[256];
    sprintf(fname, "%s", "zxcv");

    int ff = fsOpen(fname, 1);
    if(ff < 0) {
        perror("fsOpen(write)"); exit(1);
    }

    ff = fsOpen(fname, 1);
    if(ff < 0) {
        perror("fsOpen(write)"); exit(1);
    }
}

int main(int argc, char *argv[]) {
    
    int option;
    char ip[16];
    int port;
    char path[80];
    char line[MAX_LINE_SIZE];
    size_t ln;

    int keep_running = 1;
    int requireServerInfo = 1;

    if (argc == 3) {
        strcpy(ip, argv[1]);
        port = atoi(argv[2]);
        printf("Running tests with IP: %s, port: %d\n", ip, port);
        requireServerInfo = 0;
    }

    while (keep_running) {
            
        printf("\nTest cases, yay\n");
        printf("#\tTestName\t\t(Test args)\n");
        printf("1\ttestMounting\t(ip, port#, dirname)\n");
        printf("2\ttestUnmounting\t(dirname)\n");
        printf("99\tPrintMountedServers\n");
        printf("0\tEnd Testing Situation\n");

        fscanf(stdin, "%d", &option);

        switch (option) {
            case 1:
                if (requireServerInfo) {
                    printf("Enter server address:");
                    fscanf(stdin, "%s", ip);
                    printf("\tIP received: %s\nEnter port number: ", ip);
                    fscanf(stdin, "%d", &port);
                    printf("\tPort number: %d\n", port);
                }
                
                printf("Enter folder to mount: ");
                fscanf(stdin, "%s", path);
                printf("Calling testMounting(ip=%s,port=%d,dirname=%s)\n\n", ip, port, path);
                testMounting(ip,port,path);
                break;
            case 2:
                printf("Enter folder to unmount:");
                fscanf(stdin, "%s", path);
                printf("Calling testUnmounting(dirname=%s)\n", path);
                testUnmounting(path);
                break;
            case 99:
                printRemoteServers();
                break;
            default:
                keep_running = 0;
                printf("Ending test situation\n");
                break;
        }
    }
    return 0;
}

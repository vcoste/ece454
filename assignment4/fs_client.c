#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

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

int main(int argc, char *argv[]) {
    

    test1(argc, argv);

    return 0;

    // int ff = open("/dev/urandom", 0);
    // if(ff < 0) {
    // perror("open(/dev/urandom)"); exit(1);
    // }
    // else printf("open(): %d\n", ff);

    // char fname[256];
    // sprintf(fname, "%s/", dirname);
    // if(read(ff, (void *)(fname+strlen(dirname)+1), 10) < 0) {
    // perror("read(/dev/urandom)"); exit(1);
    // }

    // int i;
    // for(i = 0; i < 10; i++) {
    // //printf("%d\n", ((unsigned char)(fname[i]))%26);
    // fname[i+strlen(dirname)+1] = ((unsigned char)(fname[i+strlen(dirname)+1]))%26 + 'a';
    // }
    // fname[10+strlen(dirname)+1] = (char)0;
    // printf("Filename to write: %s\n", (char *)fname);

    // char buf[256];
    // if(read(ff, (void *)buf, 256) < 0) {
    // perror("fsRead(2)"); exit(1);
    // }

    // printBuf(buf, 256);

    // printf("close(): %d\n", close(ff));

    // ff = fsOpen(fname, 1);
    // if(ff < 0) {
    // perror("fsOpen(write)"); exit(1);
    // }

    // if(fsWrite(ff, buf, 256) < 256) {
    // fprintf(stderr, "fsWrite() wrote fewer than 256\n");
    // }

    // if(fsClose(ff) < 0) {
    // perror("fsClose"); exit(1);
    // }

    // printf("fsRemove(%s): %d\n", fname, fsRemove(fname));

    // return 0;
}

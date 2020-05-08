#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "includes/binode.h"
#include "includes/sortedTree.h"
#include "includes/av.h"

#define NewType(TYPE, NAME) TYPE *NAME = malloc(sizeof(TYPE));

#define STDERR_WRITE(STRING) write(2, STRING, strlen(STRING))

#define NewTypeValue(TYPE, NAME, VALUE) TYPE *NAME = malloc(sizeof(TYPE));\
*NAME=VALUE;\

#define HASH_SIZE 32

#define BUFFER_SIZE_TINY 64
#define BUFFER_SIZE_SMALL 128
#define BUFFER_SIZE_MEDIUM 256
#define BUFFER_SIZE_BIG 512

// Globals

av *AV;

// Enums

enum ScanType{
    SCANTYPE_FILE,
    SCANTYPE_DIRECTORY,
} typedef ScanType;

char sortedTree_tree_funcCompare(void *left, void *right){
    int *nLeft = (int *)left;
    int *nRight = (int *)right;

    return *nLeft > *nRight ? -1 : *nRight > *nLeft ? 1 : 0;
}

void init(){
    // if no signatures file, exit
    if(access("./sigs", F_OK) == -1){
        puts("Missing signatures file, run setup.sh");
        exit(1);
    }

    AV = (av *)malloc(sizeof(av));
    av_Init(AV, "./sigs");

    if(av_LoadSignatures(AV) > 0)
        fprintf(stderr, "[+] %d Signatures loaded\n", AV->hashTree->count);
    else{
        puts("LoadSignatures: Unable to load signatures");
        exit(1);
    }
    if(AV == NULL){
        puts("av: Unable to initialize antivirus");
        exit(1);
    }
}

void show_help(){
    puts("Usage: PinguAV [Options] {file or directory}");
    puts("Scan options:");
    puts("\t-f: scan file instead of directory");
    puts("MISC:");
    puts("\t--help: this screen");
}

void prompt_delete(char *path){
    printf("Deleting the file %s might resolve the problem\nDo you wish to delete it now?\n", path);
    char choise;
    choise = getc(stdin);
    getc(stdin);
    while (choise != 'y' && choise != 'n' &&
    choise != 'Y' &&  choise != 'N')
    {
        puts("Enter [y/n]");
        choise = getc(stdin);
        getc(stdin);
    }
    if(choise == 'y' || choise == 'Y'){
        if(remove(path)){
            puts("Malicious file removed");
        }
        else{
            printf("can't remove %s\n", path);
        }
    }
}

void start_scan(char *path, ScanType type, int fileIndex, int argc){
    switch (type)
    {
        case SCANTYPE_DIRECTORY:
            av_SearchViruses(AV, path);
            printf("Scan complete.\nFound %d Threats!\n", AV->threatsFound);
            break;
        
        default:
            // if no file provided
            if(argc -1 == fileIndex){
                puts("No file provided");
                exit(1);
            }
            else{
                char isVirus = av_CheckFile(AV->hashTree, path);
                if(isVirus == 1){
                    printf("%s is malicious!\n", path);
                    prompt_delete(path);
                }
                else{
                    printf("%s is OK\n", path);
                }
            }
            break;
    }
}

int main(int argc, char *argv[]){
    ScanType type = SCANTYPE_DIRECTORY;
    int fileIndex = -1;
    for(int i = 0; i<argc && fileIndex == -1; i++){
        if(strcmp(argv[i], "-f") == 0){
            type = SCANTYPE_FILE;
            fileIndex = i;
        }
    }

    if((argc == 1) || (argc > 1 && strcmp(argv[1], "--help") == 0) || (type == SCANTYPE_FILE && argc < 3)){
        show_help();
        exit(0);
    }

    char *path;
    if(type == SCANTYPE_DIRECTORY){
        path = argv[1];
    }
    else if(type == SCANTYPE_FILE){
        path = argv[2];
    }
    else{
        puts("Scan: unknown scan type");
        exit(1);
    }
    if(access(path, F_OK) == -1){
        printf("IO: cannot open: %s\n", path);
        exit(1);
    }

    init();

    start_scan(path, type, fileIndex, argc);

    // TODO: Fork - search

    return 0;
}
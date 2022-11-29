#include <sys/stat.h>
#include "shellCode.h"
#include "cmdParser.c"

int input_check(char *input) {
    if (strcmp(input, "DIR") == 0 || strcmp(input, "DIR\n") == 0) caseDir();

    char delim1[] = " ";

    char *copy = malloc(strlen(input) + 1);
    strcpy(copy, input);
    char *ptr = strtok(copy, delim1);
    if (strcmp(ptr, "COPY") == 0) {
        char *src = strtok(NULL, delim1);
        char *dest = strtok(NULL, delim1);
        dest[strcspn(dest, "\n")] = 0;
        caseCopy(src, dest);
        return 0;
    }

    //adapted from: https://jameshfisher.com/2017/02/06/how-do-i-use-fork-in-c/
    pid_t pid = fork();
    if (pid == 0) {
        printf("I'm the child process.\n");
        char **argv = inputParser(input);
        char **env = NULL;
        if (execve(argv[0], argv, env) == -1) {
            perror("error");
        }
    } else {
        printf("I'm the parent process; the child got pid %d.\n", pid);
    }


    return 0;
}

//Question number 1
void caseDir() {
    //Setting the directory and the struct of the entries
    DIR *directory;
    struct dirent *entry;
    //Open the directory
    directory = opendir(".");
    if (directory == NULL) {
        puts("Cannot open the directory");
        exit(1);
    } else {
        puts("Directory is opened!\n");
        //Read the names of the entries in the directory
        entry = readdir(directory);
        while (entry) {
            if (entry->d_name[0] != '.') printf("%s\n", entry->d_name);
            entry = readdir(directory);
        }
        //Close the directory
        closedir(directory);
    }
}

//Question number 2
void caseCopy(char *src, char *dest) {
    char *fullPath1 = malloc(PATH_MAX * sizeof(char));// We will allocate memory to the full paths of both files
    char *fullPath2 = malloc(PATH_MAX * sizeof(char));
    realpath(src,
             fullPath1);// if the path is not a full path (i.e. not start with '\' than we will create the full path.

    realpath(dest, fullPath2);

    struct stat sb;  // Find file size in bytes
    stat(fullPath1, &sb);

    int bufSize = sb.st_size;
    char buf[bufSize];
    int destFile, scrFile;
    // output file opened or created
    if ((destFile = open(fullPath2, O_CREAT | O_APPEND | O_RDWR, 0666)) == -1) {
        perror("open");
    }
    // let us open the input file
    scrFile = open(fullPath1, O_RDONLY);
    if (scrFile > 0) { // there are things to read from the input
        read(scrFile, buf, bufSize);
        write(destFile, buf, bufSize);
        close(scrFile);
    }

    close(destFile);
    close(scrFile);

    free(fullPath1); // Free the memory
    free(fullPath2);
}
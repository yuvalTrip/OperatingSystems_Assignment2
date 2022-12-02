#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>

int input_check(char *input);

void caseDir();

void caseCopy(char *src, char *dest);


// the below code for pipes adapted from https://github.com/csabagabor/Basic-Shell-implementation-in-C/blob/master/shell.c

/*
removes the newline and space character from the end and start of a char*
*/
void removeWhiteSpace(char *buf) {
    if (buf[strlen(buf) - 1] == ' ' || buf[strlen(buf) - 1] == '\n')
        buf[strlen(buf) - 1] = '\0';
    if (buf[0] == ' ' || buf[0] == '\n') memmove(buf, buf + 1, strlen(buf));
}

/*
tokenizes char* buf using the delimiter c, and returns the array of strings in param
and the size of the array in pointer nr
*/
void tokenize_buffer(char **param, int *nr, char *buf, const char *c) {
    char *token;
    token = strtok(buf, c);
    int pc = -1;
    while (token) {
        param[++pc] = malloc(sizeof(token) + 1);
        strcpy(param[pc], token);
        removeWhiteSpace(param[pc]);
        token = strtok(NULL, c);
    }
    param[++pc] = NULL;
    *nr = pc;
}

/*
loads and executes a series of external commands that are piped together
*/
void executePiped(char **buf, int nr) {//can support up to 10 piped commands
    if (nr > 10) return;

    int fd[10][2], i, pc;
    char *argv[100];

    for (i = 0; i < nr; i++) {
        tokenize_buffer(argv, &pc, buf[i], " ");
        if (i != nr - 1) {
            if (pipe(fd[i]) < 0) {
                perror("pipe creating was not successfull\n");
                return;
            }
        }
        if (fork() == 0) {//child1
            if (i != nr - 1) {
                dup2(fd[i][1], 1);
                close(fd[i][0]);
                close(fd[i][1]);
            }

            if (i != 0) {
                dup2(fd[i - 1][0], 0);
                close(fd[i - 1][1]);
                close(fd[i - 1][0]);
            }
            execvp(argv[0], argv);
            perror("invalid input ");
            exit(1);//in case exec is not successfull, exit
        }
        //parent
        if (i != 0) {//second process
            close(fd[i - 1][0]);
            close(fd[i - 1][1]);
        }
        wait(NULL);
    }
}

/**
 * Function count how many spaces in gen string
 * @param str
 * @return
 */

int howManySpaces(char *str) {
    int space = 0;
    int i = 0;
    while (i <= str[i]) {
        if (str[i] == ' ') {
            space++;
        }
        i++;
    }
    return space;
}

/**
 * Function Create array of strings fro user input
 * Example :
parsing the string: "cat file.c > output.txt &"
results:arguments = {"cat","file.c",">","output.txt","&"},
 */
char **inputParser(char *string) {
    int space_num = howManySpaces(string);
    string[strcspn(string, "\n")] = 0;
    char **argv;
    argv = malloc(space_num * sizeof(char *));
    int i = 0;
    argv[i] = strtok(string, " ");//separate string by spaces
    while (argv[i] != NULL) {
        argv[++i] = strtok(NULL, " ");
    }
    return argv;
}

int input_check(char *input) {
    if (strcmp(input, "DIR") == 0 || strcmp(input, "DIR\n") == 0) caseDir();
    else {
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
        } else {

            if (strchr(input, '|') == NULL) { // The string doesn't contain a pipe
                //adapted from: https://jameshfisher.com/2017/02/06/how-do-i-use-fork-in-c/
                pid_t pid = fork();//create 2 process
                if (pid == 0) {//child ID will always be 0.
                    //I'm the child process;
                    char **argv = inputParser(input);

                    // according to https://www.youtube.com/watch?v=iq7puCxsgHQ
                    char *path = "/bin/";
                    char *fullPath = (char *) malloc(strlen(path) + sizeof(argv[0]) + 1);
                    strcpy(fullPath, path);
                    strcat(fullPath, argv[0]);

                    char **env = {NULL};
                    if (execve(fullPath, argv, env) ==
                        -1) //execve will always override the process it run on, in this case it will override the child process
                    {
                        perror("error");
                    }
                } else {
                    // I'm the parent process
                    wait(NULL);
                }
            } else { // contains pipe
                char *buffer[2048];
                int nr = 0;
                tokenize_buffer(buffer,&nr,input,"|");
                executePiped(buffer,nr);
            }
        }
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

int main(int argc, char *argv[]) {
    char *input;
    size_t size = 256;

    while (1) {
        input = (char *) malloc(size);
        if (input == NULL) { // malloc unsuccessful
            exit(1);
        }
        //Get an input from the user
        read(STDIN_FILENO, input, 256);

        //Execute the command
        int code = input_check(input);

        //Exit if the code is 1
        if (code == 1) {
            free(input);
            exit(0);
        }
    }
}
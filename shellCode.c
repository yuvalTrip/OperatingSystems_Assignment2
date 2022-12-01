#include <sys/stat.h>
#include "shellCode.h"
#include "cmdParser.c"
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include <sys/wait.h>

char **deleteAllPipes(char **pString);

void removeChar(char *str, const char charToRemmove) {
    int i, j;
    int len = strlen(str);
    for (i = 0; i < len; i++) {
        if (str[i] == charToRemmove) {
            for (j = i; j < len; j++) {
                str[j] = str[j + 1];
            }
            len--;
            i--;
        }
    }
}

//typedef struct cmdLine cmdLine;
char **allCommands(char *input) {//function create array of all comands from user input
    char **commands = malloc(strlen(input) + 1);
    //if (input[0]=='ls')
    // Find first occurrence " " in input (because what will come before the first space is command
    char *p = strstr(input, " ");
    *p = '\0';
    commands[0] = input;
//    if (strcmp(input[0], "ls") == 0) {
//        commands[0] = input[0];
//    }
//    int i = strlen(input);//we will start to search pipes in the input AFTER the first command
    int i = 1;//we will start to search pipes in the input AFTER the first command

    while (i <= input[i]) {
        if (input[i] == '|') {
            commands[i] = &input[i + 1];
        }
        i++;

    }
    return commands;
}

int countPipes(char *input) {//function count how many pipes in user input
    int pipesNum = 0;
    int i = 0;
    while (input[i] != '\0') {
        if (input[i] == '|') {
            pipesNum++;
        }
        i++;
    }
    return pipesNum;
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
                }
            } else { //string contains pipe

                int numPipes = countPipes(input);
                //seperate by pipes
                char **SeperationByPipes;
                input[strcspn(input, "\n")] = 0;//delete the /n
                SeperationByPipes = malloc(strlen(input) * sizeof(char *));
                int index = 0;
                //create deep copy to input
                char *input3;
                input3 = (char *) malloc(strlen(input) * sizeof(char *));
                strcpy(input3, input);
                //separate string by pipes
                SeperationByPipes[index] = strtok(input3, "|");
                while (SeperationByPipes[index] != NULL) {
                    SeperationByPipes[++index] = strtok(NULL, "|");
                }//now our array look like : [ "cat file1.txt" , "sort"]
                char **FirstSeperationBySpaces = inputParser(SeperationByPipes[0]);
                if (!fork()) {//child 1
                    int file = open("file1.txt", O_WRONLY | O_CREAT, 0777);
                    dup2(file, STDOUT_FILENO);
                    close(file);
                    char *path = "/bin/";
                    char *fullPath = (char *) malloc(strlen(path) + sizeof(FirstSeperationBySpaces[0]) + 1);
                    strcpy(fullPath, path);
                    strcat(fullPath, FirstSeperationBySpaces[0]);

                    char **env = {NULL};
                    if (execve(fullPath, FirstSeperationBySpaces, env) ==
                        -1) //execve will always override the process it run on, in this case it will override the child process
                    {
                        perror("error");
                    }
                } else {
                    wait(NULL);
                }
                // seperate by space the first element i.e: ["cat","file1.txt"]
                while (SeperationByPipes != NULL) {
                    FirstSeperationBySpaces = inputParser(SeperationByPipes[0]);
                    if (!fork()) {//child 1
                        int file = open("file1.txt", O_WRONLY | O_CREAT, 0777);
                        dup2(file, STDOUT_FILENO);
                        close(file);
                        char *path = "/bin/";
                        char *fullPath = (char *) malloc(strlen(path) + sizeof(FirstSeperationBySpaces[0]) + 1);
                        strcpy(fullPath, path);
                        strcat(fullPath, FirstSeperationBySpaces[0]);

                        char **env = {NULL};
                        if (execve(fullPath, FirstSeperationBySpaces, env) ==
                            -1) //execve will always override the process it run on, in this case it will override the child process
                        {
                            perror("error");
                        }
                    } else {
                        wait(NULL);
                    }
                    struct stat sb;  // Find file size in bytes
                    stat(secondArgFilePath, &sb);

                    int bufSize = sb.st_size;
                    char buf[bufSize];
                    int scrFile = open(secondArgFilePath, O_RDONLY);

                    if (scrFile > 0) { // there are things to read from the input
                        read(scrFile, buf, bufSize);
                        close(scrFile);
                        char **args = inputParser(buf);
                        char **fullArgs = malloc(sizeof(args) + sizeof(FirstSeperationBySpaces[0]));
                        fullArgs[0] = firstArgProgram;
                        int i = 1;
                        while (args[i - 1] != NULL) {
                            fullArgs[i] = args[i - 1];
                            i++;
                        }
                    }
                    SeperationByPipes++;
                }

                //find the length of the array
                //int argc = i + 1; // <--


                // now we will start piping by using execve








                //char *argument_list = {SeperationBySpaces[0], SeperationBySpaces[1], NULL};
                //char* argument_list1= (char *) SeperationBySpaces;

                //SeperationBySpaces=deleteAllPipes(SeperationBySpaces);
                //void removeChar(char * str, char charToRemmove){

                int k;
                int separationLen = argc - numPipes; //this is the array len
                //SeperationBySpaces[separationLen]=NULL;

                for (k = 0; k < argc - 1; k++) {
                    int pd[2];
                    pipe(pd);

                    if (!fork()) {//child 1
                        dup2(pd[1], 1); // remap output back to parent
                        char *path = "/bin/";
                        char *fullPath = (char *) malloc(strlen(path) + sizeof(FirstSeperationBySpaces[0]) + 1);
                        strcpy(fullPath, path);
                        strcat(fullPath, FirstSeperationBySpaces[0]);

                        char **env = {NULL};
                        if (execve(fullPath, FirstSeperationBySpaces, env) ==
                            -1) //execve will always override the process it run on, in this case it will override the child process
                        {
                            perror("error");
                        }
//                        execvp(SeperationBySpaces[0], SeperationBySpaces);
//                        perror("exec");
//                        abort();
                    }

                    // remap output from previous child to input
                    dup2(pd[0], 0);
                    close(pd[1]);
                }
                // according to https://www.youtube.com/watch?v=iq7puCxsgHQ
                char *path = "/bin/";
                char *fullPath = (char *) malloc(strlen(path) + sizeof(FirstSeperationBySpaces[0]) + 1);
                strcpy(fullPath, path);
                strcat(fullPath, FirstSeperationBySpaces[0]);

                char **env = {NULL};
                if (execve(fullPath, FirstSeperationBySpaces, env) ==
                    -1) //execve will always override the process it run on, in this case it will override the child process
                {
                    perror("error");
                }
//                execvp(SeperationBySpaces[0], SeperationBySpaces);
//                perror("exec");
//                abort();
            }






//                int fd[2];
//                pid_t childpid;
//                pipe(fd);
//                childpid=fork();
//                if (childpid == -1)
//                {
//                    perror("Error forking...");
//                    exit(1);
//                }
//                if (childpid)   /*parent proces*/   //grep .c
//                {
//                    dup2(fd[0],0);
//                    execlp(SeperationBySpaces[3],SeperationBySpaces[3],NULL);//argv[2],argv[2],argv[3],NULL
//                }
//                if (childpid==0)  //ls
//                {
//                    dup2(fd[1],1);
//                    execlp(argv[1],NULL);
//                }


/*
                int numPipes = countPipes(input);
                if(numPipes>10)
                    exit(0);//if number of pipes is bigger than 10 , break

                int fd[10][2],ind,pc;
                char *argv[100];

                for(ind=0;ind<numPipes;ind++){
                    //seperate
                    tokenize_buffer(argv,&pc,buf[ind]," ");
                    if(ind!=numPipes-1){
                        if(pipe(fd[ind])<0){
                            perror("pipe creating was not successfull\n");
                            exit(0);
                        }
                    }
                    if(fork()==0){//child1
                        if(ind!=numPipes-1){
                            dup2(fd[ind][1],1);
                            close(fd[ind][0]);
                            close(fd[ind][1]);
                        }

                        if(ind!=0){
                            dup2(fd[ind-1][0],0);
                            close(fd[ind-1][1]);
                            close(fd[ind-1][0]);
                        }
                        execvp(argv[0],argv);
                        perror("invalid input ");
                        exit(1);//in case exec is not successfull, exit
                    }
                    //parent
                    if(i!=0){//second process
                        close(fd[ind-1][0]);
                        close(fd[ind-1][1]);
                    }
                    wait(NULL);
                }/*







                /*
                //void runPipedCommands(cmdLine* command, char* userInput) {
                int numPipes = countPipes(input);
                int status;
                int index = 0;
                pid_t pid;

                int pipefds[2 * numPipes];

                for (index = 0; index < (numPipes); index++) {
                    if (pipe(pipefds + index * 2) < 0) {
                        perror("couldn't pipe");
                        exit(EXIT_FAILURE);
                    }
                }
//                char **command = allCommands(argv);
                char **commmand = argv;
                int j = 0;
                while (*command) {
                    pid = fork();
                    if (pid == 0) {

                        //if not last command
                        if (*command!=NULL) {
                            if (dup2(pipefds[j + 1], 1) < 0) {
                                perror("dup2");
                                exit(EXIT_FAILURE);
                            }
                        }

                        //if not first command&& j!= 2*numPipes
                        if (j != 0) {
                            if (dup2(pipefds[j - 2], 0) < 0) {
                                perror(" dup2");///j-2 0 j+1 1
                                exit(EXIT_FAILURE);

                            }
                        }


                        for (i = 0; i < 2 * numPipes; i++) {
                            close(pipefds[i]);
                        }

                        if (execvp(command[0], command) < 0) {
                            printf("ERROR command");
                            exit(EXIT_FAILURE);
                        }
                    } else if (pid < 0) {
                        perror("error");
                        exit(EXIT_FAILURE);
                    }
                    *(command+1) ;
                    j += 2;
                }
                //Parent closes the pipes and wait for children

                for (i = 0; i < 2 * numPipes; i++) {
                    close(pipefds[i]);
                }

                for (i = 0; i < numPipes + 1; i++)
                    wait(&status);
            } */



            /*int fd[2];
            pid_t childpid;
            pipe(fd);
            childpid=fork();
            if (childpid == -1)
            {
                perror("Error forking...");
                exit(1);
            }
            if (childpid)   //parent proces   //grep .c
            {
                //wait(&childpid);        //waits till the child send output to pipe
                //close(fd[1]);
                //close(0);       //stdin closed
                dup2(fd[0],0);
                execlp(argv[2],argv[2],argv[3],NULL);
                //printf("parent case");
            }
            if (childpid==0)  //ls
            {
                //close(fd[0]);   //Closes read side of pipe
                //close(1);       //STDOUT closed
                dup2(fd[1],1);
                execlp(argv[1],NULL);
                //printf("child case");

            } */




            /*
            int j;
            for( j=1; j<argc-1; j++)
            {
                int pd[2];
                int temppipe=pipe(pd);

                if (!fork()) {
                    dup2(pd[1], 1); // remap output back to parent
                    execlp(argv[j], argv[j], NULL);
                    perror("exec");
                    abort();
                }

                // remap output from previous child to input
                dup2(pd[0], 0);
                close(pd[1]);
            }
//                char *path = "/usr/bin/";
//                char *fullPath = (char *) malloc(strlen(path) + sizeof(argv[0]) + 1);
//                strcpy(fullPath, path);
//                strcat(fullPath, argv[0]);
//                execlp(fullPath, argv[j-1], NULL);

//                execlp(argv[j-1], argv[j-1], NULL);

            execlp(argv[0], argv[0], NULL);

            perror("exec");
            abort();*/
        }
    }
    return 0;
}


// dest[strcspn(dest, "\n")] = 0;
// ls|dir   ls | dir | dsfsd | dsgfws - pipe take the output of ls and put it as input in dir
char **inputParserPipe(char *string) {
    //we will delete all spaces
    //string[strcspn(string, " ")] = 0;
    char **argv;
    argv = malloc(strlen(string) * sizeof(char *));
    int i = 0;
    argv[i] = strtok(string, " | ");//separate string by pipes
    while (argv[i] != NULL) {
        argv[++i] = strtok(NULL, " | ");
    }
    //find the length of the array
    int argc = sizeof(argv) / sizeof(argv[1]);

    return argc, argv;
}

// adapted from: https://stackoverflow.com/questions/21914632/implementing-pipe-in-c
void myPipe(int argc, char **argv) {
    int i;

    for (i = 1; i < argc - 1; i++) {
        int pd[2];
        pipe(pd);

        if (!fork()) {
            dup2(pd[1], 1); // remap output back to parent
            execlp(argv[i], argv[i], NULL);
            perror("exec");
            abort();
        }

        // remap output from previous child to input
        dup2(pd[0], 0);
        close(pd[1]);
    }

    execlp(argv[i], argv[i], NULL);
    perror("exec");
    abort();
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

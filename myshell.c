#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int input_check(char *input);

void caseDir();

void caseCopy(char *src, char *dest);

int singleAction(char *input);


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
            if (strcmp(argv[0], "DIR") == 0) caseDir();
            else execvp(argv[0], argv);
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

void hasCloseTriangleBrace(char *ret, char *string) {
    *(ret - 1) = '\0';
    char *firstArgProgram = string;

    char *secondArgFilePath = ret + 2;
    secondArgFilePath[strcspn(secondArgFilePath, "\n")] = 0;

    //adapted from: https://jameshfisher.com/2017/02/06/how-do-i-use-fork-in-c/
    pid_t pid = fork();
    if (pid == 0) {
        // I'm the child process

        char **args = inputParser(string);

        int file = open(secondArgFilePath, O_WRONLY | O_CREAT, 0777);
        dup2(file, STDOUT_FILENO);
        close(file);

        // according to https://www.youtube.com/watch?v=iq7puCxsgHQ
        char *path = "/bin/";
        char *fullPath = (char *) malloc(strlen(path) + sizeof(args[0]) + 1);
        strcpy(fullPath, path);
        strcat(fullPath, args[0]);

        char **env = {NULL};
        if (execve(fullPath, args, env) == -1) {
            perror("error");
        }
    } else {
        wait(NULL);
    }
}

void hasOpenTriangleBrace(char *ret, char *string) {
    *(ret - 1) = '\0';
    char *firstArgProgram = string;

    char *secondArgFilePath = ret + 2;
    secondArgFilePath[strcspn(secondArgFilePath, "\n")] = 0;

    struct stat sb;  // Find file size in bytes
    stat(secondArgFilePath, &sb);

    int bufSize = sb.st_size;
    char buf[bufSize];
    int scrFile = open(secondArgFilePath, O_RDONLY);

    if (scrFile > 0) { // there are things to read from the input
        read(scrFile, buf, bufSize);
        close(scrFile);
        char **args = inputParser(buf);

        char **fullArgs = malloc(sizeof(args) + sizeof(firstArgProgram));
        fullArgs[0] = firstArgProgram;
        int i = 1;
        while (args[i - 1] != NULL) {
            fullArgs[i] = args[i - 1];
            i++;
        }

        pid_t pid = fork();
        if (pid == 0) {
            //I'm the child process
            // according to https://www.youtube.com/watch?v=iq7puCxsgHQ
            char *path = "/bin/";
            char *fullPath = (char *) malloc(strlen(path) + sizeof(firstArgProgram) + 1);
            strcpy(fullPath, path);
            strcat(fullPath, firstArgProgram);

            char **env = {NULL};
            if (execve(fullPath, fullArgs, env) == -1) {
                perror("error");
            }
        } else {
            wait(NULL);
        }
    }
}

/*
    6
    use } and { to send and recv the output to TCP:IP4 socket.

    } IP:PORT will open (client) connection to IP:port

    { PORT will open server port (INADDR_ANY) and output the first client connected to stdin.
    i.e. DIR } 127.0.0.1:3950 will open a connection to localhost and output DIR output
*/

//following 2 function were adapted from: https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
void hasOpenCurlyBrace(char *string) { // receiver
    string = string + 2;
    int port = atoi(string);

    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2048], client_message[2048];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        exit(1);
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't bind to the port\n");
        exit(1);
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0) {
        printf("Error while listening\n");
        exit(1);
    }
    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr *) &client_addr, &client_size);

    if (client_sock < 0) {
        printf("Can't accept\n");
        exit(1);
    }
    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));


    // Receive client's message once (the command):
    if (recv(client_sock, client_message, sizeof(client_message), 0) < 0) {
        printf("Couldn't receive\n");
        exit(1);
    }
    printf("%s", client_message);

    // calculating result
    char **parsedCommand = inputParser(client_message);
    pid_t pid = fork();
    char *tempFile = "temp.txt";
    if (pid == 0) {
        int file = open(tempFile, O_WRONLY | O_CREAT | O_RDONLY, 0777);
        dup2(file, STDOUT_FILENO);
        close(file);


        //I'm the child process
        // according to https://www.youtube.com/watch?v=iq7puCxsgHQ
        char *path = "/bin/";
        char *fullPath = (char *) malloc(strlen(path) + sizeof(parsedCommand[0]) + 1);
        strcpy(fullPath, path);
        strcat(fullPath, parsedCommand[0]);

        char **env = {NULL};
        if (execve(fullPath, parsedCommand, env) == -1) {
            perror("error");
        }
    } else {
        wait(NULL);
    }

    char messageToSend[2048];
    int scrFile;
    scrFile = open(tempFile, O_RDONLY);
    if (scrFile > 0) {
        read(scrFile, messageToSend, 2048);
        close(scrFile);
        remove(tempFile);
    }

    if (send(client_sock, messageToSend, strlen(messageToSend), 0) < 0) {
        printf("Can't send\n");
        exit(1);
    }

    // Closing the socket:
    close(client_sock);
    close(socket_desc);
}

void hasCloseCurlyBrace(char *string) {
    char *curlyBrace = strstr(string, "}");
    *(curlyBrace - 1) = '\0';
    char *command = string;

    char *ip = curlyBrace + 2;
    char *portStr = strstr(ip, ":");
    *portStr = '\0';
    portStr += 1;

    int port = atoi(portStr);


    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2048];
    char *client_message = command;

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Unable to create socket\n");
        exit(1);
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Send connection request to server:
    if (connect(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Unable to connect\n");
        exit(1);
    }
    printf("Connected with server successfully\n");

    // Send the message to server:
    if (send(socket_desc, client_message, strlen(client_message), 0) < 0) {
        printf("Unable to send message\n");
        exit(1);
    }

    // Receive the server's response:
    if (recv(socket_desc, server_message, sizeof(server_message), 0) < 0) {
        printf("Error while receiving server's msg\n");
        exit(1);
    }

    printf("Server's response: \n%s\n", server_message);

    // Close the socket:
    close(socket_desc);
}

int singleAction(char *input) {
    if (strcmp(input, "DIR") == 0 || strcmp(input, "DIR\n") == 0) {
        caseDir();
        return 0;
    }
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
        } else if (strchr(input, '|') == NULL && strchr(input, '<') == NULL && strchr(input, '>') == NULL
                   && strchr(input, '{') == NULL && strchr(input, '}') == NULL) {
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
                return 0;
            } else {
                // I'm the parent process
                wait(NULL);
                return 0;
            }
        } else {
            return 1;
        }
    }
}

int input_check(char *input) {
    if (singleAction(input) == 0) return 0;
    else if (strchr(input, '|') != NULL) { // contains pipe
        char *buffer[2048];
        int nr = 0;
        tokenize_buffer(buffer, &nr, input, "|");
        executePiped(buffer, nr);
    } else {
        char *retClose = strstr(input, ">");
        char *retOpen = strstr(input, "<");
        if (retClose || retOpen) {
            if (retClose) {
                hasCloseTriangleBrace(retClose, input);
            } else { //if (retOpen)
                hasOpenTriangleBrace(retOpen, input);
            }
        }

        char *curlyBrace = strstr(input, "}");
        if (input[0] == '{' || curlyBrace) {
            if (curlyBrace) {
                hasCloseCurlyBrace(input);
            } else { //if (string[0] == '{')
                hasOpenCurlyBrace(input);
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
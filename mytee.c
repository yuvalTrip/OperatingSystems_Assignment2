#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

//1. Implement your own version of the tee(1) command.
// Call it mytee (compile mytee.c) read man tee for tee(1) specification you do not have to support any of the options (arguments)
//Function get input from user and file name, the function will open new file in this name and put there the input, also we will print the input
int myTeeFunc(char filename[]) {

    char buffer[1024] = {0};
    int outfd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0664);//open new file
    if (outfd == -1) {
        fprintf(stderr, "%s\n", "Failed to create output file");
        return 2;
    }
    ssize_t nread = 0;
    bool flag = true;
    while ((nread = read(STDIN_FILENO, buffer, 1024)) != nread || (flag))//read from user
    {
        flag = false;
        if (write(STDOUT_FILENO, buffer, nread) != nread)//write to stdout
        {
            fprintf(stderr, "%s\n", "Failed to write to stdout");
            return 3;
        }
        if (write(outfd, buffer, nread) != nread)//write to file
        {
            fprintf(stderr, "%s\n", "Failed to write to file");
            return 4;
        }
    };
}


int main(int argc, char *argv[]) {
    myTeeFunc(argv[1]);
    return 0;
}
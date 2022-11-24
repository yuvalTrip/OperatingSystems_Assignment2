//Implement the nc(1) commmand support only TCP listener/sender.
//All other arguments can be ignored
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void ncFuncListener(char* portNumber)
{
    int port=atoi(portNumber);
}

void ncFuncSender(char* IP, char* Port)
{

}

int main(int argc, char *argv[]) {
    if (strcmp(argv[1], "-lv")==0) {
        ncFuncListener(argv[2]); //nc -lv 1234
    } else {
        ncFuncSender(argv[2], argv[3]); //nc -v 10.0.2.4 1234
    }

    return 0;
}
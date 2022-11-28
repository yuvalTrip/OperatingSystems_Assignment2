//Implement the nc(1) commmand support only TCP listener/sender.
//All other arguments can be ignored
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define SA struct sockaddr
//The using of NC :
//In one console: nc -l 1234  ->NC listening on a specific port for a connection
//In second console: nc 127.0.0.1 1234
//Server
void ncFuncListener(char *portNumber, struct sockaddr_in in,struct sockaddr_in cli)
{
    int port=atoi(portNumber);//convert the string of port from user to int
    //int IP_=atoi(IP);//convert the string of IP from user to int
//Create socket and verification
    int sockfd=socket (AF_INET,SOCK_STREAM,0);
    if (sockfd==-1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }else {
        printf("socket successfully created..\n");
    }
    bzero (&in,sizeof(in));

    //assign IP,PORT
    in.sin_family=AF_INET;
    in.sin_addr.s_addr=htonl(INADDR_ANY);
    in.sin_port=htons(port);

    //binding newly created socket to given IP and verification
    if ((bind(sockfd,(SA*)&in,sizeof(in)))!=0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else{printf("server successfully binded...\n");}

    //now server ready to listen and verification
    if((listen(sockfd,5))!=0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else{printf("server listening...\n");}
    int len=sizeof(cli);

    //accept the data packet from client and verification
    int connfd=accept(sockfd,(SA*)&cli,&len);
    if (connfd<0)
    {
        printf(printf("server accept failed...\n");
        exit(0);
    }
    else{printf("server accept the client...\n");}

    //after finishing, close the socket
    close(sockfd);

}
//Client
void ncFuncSender(char *IP, char *Port, struct sockaddr_in in)
{
    int port=atoi(Port);//convert the string of port from user to int


}

int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr,cli;
    if (strcmp(argv[1], "-lv")==0) {
        ncFuncListener(argv[2], servaddr,cli); //nc -lv 1234
    } else {
        ncFuncSender(argv[2], argv[3], servaddr); //nc -v 10.0.2.4 1234
    }

    return 0;
}
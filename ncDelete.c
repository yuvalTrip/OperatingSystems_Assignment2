//Implement the nc(1) commmand support only TCP listener/sender.
//All other arguments can be ignored
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <resolv.h>
#include <z3.h>
#include <backtrace.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 80
#define SA struct sockaddr


//The using of NC :
//In one console: nc -l 1234  ->NC listening on a specific port for a connection
//In second console: nc 127.0.0.1 1234

void funcToChatClient(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;)
        {
        bzero(buff,sizeof(buff));
        printf("enter the string:");

        read(STDIN_FILENO, buff, 1024);//read from user to buffer

        write(buff,sockfd,sizeof(buff));//write from buffer to socket
        n=0;
        while((buff[n++]=getchar())!='\n')
            ;
        //write(sockfd,buff,sizeof(buff));
        write(buff,sockfd,sizeof(buff));
        bzero(buff,sizeof(buff));
        read(sockfd,buff, sizeof(buff));
        printf("From server: %s",buff);
        if((strncmp(buff,"exit",4))==0)
            {
            printf("Client Exit...\n");
            break;
            }

        }
}

void funcToChatServer(int sockfd)
{
    char buff[MAX];
    int n;
    for (;;)
        {
        bzero(buff,MAX);

        //read the massege from client and copy it in buffer
//        read(sockfd,buff,sizeof(buff));
        //print buffer which contains the client contents
        if (read(sockfd,buff,sizeof(buff)))
        {
            printf("from client: %s\t ",buff);
        }
        //bzero(buff,MAX);
        //n=0;
        //copy server message in the buffer
//        while((buff[n++]=getchar())!='\n')
//            ;
//        //send that buffer to client
//        write(sockfd,buff,sizeof (buff));


//        printf("enter the string:");
//        n=0;
//        while((buff[n++]=getchar())!='\n')
//            ;
//        write(sockfd,buff,sizeof(buff));
//        bzero(buff,sizeof(buff));
//        read(sockfd,buff, sizeof(buff));
//        printf("From server: %s",buff);
//        if((strncmp(buff,"exit",4))==0)
//            {
//            printf("Client Exit...\n");
//            break;
//            }

        }
}
//Server
void ncFuncListener(char *portNumber, struct sockaddr_in in,struct sockaddr_in cli)
{
    char client_message[2000];
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
    //in.sin_addr.s_addr=htonl(INADDR_ANY);
    in.sin_addr.s_addr = inet_addr("0.0.0.0");
    in.sin_port=htons(port);

    //binding newly created socket to given IP and verification
    if ((bind(sockfd,(SA*)&in,sizeof(in)))!=0)
    {
        printf("socket bind failed...\n");
        exit(0);
    }
    else{printf("server successfully binded...\n");}

    //now server ready to listen and verification
    if((listen(sockfd,1))!=0)
    {
        printf("Listen failed...\n");
        exit(0);
    }
    else{printf("server listening...\n");}
    int len=sizeof(cli);

    //accept the data packet from client and verification
    int client_sock=accept(sockfd,(SA*)&cli,&len);
    if (client_sock<0)
    {
        printf("server accept failed...\n");
        exit(0);
    }
    else{printf("server accept the client...\n");}

    //function to write to server
    //funcToChatServer(sockfd);
// Receive client's message:
    if (recv(client_sock, client_message, sizeof(client_message), 0) < 0){
        printf("Couldn't receive\n");
        exit(0);
    }
    printf("Msg from client: %s\n", client_message);

    //after finishing, close the socket
    close(sockfd);

}
//Client
void ncFuncSender(char *IP, char *Port, struct sockaddr_in in,char *argv[])
{
    char client_message[2000];
    int port=atoi(Port);//convert the string of port from user to int
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
    in.sin_addr.s_addr=inet_addr(argv[1]);
    in.sin_port=htons(port);

    //connect the client socket to server socket
    if(connect(sockfd, (SA*)&in,sizeof(in))!=0)
    {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else{printf("connected to the server..\n");}

    //function to write to server
   // funcToChatClient(sockfd);
    // Get input from the user:
    memset(client_message,'\0',sizeof(client_message));

    printf("Enter message: ");
    //gets(client_message);
    read(STDIN_FILENO, client_message, 1024);
    // Send the message to server:
    if(send(sockfd, client_message, strlen(client_message), 0) < 0){
        printf("Unable to send message\n");
        exit(0);
    }
    close(sockfd);


}



int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr,cli;
    //printf("hiiiiiii");
    if (strcmp(argv[1], "-lv")==0) {
        ncFuncListener(argv[2], servaddr,cli); //nc -lv 1234
    } else {
        ncFuncSender(argv[2], argv[3], servaddr,argv); //nc -v 10.0.2.4 1234
    }

    return 0;
}
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

// https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c

int serverListener(char *portNumber) {
    int port=atoi(portNumber);//convert the string of port from user to int
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0) {
        printf("Error while listening\n");
        return -1;
    }
    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr *) &client_addr, &client_size);

    if (client_sock < 0) {
        printf("Can't accept\n");
        return -1;
    }
    printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));


    while (1) {
        // Receive client's message:
        if (recv(client_sock, client_message, sizeof(client_message), 0) < 0) {
            printf("Couldn't receive\n");
            return -1;
        }
//        if (fgets(client_message, sizeof(client_message), stdout) == NULL)
        printf("%s", client_message);
    }

    // Closing the socket:
    close(client_sock);
    close(socket_desc);

    return 0;
}

int clientSender(char * portNumber, char *IpAddr) {
    //char *IP, char *Port
    int port=atoi(portNumber);//convert the string of port from user to int
    //int  port=(int)(portNumber);
    struct sockaddr_in sa;
    char str[INET_ADDRSTRLEN];
    //store the IP address in sa
    inet_pton(AF_INET,IpAddr,&(sa.sin_addr));
    //now get it back and we will use it
    inet_ntop(AF_INET,&(sa.sin_addr),str,INET_ADDRSTRLEN);
    //int ipAddr=atoi(IpAddr);
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[2000], client_message[2000];

    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");

    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(str);

    // Send connection request to server:
    if (connect(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    // Get input from the user:
    printf("Enter message: ");

    while (1) {
        read(STDIN_FILENO, client_message, 256);

        // Send the message to server:
        if (send(socket_desc, client_message, strlen(client_message), 0) < 0) {
            printf("Unable to send message\n");
            return -1;
        }
    }

    // Close the socket:
    close(socket_desc);
    return 0;
}


int main(int argc, char *argv[]) {
    struct sockaddr_in servaddr,cli;
    if (strcmp(argv[1], "-lv")==0) {
        serverListener(argv[2]); //nc -lv 1234
    } else {
        clientSender(argv[3], argv[2]); //nc -v 10.0.2.4 1234
    }

    return 0;
}
/*
 * A simple UDP server.
 * Usage: udpserver <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/** Constants **/
const int BUFFER_SIZE = 1024;

/** Function Prototypes **/
void error(char* msg);

int main(int argc, char** argv) {
    int sockfd, portNumber, optVal, n;
    unsigned int clientLength;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    struct hostent* client;
    char buffer[BUFFER_SIZE];
    char* hostAddr;

    /** Check for Host Name and Port Number **/
    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    portNumber = atoi(argv[1]);

    /** Create Parent Socket **/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        error("Error Opening Socket");
    }

    /** Handy Trick to Prevent ERROR on Binding: Address Already in Use. **/
    optVal = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optVal, sizeof(int));

    /** Build the Server's Internet Address **/
    bzero((char*) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port        = htons((unsigned short)portNumber);
    printf("Server Port Number: %hu (%d)\n", (unsigned short)serverAddr.sin_port, portNumber);

    /** Bind: Associate the Parent Socket with a Port **/
    if(bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        error("ERROR on binding");
    }

    /** Main Loop: Wait for Datagram, Then Send Back */
    clientLength = sizeof(clientAddr);
    while(1) {
        /** Receive UDP Datagram from a Client **/
        bzero(buffer, BUFFER_SIZE);
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLength);
        if(n < 0) {
            error("Error in Receiving Message from the Client (recvfrom).");
        }

        /** Determine Who Sent the Datagram **/
        client = gethostbyaddr((const char*)&clientAddr.sin_addr.s_addr, sizeof(clientAddr.sin_addr.s_addr), AF_INET);
        if(client == NULL) {
            error("ERROR Retrieving Client (gethostbyaddr).");
        }

        hostAddr = inet_ntoa(clientAddr.sin_addr);
        if(hostAddr == NULL) {
            error("ERROR Retrieving Client Address (inet_ntoa).");
        }

        //Check Cache

        //Retrieve Datagram

        printf("Server Received Datagram From: %s (%s)\n",client->h_name, hostAddr);
        //printf("Client Port Number: %d\n",clientAddr.sin_port);
        printf("Server Received %d/%d Bytes: %s\n", (int)strlen(buffer), n, buffer);

        /** Send Input Back to Client **/
        n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&clientAddr, clientLength);
        if(n < 0) {
            error("Error in Sending Message to the Client (sendto).");
        }
    }

    return 0;
}

/** Function Definitions **/

/**
 * Wrapper for perror.
 * 
 * @param msg Error message.
 */
void error(char* msg) {
    perror(msg);
    exit(0);
}
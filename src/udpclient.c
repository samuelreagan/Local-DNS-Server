/**
 * A simple UDP client.
 * Usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/** Constants **/
const int BUFFER_SIZE = 1024;
const int SRC_PORT    = 6010;

/** Function Prototypes **/
void error(char* msg);

int main(int argc, char** argv) {
    int sockfd, portNumber, n;
    unsigned int serverLength;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    struct hostent* server;
    char* hostName;
    char buffer[BUFFER_SIZE];
    char receiveBuffer[BUFFER_SIZE];

    /** Check for Host Name and Port Number **/
    if(argc != 3) {
        fprintf(stderr, "usage: %s <hostname> <port>\n", argv[0]);
        exit(1);
    }

    hostName   = argv[1];
    portNumber = atoi(argv[2]);

    /** Create Socket **/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        error("Error Opening Socket");
    }

    /** Bind Socket **/
    bzero((char*) &clientAddr, sizeof(clientAddr));
    clientAddr.sin_family      = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port        = htons((unsigned short)SRC_PORT);
    printf("Before Bind Success Port Number: %d \n", clientAddr.sin_port);

    if(bind(sockfd, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0) {
        perror("Bind");
        exit(1);
    } else {
        printf("Bind Success, Port Number: %d\n", clientAddr.sin_port);
    }

    /** Retrieve the Server's DNS Entry **/
    server = gethostbyname(hostName);
    if(server == NULL) {
        fprintf(stderr,"ERROR. No Host: %s\n", hostName);
        exit(0);
    }

    /** Build the Server's Internet Address **/
    bzero((char*)&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(portNumber);

    /** Get A Message from the User **/
    bzero(buffer, BUFFER_SIZE);
    printf("Please enter hostname: ");
    fgets(buffer, BUFFER_SIZE, stdin);

    /** Send Message to the Server */
    serverLength = sizeof(serverAddr);
    n = sendto(sockfd, buffer, (int)strlen(buffer), 0, (struct sockaddr*)&serverAddr, serverLength);
    if(n < 0) {
        error("Error Sending Message to the Server (sendto).");
    }


    /** Receive Message from Server **/
    n = recvfrom(sockfd, receiveBuffer, strlen(receiveBuffer), 0, (struct sockaddr*)&serverAddr, &serverLength);
    if(n < 0) {
        error("Error in Receiving Message from the Server (recvfrom).");
    }

    /** Print the Server's Reply **/
    printf("%s", buffer);

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

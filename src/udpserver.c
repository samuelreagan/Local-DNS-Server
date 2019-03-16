/*
 * A simple UDP server.
 * Usage: udpserver <port>
 * 
 * Samuel Reagan
 * March 15, 2019
 * D.Kim
 * CS 4313
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/** Constants **/
const int BUFFER_SIZE = 1024;
const int IP_SIZE     = 40;
const int DNS_SIZE    = 5;
const double TTL = 10.0;

/** Data Structures **/
typedef struct Record {
    char   hostname[BUFFER_SIZE];
    char   ipAddress[IP_SIZE];
    time_t timeCreated;
} Record;

typedef struct Cache {
    int    numRecords;
    Record records[DNS_SIZE];
} Cache;

/** Function Prototypes **/
void   error(char* msg);
char*  trim(char* str, int* length);
Cache  createCache();
int    isCacheEmpty(Cache c);
int    checkCache(Cache* c, char* hostname);
Record createRecord(char* hostname, char* ipAddress);
int    checkTTL(Record r);
int    insertRecord(Cache* c, char* hostname, char* ipAddress);
void   printRecord(Record r);
void   printCache(Cache* c);

int main(int argc, char** argv) {
    int                sockfd, portNumber, optVal, n;
    unsigned int       clientLength;
    struct sockaddr_in serverAddr;          //Server's Address Record
    struct sockaddr_in clientAddr;          //Client's Address Record  
    struct sockaddr_in queryAddr;           //Query's Address Record               
    struct hostent*    client;              //Client DNS Info  
    struct hostent*    queryHost;           //Query DNS Info  
    char               buffer[BUFFER_SIZE]; //Buffer to hold Hostname
    char*              hostAddr;            //Host's (Client's) Address
    char               ipAddress[IP_SIZE];  //Buffer to hold IP Address
    char*              trimmed;             //Buffer to hold trimmed input
    
    /** Check for Host Name and Port Number **/
    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    puts("");

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
    printf("Server Port Number: %hu (%d)\n\n", (unsigned short)serverAddr.sin_port, portNumber);

    /** Bind: Associate the Parent Socket with a Port **/
    if(bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        error("ERROR on binding");
    }

    /** Initialize Cache **/
    Cache DNSCache = createCache();

    /** Main Loop: Wait for Datagram, Then Send Back */
    clientLength = sizeof(clientAddr);
    while(1) {
        bzero(buffer, BUFFER_SIZE);
        memset(ipAddress, '0', IP_SIZE);

        /** Receive UDP Datagram from a Client **/
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientLength);
        if(n < 0) {
            error("Error in Receiving Message from the Client (recvfrom).");
        }

        /** Determine Who Sent the Datagram **/
        client = gethostbyaddr((const char*)&clientAddr.sin_addr.s_addr, sizeof(clientAddr.sin_addr.s_addr), AF_INET);
        if(client == NULL) {
            error("ERROR Retrieving Client (gethostbyaddr).");
        }

        /** Retrieve Client Address **/
        hostAddr = inet_ntoa(clientAddr.sin_addr);
        if(hostAddr == NULL) {
            error("ERROR Retrieving Client Address (inet_ntoa).");
        }

        printf("Server Received Datagram From: %s (%s)\n",client->h_name, hostAddr);
        printf("Server Received %d/%d Bytes: %s\n", (int)strlen(buffer), n, buffer);

        /** Clean Up Input Received from the Client **/
	    int bufferLength = strlen(buffer);
	    trimmed = trim(buffer, &bufferLength);
	    memcpy(buffer, trimmed, bufferLength);
        int retrieveIP = 0; //Determines if Query to DNS Server is Needed

        /** Check if the Hostname is in the Cache **/
        int inCache = checkCache(&DNSCache, buffer);

        /** Hostname is in the Cache **/
        if(inCache != -1) {
            /** Record in Cache is Expired **/
            if(checkTTL(DNSCache.records[inCache]) == 1) {
                printf("Server has %s in cache, but invalid\n", buffer);
                retrieveIP = 1;
            } 
            /** Record in Cache isn't Expired **/
            else {
                printf("Server has %s in cache\n", buffer);
                memcpy(ipAddress, DNSCache.records[inCache].ipAddress, strlen(DNSCache.records[inCache].ipAddress));
                ipAddress[strlen(DNSCache.records[inCache].ipAddress)] = '\0';
            }
        } 
        /** Hostname isn't in the Cache **/
        else {
            printf("Server has no %s in cache\n", buffer);
            retrieveIP = 1;
        }

        /** Query DNS Server **/
        if(retrieveIP == 1) {
            printf("Server requests to the DNS server\n");
            memset(&queryAddr, '0', sizeof(queryAddr));
            queryAddr.sin_addr.s_addr = inet_addr(buffer);
            queryHost = gethostbyname(buffer);

            /** Get IP Address of Hostname **/
            if(queryHost) {
                char* tmpIP = inet_ntoa(*(struct in_addr*)queryHost->h_addr_list[0]);
                memcpy(ipAddress, tmpIP, strlen(tmpIP));
                ipAddress[strlen(tmpIP)] = '\0';
                printf("Server gets IP address (%s)\n", ipAddress);
                
                /** Save Record in Cache **/
                printf("Server saves %s %s in cache\n", buffer, ipAddress);
                /** Replace Expired Record in Cache **/
                if(inCache != -1) {
                    DNSCache.records[inCache] = createRecord(buffer, ipAddress);
                } 
                /** Insert Record into Cache **/
                else {
                    insertRecord(&DNSCache, buffer, ipAddress);
                }
            } 
            /** No IP Address was Found **/
            else {
                memcpy(ipAddress, "None", strlen("None"));
                ipAddress[strlen("None")] = '\0';
                printf("Server didn't find an IP address\n");
            }
        }   

        /** Send Input Back to Client **/
        printf("Server sends %s to the client\n", ipAddress);
        n = sendto(sockfd, ipAddress, strlen(ipAddress), 0, (struct sockaddr*)&clientAddr, clientLength);
        if(n < 0) {
            error("Error in Sending Message to the Client (sendto).");
        }

        /** Prints Contents of the Cache - Uncomment to check contents **/
        //printCache(&DNSCache);
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

/**
 * Trim whitespace for a c-string.
 * 
 * @param str*    C-string to be trimmed.
 * @param length* The pointer to the length of the c-string.
 * 
 * @return The trimmed c-string.
 */
char* trim(char* str, int* length) {
    char* end;

    // Trim Beginning Whitespace
    while(isspace((unsigned char)*str)) {
        str++;
	(*length)--;
    }

    // Trim Ending Whitespace
    if(*str != 0) {
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) {
	    end--;
	    (*length)--;
	}

        // Add Null Terminator, Update Length
   	*(end + 1) = '\0';
    	(*length)++;
    }

    return str;
}

/**
 * Initializes a cache.
 * 
 * @return The initialized cache.
 */
Cache createCache() {
    Cache newCache;
    newCache.numRecords = 0;

    for(int i; i < DNS_SIZE; i++) {
        newCache.records[i] = createRecord("", "");
    }
    
    return newCache;
}

/**
 * Checks if the cache is empty.
 * 
 * @param c The cache.
 * 
 * @return 0 if the cache isn't empty, 1 otherwise.
 */
int isCacheEmpty(Cache c) {
    return c.numRecords == 0;
}

/**
 * Checks for a hostname in the cache.
 * 
 * @param c*         The pointer to the cache.
 * @param hostname*  The hostname c-string.
 * 
 * @return The index of the record or -1 if the cache is empty.
 */
int checkCache(Cache* c, char* hostname) {
    int foundIndex = -1;
    int i = 0;

    while(foundIndex == -1 && i < c->numRecords) {
        if(strcmp(c->records[i].hostname,  hostname) == 0) {
            foundIndex = i;
        }
        i++;
    }

    return foundIndex;
}

/**
 * Creates a DNS record.
 * 
 * @param hostname*   The hostname c-string.
 * @param ipAddress*  The ipAddress c-string.
 * 
 * @return The newly created record.
 */
Record createRecord(char* hostname, char* ipAddress) {
    Record newRecord;

    memcpy(newRecord.hostname, hostname, strlen(hostname));
    newRecord.hostname[strlen(hostname)] = '\0';
    memcpy(newRecord.ipAddress, ipAddress, strlen(ipAddress));
    newRecord.ipAddress[strlen(ipAddress)] = '\0';
    time(&newRecord.timeCreated);

    return newRecord;
}

/**
 * Checks if the record is still valid by seeing if it's been alive longer
 * than the prescribed TTL (Time to Live).
 * 
 * @param r The record to be checked.
 * 
 * @return 0 if the record is valid, 1 otherwise.
 */
int checkTTL(Record r) {
    time_t currentTime;
    time(&currentTime);

    return difftime(currentTime, r.timeCreated) > TTL;
}

/**
 * Inserts a record into the cache. In the cache isn't full, then the record is
 * inserted into the first empty spot. If the cache is full, then the record takes
 * the place of the first expired record. If no records are expired, then the last
 * record in the cache is replaced by the new record.
 * 
 * @param c*          The pointer to the cache.
 * @param hostname*   The hostname c-string.
 * @param ipAddress*  The hostname c-string.
 * 
 * @return 1 since the record is always inserted (This can be modified).
 */
int insertRecord(Cache* c, char* hostname, char* ipAddress) {
    int inserted = 0;
    if(c->numRecords != DNS_SIZE) {
        c->records[c->numRecords] = createRecord(hostname, ipAddress);
        c->numRecords++;
        inserted = 1;
    } else {
        int i = 0;
        while(inserted != 1 && i < c->numRecords) {
            if(checkTTL(c->records[i]) == 1) {
                c->records[i] = createRecord(hostname, ipAddress);
                inserted = 1;
            }
            i++;
        }

        if(inserted != 1) {
            c->records[c->numRecords - 1] = createRecord(hostname, ipAddress);
        }
    }

    return inserted;
}

/**
 * Prints the contents of a record.
 * 
 * @param r The record to be printed.
 */
void printRecord(Record r) {
    printf("Host: %s\n", r.hostname);
    printf("IP: %s\n", r.ipAddress);
    printf("Time Created: %ld\n", r.timeCreated);
}

/**
 * Prints the contents of a cache.
 * 
 * @param c* The pointer to the cache to be printed.
 */
void printCache(Cache* c) {
    puts("\n------------------");
    puts("DNS Cache Contents");
    puts("------------------");
    for(int i = 0; i < c->numRecords; i++) {
        printRecord(c->records[i]);
        puts("");
    }
    puts("------------------");
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
 
int main(int argc, char **argv) {
 
    struct hostent     *host;
    struct sockaddr_in addr;
 
    memset(&addr, '0', sizeof(addr));
 
    //Check That IP Address or Domain Name is Present
    if(argc!=2) {
        printf("Please, Input IP Address or Domain Name\n");
        exit(1);
    }
 
    //Converts the string, in dot notation, to an integer value suitable for use as an Internet address.
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    
    host = gethostbyaddr((char*)&addr.sin_addr, 4, AF_INET);
 
    if(host) {
        printf("DOMAIN NAME : %s \n\n",host->h_name);
    }
    else {
        host = gethostbyname(argv[1]);
        if(host) {
            printf("IP ADDRESS TYPE : %s \n", (host->h_addrtype == AF_INET) ? "IPv4" : "IPv6");
            printf("IP ADDRESS >>>> \n");
         
            for(int i= 0; host->h_addr_list[i]; i++) {
                puts(inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
            }
            fputc('\n',stdout);
        }
        else {
            printf("Get Host Error\n");
        }
    }
 
    return 0;
}

/**
 * Function Reference
 * ------------------
 * Source: https://pubs.opengroup.org/onlinepubs/7908799/xns/arpainet.h.html
 *         https://pubs.opengroup.org/onlinepubs/7908799/xns/netdb.h.html
 *
 * in_addr_t inet_addr(const char *cp)
 * -------------------------
 * Converts the string pointed to by cp, in the Internet standard dot notation,to an integer value suitable
 * for use as an Internet address. <arpa/inet.h>
 *
 * struct hostent *gethostbyaddr(const void *addr, size_t len, int type)
 * -----------------------------------------------------
 * searches the database and finds an entry which matches the address family specified by the type argument
 * and which matches the address pointed to by the addr argument, opening a connection to the database if
 * necessary. The addr argument is a pointer to the binary-format (that is, not null-terminated) address in
 * network byte order, whose length is specified by the len argument. The datatype of the address depends
 * on the address family. For an address of type AF_INET, this is an in_addr structure, defined in
 * <netinet/in.h>. <netdb.h>
 *
 * struct hostent *gethostbyname(const char *name)
 * -----------------------------------------------
 * Searches the database and finds an entry which matches the host name specified by the name argument,
 * opening a connection to the database if necessary. If name is an alias for a valid host name, the
 * function returns information about the host name to which the alias refers, and name is included in the
 * list of aliases returned.
 *
 * char *inet_ntoa(struct in_addr in)
 * ----------------------------
 * Converts the Internet host address specified by in to a string in the Internet standard dot notation.
 * <arpa/inet.h>           
 * 
 * ............................/////////////////////////////////////////////////////////////////////
 */

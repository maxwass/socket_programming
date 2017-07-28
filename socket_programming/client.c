#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> //convert ip address to human readable (and back)

 /*
    typedef struct sockaddr_in {
            sa_family_t    sin_family; //address family: AF_INET
            in_port_t      sin_port;   // port in network byte order
            struct in_addr sin_addr;   // internet address
    } sockaddr_in;
    
    // Internet address.
    struct in_addr {
            uint32_t       s_addr;     // address in network byte order
    };
 */

int main ( int argc, char *argv[] ) {
 
    //parse CLI
    assert(argc == 4 && "Incorrect number of Arguments to client");
    char *server_ip_cli = argv[1]; //server ip address ?what would happen if char a = *arg...
    char *port      = argv[2]; //port number
    char *msg       = argv[3]; //message to be sent
    
    //CLI processing
    struct sockaddr_in server_addr;
    char str[INET_ADDRSTRLEN];

    // store this IP address in sockadd structure:
    inet_pton(AF_INET, server_ip_cli, &(server_addr.sin_addr));
    // now get it back and put into str
    inet_ntop(AF_INET, &(server_addr.sin_addr), str, INET_ADDRSTRLEN);
    assert(!strcmp(server_ip_cli, str) && "ip addr string-> int error");
    printf("CLI Inputs: \n   server_ip: %s, port: %s, msg: %s \n", server_ip_cli, port, msg);

}



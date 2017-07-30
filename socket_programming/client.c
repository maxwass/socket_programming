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
#include <netdb.h>

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
    assert(argc == 4 && "Incorrect number of Arguments to client: server ip, port #, string message");
    char *server_ip_cli = argv[1]; //server ip address ?what would happen if char a = *arg...
    char *port      = argv[2]; //port number
    char *msg       = argv[3]; //message to be sent
    
    //CLI processing
    struct sockaddr_in server_addr;
    char str[INET_ADDRSTRLEN];

    // store this IP address in sockadd structure:
    inet_pton(AF_INET, server_ip_cli, &(server_addr.sin_addr));
    // now get it back and put into str
    inet_ntop(AF_INET, &(server_addr.sin_addr), str, INET_ADDRSTRLEN); //STILL SOME PROBLEMS WITH STR
    //assert(!strcmp(server_ip_cli, str) && "ip addr string-> int error");
    printf("CLI Inputs: \n   server_ip: %s, port: %s, msg: %s \n\n", server_ip_cli, port, msg);

    
    //get address info with getaddrinfo()
    int status;
    struct addrinfo hints, *rp, *res;
    
    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me = HOST MACHINE IP

    if ((status = getaddrinfo(server_ip_cli, port, &hints, &res)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
                exit(1);
    }

    /* loop over all returned results and do inverse lookup */
    printf("Printing hostnames returned from getaddrinfo() in linked list: \n\n");
    int error, sockfd;
    for (rp = res; rp != NULL; rp = rp->ai_next)
    { 
        char hostname[NI_MAXHOST] = "";
        error = getnameinfo(rp->ai_addr, rp->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0); 
        if (error != 0)
        {
            fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
            continue;
        }
        //reads addrinfo structures, looks up addr, checks local files for ip/hostname mappings, prints out
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        int gn = getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf,sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
        if (gn == 0){                                                           
            printf("    host=%s, serv=%s \n", hbuf, sbuf);
        }
        else{
            printf("FAIL HOSTNAME LOOKUP!\n\n\n");
        }

        //if (*hostname != '\0') {printf("    hostname: %s", hostname);}
        //trying to connect to socket
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        int c = connect(sockfd, rp->ai_addr, rp->ai_addrlen);
        
        if( sockfd !=-1 && c !=-1){
            printf("Connection success!! sockfd: %i, connect: %i\n", sockfd, c);
            break;
        }
        else{
            printf("Could not Connect.   sockfd: %i, connect: %i\n", sockfd, c);
        }
 
        printf("\n");
    }


    if (rp == NULL) {               /* No address succeeded */
            fprintf(stderr, "Could not connect to any returned hostname\n");
            exit(EXIT_FAILURE);
            
    }

    //send data
    int n = send(sockfd, msg, strlen(msg), 0);
    printf("\nSend: %i/%i bytes written\n", n ,(int)strlen(msg));
    //freeaddrinfo(res); freeaddrinfo(&hints); freeaddrinfo(rp);
}


#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> //convert ip address to human readable (and back)
#include <netdb.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define NUM_CLIENTS 3
pthread_t client_thread[NUM_CLIENTS];

void* client_handler(void*);
void local_addr_setup(char *port, int *sockfd,  struct addrinfo *rp);


int main ( int argc, char *argv[] ) {
 
    //parse CLI
    assert(argc == 2 && "Incorrect number of Arguments to client: server ip, port #, string message");
    char *port      = argv[1]; //port number
    printf(ANSI_COLOR_RED "CLI Input: \n  port: %s\n\n"ANSI_COLOR_RESET , port);

    //get local network address info
    struct addrinfo rp, servinfo;
    int* sockfd = malloc(sizeof(int));
    
    local_addr_setup(port, sockfd, &rp);
    
    //bind socket to local address (port)
    if(bind(*sockfd, rp.ai_addr, rp.ai_addrlen) < 0) perror("ERROR on binding");

    //listen for up to 10 connections
    listen(*sockfd, NUM_CLIENTS); //listen for up to 10 connnections on this port
    
    
    int new_sockfd[NUM_CLIENTS];   
    for(int clients_taken = 0; clients_taken < NUM_CLIENTS; clients_taken++){
        //begin accepting connections
        struct sockaddr_storage their_addr;
        int addr_size = sizeof(their_addr);
        
        new_sockfd[clients_taken] = accept(*sockfd, (struct sockaddr *)&their_addr, (socklen_t *)&addr_size);
        if(new_sockfd[clients_taken] < 0) perror("Error on accepting connection!");
        else printf(ANSI_COLOR_RED"Successfully Accepted Connection: %i \n"ANSI_COLOR_RED, new_sockfd[clients_taken]);
    
        printf(ANSI_COLOR_RED"CREATING CLIENT %i !!!!\n"ANSI_COLOR_RESET, clients_taken);
        usleep(3000000);
        if(pthread_create(&client_thread[clients_taken], NULL, client_handler, &new_sockfd[clients_taken]) <0){
            perror("Thread creation failure!!");
        }
    }
    

    for(int i=0; i < NUM_CLIENTS; i++) {
        pthread_join(client_thread[i], NULL);
    }

    ///freeaddrinfo(res); freeaddrinfo(&hints); freeaddrinfo(rp);
}


void* client_handler(void * sockfd){
    printf(ANSI_COLOR_RED"In CLIENT_HANDLER \n"ANSI_COLOR_RESET);
    int local_sockfd = *(int*)sockfd;
    printf(ANSI_COLOR_RED"client sockfd: %i \n"ANSI_COLOR_RESET, local_sockfd);
    
    //recieving data
    uint8_t size_buffer = 100;
    char read_buffer[size_buffer];
    int r = recv(local_sockfd, &read_buffer, size_buffer, 0);
    printf(ANSI_COLOR_RED"\nRead: %s \n bytes read %i \n"ANSI_COLOR_RESET,read_buffer, r);
 
    usleep(3000000);

    //send reply
    char reply[100];
    char recv_msg[] = "I recieved ";
    int len1 = strlen(recv_msg);
    int len2 = strlen(read_buffer);
    memcpy(reply, recv_msg, len1);
    memcpy(reply+len1, read_buffer, len2+1); //+1 to copy the null-terminator
    printf(ANSI_COLOR_RED"%s \n"ANSI_COLOR_RESET, reply);
    int s = send(local_sockfd, &reply, strlen(reply), 0);
    printf(ANSI_COLOR_RED"bytes sent in reply: %i/%i \n"ANSI_COLOR_RESET, s, len1+len2);


    return NULL;
}

void local_addr_setup(char *port, int *sockfd, struct addrinfo *toReturn){
    
    //get local address info with getaddrinfo()
    int status;
    struct addrinfo hints, *rp, *servinfo;

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me = HOST MACHINE IP

    if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
                exit(1);
    }

    /* loop over all returned results and do inverse lookup */
    printf(ANSI_COLOR_RED"Printing hostnames returned from getaddrinfo() in linked list: \n\n"ANSI_COLOR_RESET);
    for (rp = servinfo; rp != NULL; rp = rp->ai_next)
    { 
        //reads addrinfo structures, looks up addr, checks local files for ip/hostname mappings, prints out
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        int gn = getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf,sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
        if (gn == 0){ 
            printf(ANSI_COLOR_RED"    host = %s, serv = %s \n"ANSI_COLOR_RESET, hbuf, sbuf);
        }
        else{
            printf(ANSI_COLOR_RED"FAIL HOSTNAME LOOKUP!\n\n\n"ANSI_COLOR_RESET);
        }

        //trying to create socket
        *sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if( *sockfd !=-1 ){
            printf(ANSI_COLOR_RED"Created socket!!, sockfd: %i\n"ANSI_COLOR_RESET, *sockfd);
            break;
        }
        else{
            printf(ANSI_COLOR_RED"Could not create socket!!, sockfd: %i\n"ANSI_COLOR_RESET, *sockfd);
        }
 
        printf("\n");
    }


    if (rp == NULL) {               /* No address succeeded */
            fprintf(stderr, "Could not create socket with any returned hostname\n");
            exit(EXIT_FAILURE);
            
    }
    //copy local rp to the memory pointed to by toReturn -> viewable in main
    memcpy(toReturn, rp, sizeof(struct addrinfo));
}

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

pthread_t client_thread;

void* client_handler(void*);
void local_addr_setup(char *port, int *sockfd,  struct addrinfo *rp);


int main ( int argc, char *argv[] ) {
 
    //parse CLI
    assert(argc == 2 && "Incorrect number of Arguments to client: server ip, port #, string message");
    char *port      = argv[1]; //port number
    printf("CLI Input: \n  port: %s\n\n", port);

    //get local network address info
    struct addrinfo rp, servinfo;
    int* sockfd = malloc(sizeof(int));
    
    local_addr_setup(port, sockfd, &rp);
    
    //bind socket to local address (port)
    if(bind(*sockfd, rp.ai_addr, rp.ai_addrlen) < 0) perror("ERROR on binding");

    //listen for up to 10 connections
    listen(*sockfd, 10); //listen for up to 10 connnections on this port

    //begin accepting connections
    struct sockaddr_storage their_addr;
    int addr_size;
    addr_size = sizeof(their_addr);
    
    int new_sockfd = (int) malloc(sizeof(int));
    new_sockfd = accept(*sockfd, (struct sockaddr *)&their_addr, (socklen_t *)&addr_size);
    if(new_sockfd < 0) perror("Error on accepting connection!");
    else printf("Successfully Accepted Connection: %i \n", new_sockfd);

    if(pthread_create(&client_thread, NULL, client_handler, &new_sockfd) <0){
        perror("Thread creation failure!!");
    }
    printf("    after create thread\n");
    pthread_join(client_thread, NULL);

    ///freeaddrinfo(res); freeaddrinfo(&hints); freeaddrinfo(rp);
}


void* client_handler(void * sockfd){
    printf("In CLIENT_HANDLER \n");
    int local_sockfd = *(int*)sockfd;
    printf("client sockfd: %i \n", local_sockfd);
    
    //recieving data
    uint8_t size_buffer = 100;
    char read_buffer[size_buffer];
    int r = recv(local_sockfd, &read_buffer, size_buffer, 0);
    printf("\nRead: %s \n bytes read %i \n",read_buffer, r);
 
    usleep(3000000);

    //send reply
    char reply[100];
    char recv_msg[] = "I recieved ";
    int len1 = strlen(recv_msg);
    int len2 = strlen(read_buffer);
    memcpy(reply, recv_msg, len1);
    memcpy(reply+len1, read_buffer, len2+1); //+1 to copy the null-terminator
    printf("%s \n", reply);
    int s = send(local_sockfd, &reply, strlen(reply), 0);
    printf("bytes sent in reply: %i/%i \n", s, len1+len2);


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
    printf("Printing hostnames returned from getaddrinfo() in linked list: \n\n");
    for (rp = servinfo; rp != NULL; rp = rp->ai_next)
    { 
        //reads addrinfo structures, looks up addr, checks local files for ip/hostname mappings, prints out
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        int gn = getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf,sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
        if (gn == 0){ 
            printf("    host = %s, serv = %s \n", hbuf, sbuf);
        }
        else{
            printf("FAIL HOSTNAME LOOKUP!\n\n\n");
        }

        //trying to create socket
        *sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if( *sockfd !=-1 ){
            printf("Created socket!!, sockfd: %i\n", *sockfd);
            break;
        }
        else{
            printf("Could not create socket!!, sockfd: %i\n", *sockfd);
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

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

int main ( int argc, char *argv[] ) {
 
    //parse CLI
    assert(argc == 2 && "Incorrect number of Arguments to client: server ip, port #, string message");
    char *port      = argv[1]; //port number
    
    //CLI processing
    struct sockaddr_in server_addr;
    char str[INET_ADDRSTRLEN];

    printf("CLI Input: \n  port: %s\n\n", port);

   
    //get local address info with getaddrinfo()
    int status;
    struct addrinfo hints, *rp;
    struct addrinfo *servinfo;

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
    int error, sockfd;
    for (rp = servinfo; rp != NULL; rp = rp->ai_next)
    { 
        //reads addrinfo structures, looks up addr, checks local files for ip/hostname mappings, prints out
        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        int gn = getnameinfo(rp->ai_addr, rp->ai_addrlen, hbuf, sizeof(hbuf), sbuf,sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);
        if (gn == 0){                                                           
            printf("    host=%s, serv=%s \n", hbuf, sbuf);
        }
        else{
            printf("FAIL HOSTNAME LOOKUP!\n\n\n");
        }

        //trying to create socket
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        
        if( sockfd !=-1 ){
            printf("Created socket!!, sockfd: %i\n", sockfd);
            break;
        }
        else{
            printf("Could not create socket!!, sockfd: %i\n", sockfd);
        }
 
        printf("\n");
    }


    if (rp == NULL) {               /* No address succeeded */
            fprintf(stderr, "Could not create socket with any returned hostname\n");
            exit(EXIT_FAILURE);
            
    }

    //bind socket to local address (port)
    if(bind(sockfd, rp->ai_addr, rp->ai_addrlen)< 0) {perror("ERROR on binding");}

    listen(sockfd, 10); //listen for up to 10 connnections on this port

    struct sockaddr_storage their_addr;
    int addr_size;
    addr_size = sizeof(their_addr);
    
    int new_sockfd;
    new_sockfd = accept(sockfd, (struct sockaddr *)&their_addr, (socklen_t *)&addr_size);
    if(sockfd < 0) perror("Error on accepting connection!");
    else printf("Successfully Accepted Connection: %i \n", new_sockfd);



/*


    char read_buffer[100];
    int r = read(sockfd,read_buffer,100);
    printf("\nRead: %s \n bytes read %i \n",read_buffer, r);
    

    //send data
    char reply[100];
    strcpy( strcat("I recieved: ", read_buffer), reply);
    int n = send(sockfd, reply, strlen(reply), 0);
    printf("\nSend: %i/%i bytes written\n", n ,(int)strlen(reply));
    
*/
    //freeaddrinfo(res); freeaddrinfo(&hints); freeaddrinfo(rp);
}


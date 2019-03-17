/* 
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <errno.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd; //descriptors return from socket and accept system calls
    int portno; //Port Number passed by user, via command line argument
    socklen_t clilen; //Variable to store address length of client device
    
    char buffer[256];
    
    /*sockaddr_in: Structure Containing an Internet Address*/
    struct sockaddr_in serv_addr, cli_addr; //struct variable to store server/client address
    
    /* Integer variable to store return values from system calls. 
    * This variable is evaluated by if statements after each functions are called. 
    * Most system calls returns negative integers, so we'll check whether the return value is negative or not.
    */
    int n;

    if (argc < 2) {
        /* If statement to check if user has provided valid port number by checking argument count */
        /* Entering this statement means user hasn't provided valid port number. 
        * So we'll print error description to stderr pipe using error function. 
        */
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1); //terminates program with positive exit code
    }
    
    /* Create a new socket
    AF_INET: Address Domain is Internet 
    SOCK_STREAM: Socket Type is STREAM Socket */

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //Creates socket to accept input from user. (IPv4, TCP)
    portno = atoi(argv[1]); //All arguments are passed to program as char array, so we should convert it into Integer to use it.

    if (sockfd < 0) //error
    perror("ERROR on opening socket");
    
    /* Fills serv_addr struct with zero. */
    bzero((char *) &serv_addr, sizeof(serv_addr)); //erases the data in the sizeof(serv_addr) bytes of the memory starting at the location pointed to by &serv_addr, by writing zeros (bytes containing '\0') to that area.
    
    /* Declares address type of server socket. */
    serv_addr.sin_family = AF_INET; //The Internet Protocol version 4 (IPv4) address family.
    //for the server the IP address is always the address that the server is running on
    serv_addr.sin_addr.s_addr = INADDR_ANY; //initialized(INADDR_ANY : 0x00000000)

    /* Set port number to value which we have collected earlier. */
    serv_addr.sin_port = htons(portno); //convert from host to network byte order
    
    /* Tries to bind socket to system with given value. 
    * if it fails, print error and terminate program.
    */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Bind the socket to the server address
            perror("ERROR on binding");
    
    if (listen(sockfd, 5) < 0) // Listen for socket connections. Backlog queue (connections to wait) is 5
    perror("ERROR on listen");
    // listen(sock_fd, BACKLOG); /* starts listening from client. */

    clilen = sizeof(cli_addr);
    /*accept function:
    1) Block until a new connection is established
    2) the new socket descriptor will be used for subsequent communication with the newly connected client.
    */
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        perror("ERROR on accept");
    printf("server : got connection.\n");
    
    while(1) {
        bzero(buffer, 256); //fills list buffer with 256 zeros.
        n = read(newsockfd,buffer,255); //Read is a block function. It will read at most 255 bytes
        if (n < 0) perror("ERROR reading from socket");
        printf("Here is the message: %s\n",buffer);
        
        n = write(newsockfd,"I got your message",18); //NOTE: write function returns the number of bytes actually sent out �> this might be less than the number you told it to send
        if (n < 0) perror("ERROR writing to socket");
    }

    close(sockfd);
    close(newsockfd);
    
    return 0; 
}

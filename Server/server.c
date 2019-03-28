/* 
   A simple server in the internet domain using TCP
   Usage:./server port (E.g. ./server 10000 )
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <sys/stat.h>
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <unistd.h> // lseek(), close(), write(), read()
#include <string.h>
#include <errno.h>
#include <time.h> // for getting current time
#include <fcntl.h> // open(), and its argument(O_RDONLY)

// check user os
// #ifdef __linux
//     char* os = "Linux";
// #else
//     char* os = "Other";
// #endif

void error(char *msg); // make error msg
char* weekday(int wday); // return weekday in string
char* month(int mon); // return month in string
int file_size(int fd); // return file size
char* content_type(char* c_type); // return content type
char* write_http_response_msg(char* c_type, char* http_version, char* status_code, char* os, int filesize); // return http response msg

void error(char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    int sockfd, newsockfd; // descriptors return from socket and accept system calls
    int portno; // Port Number passed by user, via command line argument
    socklen_t clilen; // Variable to store address length of client device
    
    char buffer[256];
    
    /* sockaddr_in: Structure Containing an Internet Address */
    struct sockaddr_in serv_addr, cli_addr; // struct variable to store server/client address
    
    /* Integer variable to store return values from system calls. 
    * This variable is evaluated by if statements after each functions are called. 
    * Most system calls returns negative integers, so we'll check whether the return value is negative or not.
    */
    int n;

    char* splited_str[256]; // store http request message
    char delimit[] = " \r\n"; // delimiter of strtok
    char* method, * req_file, * http_version; // method, requested file, http version
    char* c_type; // content-type
    char file_path[256] = "./rsc"; // resources directory
    int fd; // file
    char * f_buff;
    int i, filesize;
    char* status_code; // 200 OK
    char* response; // http response message
	char* os = "Linux";

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

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Creates socket to accept input from user. (IPv4, TCP)
    portno = atoi(argv[1]); // All arguments are passed to program as char array, so we should convert it into Integer to use it.

    if (sockfd < 0) // error
    error("ERROR on opening socket");
    
    /* Fills serv_addr struct with zero. */
    bzero((char *) &serv_addr, sizeof(serv_addr)); // erases the data in the sizeof(serv_addr) bytes of the memory starting at the location pointed to by &serv_addr, by writing zeros (bytes containing '\0') to that area.
    
    /* Declares address type of server socket. */
    serv_addr.sin_family = AF_INET; // The Internet Protocol version 4 (IPv4) address family.
    //f or the server the IP address is always the address that the server is running on
    serv_addr.sin_addr.s_addr = INADDR_ANY; // initialized(INADDR_ANY : 0x00000000)

    /* Set port number to value which we have collected earlier. */
    serv_addr.sin_port = htons(portno); // convert from host to network byte order
    
    /* Tries to bind socket to system with given value. 
    * if it fails, print error and terminate program.
    */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) // Bind the socket to the server address
        error("ERROR on binding");
    
    if (listen(sockfd, 5) < 0) // Listen for socket connections. Backlog queue (connections to wait) is 5
    perror("ERROR on listen");
    // listen(sock_fd, BACKLOG); /* starts listening from client. */

    while (1){
        clilen = sizeof(cli_addr);
        /* accept function:
        1) Block until a new connection is established
        2) the new socket descriptor will be used for subsequent communication with the newly connected client.
        */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        
		if (newsockfd < 0)
            error("ERROR on accept");
        // printf("server : got connection.\n");

        bzero(buffer, 256); // fills list buffer with 256 zeros.
        n = read(newsockfd, buffer, 255); // Read is a block function. It will read at most 255 bytes
        if (n < 0) error("ERROR reading from socket");
        printf("<HTTP Request MSG>\n%s\n\n", buffer);


        i = 0; // initalize
        splited_str[i] = strtok(buffer, delimit); // split string into token( , \n, \r)
        while (splited_str[i] != NULL) {
            i++;
            splited_str[i] = strtok(NULL, delimit); // start to split at the point where cursor stopped
        }

        method = splited_str[0]; // GET
        req_file = splited_str[1]; // /index.html
        strcat(file_path, req_file); // ./rsc/index.html
        http_version = splited_str[2]; // HTTP/1.1

        // open requested file and check whether the file exists
        if (((fd = open(file_path, O_RDONLY)) == -1)) {
            char cc_type[2048];
            memset(cc_type, '\0', 2048); // initialize char by NULL
            strcpy(cc_type, "/a.html"); // temporary file name

            response = write_http_response_msg(cc_type, http_version, "404 NOT FOUND", os, filesize); // http response message

            if (write(newsockfd, response, strlen(response)) < 0) error("ERROR on writing to socket"); // response 404 not found

            error("404 file not found\n"); // error msg
        }


        status_code = "200 OK"; // Standard response for successful HTTP requests
        c_type = req_file; // content-type

        filesize = 0; // initialize
		f_buff = malloc(file_size(fd));
        filesize = file_size(fd); // get file size
        read(fd, f_buff, filesize); // read requested file

        response = write_http_response_msg(c_type, http_version, status_code, os, filesize); // http response message

        if (write(newsockfd, response, strlen(response)) < 0) error("ERROR on writing to socket"); // send http response message
        if (write(newsockfd, f_buff, filesize) < 0) error("ERROR on writing to socket"); // send the requested file

        bzero(file_path, 256); // initialize
        strcpy(file_path, "./rsc"); // init file_path
        // free malloc
        free(response);
        free(f_buff);
        // close file and socket
        close(fd);
        close(newsockfd);
    }

    // close socket
    close(sockfd);
    
    return 0; 
}

// return weekday in string
char* weekday(int wday){
    char* w_day[7] = {"Sun", "Mon", "Tues", "Wed", "Thurs", "Fir", "Sat"};
    return w_day[wday];
}

// return month in string
char* month(int mon){
    char* mon_str[12] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};
    return mon_str[mon];
}

// return file size
int file_size(int fd){
    int filesize = lseek(fd, 0, SEEK_END); // move cursor(file pointer) to the end of file
    lseek(fd, 0, SEEK_SET); // move back to the first of file
    return filesize; // current position of the cursor(file pointer)
}

// return content type of the requested file
char* content_type(char* c_type){
    char* tmp = strtok(c_type, "."); // /index.html
    tmp = strtok(NULL, "\0"); // html
    char file_type[10] = ".";
    strcat(file_type, tmp); // .html
    // compare file_type and <file format> and return mime content-type
	if (!strcmp(file_type, ".html")) return "text/html";
    if (!strcmp(file_type, ".png")) return "image/png";
    if (!strcmp(file_type, ".gif")) return "image/gif";
    if (!strcmp(file_type, ".jpeg")) return "image/jpeg";
    if (!strcmp(file_type, ".pdf")) return "application/pdf";
    if (!strcmp(file_type, ".mp3")) return "audio/mpeg3";
	if (!strcmp(file_type, ".m4a")) return "audio/m4a";
    if (!strcmp(file_type, ".mov")) return "video/quicktime";
	else return "text/plain";
    return NULL;
}

char* write_http_response_msg(char* c_type, char* http_version, char* status_code, char* os, int filesize){
    /* for take current date and time */
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char* response = (char*) malloc(2048); // http response message
    char * temp = content_type(c_type); // get content-type
    int rsp_size; // response msg size

    bzero(response, 2048);
    rsp_size = sprintf(response, "%s %s\r\n", http_version, status_code); // HTTP/1.1 200 OK
    rsp_size += sprintf(response + rsp_size, "Date: %s, %d %s %d %d:%d:%d GMT\r\n", weekday(tm.tm_wday), tm.tm_mday, month(tm.tm_mon), tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec); // current date and time
    rsp_size += sprintf(response + rsp_size, "Server: myServer/1.0 (%s)\r\n", os); // Linux
    rsp_size += sprintf(response + rsp_size, "Content-Length: %d\r\n", filesize);
    rsp_size += sprintf(response + rsp_size, "Connection: Keep-Alive\r\nContent-Type: %s\n\n", temp); // text/html

    printf("<HTTP Response MSG>\n%s\n", response);

    return response;
}

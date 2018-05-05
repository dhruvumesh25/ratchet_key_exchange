/*
** server.c -- a stream socket server demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold
#define MAXDATASIZE 100 // max number of bytes we can get at once 

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct Server
{
    int sockfd, new_fd, numbytes;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char buf[MAXDATASIZE];

    Server(){
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // use my IP

        if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return;
        }

        // loop through all the results and bind to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("server: socket");
                continue;
            }

            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                    sizeof(int)) == -1) {
                perror("setsockopt");
                exit(1);
            }

            if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("server: bind");
                continue;
            }

            break;
        }

        freeaddrinfo(servinfo); // all done with this structure

        if (p == NULL)  {
            fprintf(stderr, "server: failed to bind\n");
            return;
        }

        if (listen(sockfd, BACKLOG) == -1) {
            perror("listen");
            return;
        }

        sa.sa_handler = sigchld_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            return;
        }

        printf("server: waiting for connections...\n");

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
    }

    void sendData(char* inp, int size){
        // printf("%s\n",inp);
        if (send(new_fd, inp, size, 0) == -1)
                perror("send");
    }

    char* receiveData(){
        while ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) <= 0) {
            if(numbytes == -1){
                perror("recv");
                exit(1);
            }
        }

        buf[numbytes] = '\0';

        // printf("Server: received '%s'\n",buf);
        if(strcmp(buf,"0")==0){
            printf("closing connection\n");
        }

        return buf;
    }

    void sendAck(){
        char data[] = "ack";
        sendData(data,4);
    }

    void getAck(){
        char * in = receiveData();
        while(strcmp(in,"ack")!=0){
            printf("error : got %s\n",in);
        }
        // printf("got ack\n");  
    }

    void closeConnection(){
        close(new_fd);
    }
    // char inp[100];
    // while(1) {
    //     scanf("%s",inp);
        
    // }
    // close(new_fd);

};
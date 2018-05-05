/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct Client
{
    int sockfd, numbytes;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char buf[MAXDATASIZE];

    Client(){
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
    }

    int connectToServer(){

        if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through all the results and connect to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("client: socket");
                continue;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("client: connect");
                continue;
            }

            break;
        }

        if (p == NULL) {
            fprintf(stderr, "client: failed to connect\n");
            return 2;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                s, sizeof s);
        printf("client: connecting to %s\n", s);

        freeaddrinfo(servinfo); // all done with this structure
    }

    char* receiveData(){
        while ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) <= 0) {
            if(numbytes == -1){
                perror("recv");
                exit(1);
            }
        }

        buf[numbytes] = '\0';

        // printf("client: received '%s'\n",buf);
        if(strcmp(buf,"0")==0){
            printf("closing connection\n");
        }
        return buf;
    }

    void sendData(char* inp, int size){
        // printf("%s\n",inp);
        if (send(sockfd, inp, size, 0) == -1)
                perror("send");
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
};
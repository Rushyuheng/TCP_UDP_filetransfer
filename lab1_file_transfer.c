//c library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//unix library
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


void errormsg(const char * msg){
    perror(msg);
    perror("program stop");
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
    if(strcmp(argv[1], "tcp") == 0){

        int socketd, newsocketd;               // return value when opening new scoket for checking socket construction status
        char buffer[256];                       // data buffer for reading data
        struct sockaddr_in serv_addr, cli_addr; //socket structure defined in <netinet/in.h>
        socklen_t clilen;                       // client length for accept()
        int portnum,rw_status;

        socketd = socket(AF_INET,SOCK_STREAM,0); // construct socket in IPv4 form with TCP protocol

        bzero((char *) &serv_addr, sizeof(serv_addr)); //clear byte to 0 for intializing serv_addr struct

        portnum = atoi(argv[4]); 
        serv_addr.sin_family = AF_INET; // config IP form as IPv4
        serv_addr.sin_addr.s_addr = inet_addr(argv[3]); //config server IP
        serv_addr.sin_port = htons(portnum); //config server port number

        if(socketd < 0){
            errormsg("fail to open a new socket");
        }

        if(strcmp(argv[2], "recv") == 0){ //server side
            printf("tcp recv\n");
            if (bind(socketd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){// bind socket and server
                errormsg("fail to bind socket and server address");
            } 
            listen(socketd,5); // set server socket to listen mode with max connection queue of 5
            socklen_t clilen = sizeof(cli_addr);
            newsocketd = accept(socketd,(struct sockaddr *) &cli_addr, &clilen); // accept the first connection request in the queue
            if(newsocketd < 0){
                errormsg("fail to accept socket from client");
            }

            //----The code we should revise----
            bzero(buffer,256); // clear buffer
            rw_status = read(newsocketd,buffer,255);
            if (rw_status < 0)
                errormsg("fail to read from socket");
            printf("Here is the message: %s\n",buffer);

            rw_status = write(newsocketd,"I got your message",18);
            if (rw_status < 0)
                errormsg("fail to write to socket");
            //--------

            //close socket and end file transfering
            close(newsocketd);
            close(socketd);
        }
        else if(strcmp(argv[2], "send") == 0){ // client side
            printf("tcp send\n");
            if (connect(socketd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ //try to connect to server socket
                errormsg("fail to connect socket");
            }

            //----The code we should revise----
            printf("Please enter the message:\n");
            bzero(buffer,256);
            fgets(buffer,255,stdin);

            rw_status = write(socketd,buffer,strlen(buffer));
            if (rw_status < 0) 
                errormsg("fail to write to socket");
            bzero(buffer,256);

            rw_status = read(socketd,buffer,255);
            if (rw_status < 0) 
                errormsg("fail to read from socket");
            printf("%s\n",buffer);
            //--------

            close(socketd);//close socket and end file transfering
        }
        else{
            errormsg("invalid mode");
        }

    }
    else if (strcmp(argv[1], "udp") == 0){
        if(strcmp(argv[2], "recv") == 0){
            printf("udp recv\n");
        }
        else if(strcmp(argv[2], "send") == 0){
            printf("udp send\n");
        }
        else{
            errormsg("invalid mode");
        }
    }
    else{
        errormsg("invalid protocol");
    }
    
    return 0;
}
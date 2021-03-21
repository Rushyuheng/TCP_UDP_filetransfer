//c library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//unix library
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


void errormsg(const char * msg){
    perror(msg);
    perror("program stop");
    exit(EXIT_FAILURE);
}

void showprogress(int percent){
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    printf("%d %%",percent);
    printf(" %s\n", asctime(tm));
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

        memset(&serv_addr, 0, sizeof(serv_addr));//clear every byte in struct for intializing serv_addr struct

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
            FILE *fp = fopen("recv.txt", "w");
            if(fp == NULL){
                errormsg("fail to open write file");
            }
            else{
                bzero(buffer,256); // clear buffer perpare to recv data from socket
                int length = 0;
                while(length = recv(newsocketd, buffer, 256, 0)){ // recv data from socket
                        if (length < 0)
                            errormsg("fail to recv data from socket");

                        int write_length = fwrite(buffer, sizeof(char), length, fp);//write buffer to file
                        if(write_length < length)
                            errormsg("fail to write data from recv buffer to file");

                        bzero(buffer, 256);//clear buffer for next loop
                } 
            }
            printf("file receive complete");

            // close socket and file pointer
            fclose(fp);
            close(newsocketd);
            close(socketd);
        }
        else if(strcmp(argv[2], "send") == 0){ // client side
            printf("tcp send\n");
            if (connect(socketd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){ //try to connect to server socket
                errormsg("fail to connect socket");
            }

            FILE *fp = fopen(argv[5],"r"); //open file and creat file pointer
            if(fp == NULL){
                errormsg("fail to open file");
            }
            else{
                bzero(buffer,256); // clear buffer
                int file_block_length = 0;
                int sended_block_length = 0;

                struct stat st; //get file size from system
                stat(argv[5], &st);
                float filesize = st.st_size;
                float filesizeMB = filesize / 1024.0 / 1024.0; // byte to MB

                int progressindex = 1;
                clock_t t1, t2;

                showprogress(0);//print time and progress

                t1 = clock();
                while( (file_block_length = fread(buffer, sizeof(char), 256, fp)) > 0){//read file into buffer
                    if (send(socketd, buffer, file_block_length, 0) < 0)// send file to socket
                        errormsg("fail to send file");
                    
                    sended_block_length += file_block_length;
                    if(sended_block_length / filesize >= progressindex * 0.25){ //print progress evert 25%
                        showprogress((int) progressindex * 0.25);
                        ++progressindex;
                    }
                    bzero(buffer, sizeof(buffer));//clear buffer for next loop
                }
                fclose(fp);
                t2 = clock();
                showprogress(100);
                double elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
                printf(" total transfer time : %f ms",elapsed );
            }

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
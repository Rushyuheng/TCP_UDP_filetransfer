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
#include <errno.h>

#define BUFFER_SIZE 256

void errormsg(const char * msg){
    perror(msg);
    perror("program stop");
    exit(EXIT_FAILURE);
}

void showprogress(int percent){
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    printf("%d %%",percent);
    printf("%s", asctime(tm));
}

int main(int argc, char const *argv[])
{
    int socketd, newsocketd;               // return value when opening new scoket for checking socket construction status
    char buffer[BUFFER_SIZE];                       // data buffer for reading data
    struct sockaddr_in serv_addr, cli_addr; //socket structure defined in <netinet/in.h>
    socklen_t clilen;                       // client length for accept()
    int portnum,rw_status;

    memset(&serv_addr, 0, sizeof(serv_addr));//clear every byte in struct for intializing serv_addr struct

    portnum = atoi(argv[4]); 
    serv_addr.sin_family = AF_INET; // config IP form as IPv4
    serv_addr.sin_addr.s_addr = inet_addr(argv[3]); //config server IP
    serv_addr.sin_port = htons(portnum); //config server port number

    if(strcmp(argv[1], "tcp") == 0){

        socketd = socket(PF_INET,SOCK_STREAM,0); // construct socket in IPv4 form with TCP protocol
        if(socketd < 0)
            errormsg("fail to open a new socket");


        if(strcmp(argv[2], "recv") == 0){ //server side
            printf("tcp recv mode\n");
            if (bind(socketd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)// bind socket and server
                errormsg("fail to bind socket and server address");

            listen(socketd,5); // set server socket to listen mode with max connection queue of 5
            socklen_t clilen = sizeof(cli_addr);
            newsocketd = accept(socketd,(struct sockaddr *) &cli_addr, &clilen); // accept the first connection request in the queue and store client info stuct
            if(newsocketd < 0)
                errormsg("fail to accept socket from client");

            //file transfer
            FILE *fp = fopen("recv.txt", "w");
            if(fp == NULL)
                errormsg("fail to open write file");

            else{
                bzero(buffer,BUFFER_SIZE); // clear buffer perpare to recv data from socket
                int length = 0;
                while(length = recv(newsocketd, buffer, BUFFER_SIZE, 0)){ // recv data from socket
                        if (length < 0)
                            errormsg("fail to recv data from socket");

                        int write_length = fwrite(buffer, sizeof(char), length, fp);//write buffer to file
                        if(write_length < length)
                            errormsg("fail to write data from recv buffer to file");

                        bzero(buffer, BUFFER_SIZE);//clear buffer for next loop
                } 
            }
            printf("file receive complete\n");

            // close socket and file pointer
            fclose(fp);
            close(newsocketd);
            close(socketd);
        }
        else if(strcmp(argv[2], "send") == 0){ // client side
            printf("tcp send mode\n");
            if (connect(socketd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) //try to connect to server socket
                errormsg("fail to connect socket");

            FILE *fp = fopen(argv[5],"r"); //open file to be sent and create file pointer
            if(fp == NULL)
                errormsg("fail to open file");
            
            else{
                bzero(buffer,BUFFER_SIZE); // clear buffer
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
                while( (file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0){//read file into buffer
                    if (send(socketd, buffer, file_block_length, 0) < 0)// send file to socket
                        errormsg("fail to send file");
                    
                    sended_block_length += file_block_length;//accumulate data sent
                    if(sended_block_length / filesize > progressindex * 0.25){ //print progress evert 25%
                        showprogress(progressindex * 25);
                        ++progressindex;
                    }
                    bzero(buffer, sizeof(buffer));//clear buffer for next loop
                }

                t2 = clock();
                showprogress(100);
                double elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
                printf("total transfer time : %f ms\n",elapsed);
                printf("file size : %f MB \n",filesizeMB);
            }
            fclose(fp);
            close(socketd);//close socket and end file transfering
        }
        else{
            errormsg("invalid mode");
        }

    }
    else if (strcmp(argv[1], "udp") == 0){

        socketd = socket(PF_INET,SOCK_DGRAM,0); // construct socket in IPv4 form with UDP protocol
        if(socketd < 0)
            errormsg("fail to open a new socket");

        if(strcmp(argv[2], "recv") == 0){ //server side
            printf("udp recv mode\n");

            if (bind(socketd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)// bind socket and server
                errormsg("fail to bind socket and server address");

            struct timeval timeOut;//set recv timer for socket
            timeOut.tv_sec = 5;// 5 sec timeout
            timeOut.tv_usec = 0;
            if (setsockopt(socketd, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut)) < 0)
                errormsg("fail to set socket recv timeout timer\n");


            socklen_t clilen = sizeof(cli_addr);
            int length = 0;

            FILE *fp = fopen("recv.txt", "w");//prepare to write file
            if(fp == NULL)
                errormsg("fail to open write file");

            while(1){
                length = 0;

                length = recvfrom(socketd,buffer,BUFFER_SIZE, 0,(struct sockaddr *)&cli_addr, &clilen);
                if(length < 0){
                    if(errno == EAGAIN){
                        printf("socket reach time limit : %ld sec ,socket closed\n",timeOut.tv_sec);
                        break;
                    }
                    else
                        errormsg("fail to recv data from socket");
                }
                else if(length > 0){
                    int write_length = fwrite(buffer, sizeof(char), length, fp);//write buffer to file
                    if(write_length < length)
                        errormsg("fail to write data from recv buffer to file");
                    bzero(buffer, BUFFER_SIZE);//clear buffer for next loop
                }
            }
            printf("file receive complete\n");

            // close socket and file pointer
            fclose(fp);
            close(socketd);

        }
        else if(strcmp(argv[2], "send") == 0){
            printf("udp send mode \n");
            FILE *fp = fopen(argv[5],"r"); //open file to be sent and create file pointer
            if(fp == NULL)
                errormsg("fail to open file");

            else{
                bzero(buffer,BUFFER_SIZE); // clear buffer
                int file_block_length = 0;
                int sended_block_length = 0;

                struct stat send_st; //get file size from system
                stat(argv[5], &send_st);
                float send_filesize = send_st.st_size;
                float send_filesizeMB = send_filesize / 1024.0 / 1024.0; // byte to MB

                int progressindex = 1;
                clock_t t1, t2;

                showprogress(0);//print time and progress

                t1 = clock();
                while( (file_block_length = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0){//read file into buffer
                    if (sendto(socketd, buffer,file_block_length, 0,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)// send file to socket
                        errormsg("fail to send file");
                    
                    sended_block_length += file_block_length;//accumulate data sent
                    if(sended_block_length / send_filesize > progressindex * 0.25){ //print progress evert 25%
                        showprogress(progressindex * 25);
                        ++progressindex;
                    }
                    bzero(buffer, sizeof(buffer));//clear buffer for next loop
                }

                t2 = clock();
                showprogress(100);
                double elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
                printf("total transfer time : %f ms\n",elapsed);
                printf("file size : %f MB \n",send_filesizeMB);
                printf("calculating packet loss...\n");
                sleep(6);// wait 6 sec until server close file pointer

                struct stat recv_st; //get file size from system
                stat("recv.txt", &recv_st);
                float recv_filesize = recv_st.st_size;
                printf("recv size %f\n", recv_filesize);
                float packet_loss = (send_filesize - recv_filesize) / send_filesize;
                printf("packet loss : %f %%\n",packet_loss);

                // close socket and file pointer
                fclose(fp);
                close(socketd);
            }


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
# TCP/UDP file transfer
###### tags: `assigment` `junior` `net`

## Intro
This is a practice of socket programming, the goal is to write "one program" that have 4 mode, TCP send, TCP recv, UDP send, UDP recv.  

Command should be the following form
```shell=
./lab1_file_transfer <protocal> send <ip> <port> <send text name>
./lab1_file_transfer <protocal> recv <ip> <port>

ex:
./lab1_file_transfer udp send 127.0.0.1 5566 test_input.txt
./lab1_file_transfer udp recv 127.0.0.1 5566

```
:::warning
note: recv file name is recv.txt in default
:::

## TCP
Fisrt config server struct
```c=
struct sockaddr_in serv_addr

serv_addr.sin_family = AF_INET; // config IP form as IPv4
serv_addr.sin_addr.s_addr = inet_addr("{IP in sting}"); //config server IP
serv_addr.sin_port = htons({portnum}); //config server port number

```
feature function:  
```send()```  
```recv()```
```c=
socketd = socket(...,SOCK_STREAM,...);
```
### server side
1. ```socket()```: construct socket(a open pipe as entrance) 
2. ```bind()```: bind socket and server ip together(so data in socket will be passed to server)
3. ```listen()```: listen connection request from network
4. ```accept()```: accept fisrt connection request from listen queue, newsocket will connect to socket in (1), meanwhile it will store client info struct

### client side
1. ```socket()```:construct socket(a open pipe as entrance)
2. ```connent()```:send connection request to server and wait in queue

now you can use ```send()```and```recv()``` to communicate

### UDP
Fisrt config server struct
```c=
struct sockaddr_in serv_addr

serv_addr.sin_family = AF_INET; // config IP form as IPv4
serv_addr.sin_addr.s_addr = inet_addr("{IP in sting}"); //config server IP
serv_addr.sin_port = htons({portnum}); //config server port number

```
feature function:  
```sendto()```  
```recvfrom()```
```c=
socketd = socket(...,SOCK_DGRAM,...);
```

### server side
1. ```socket()```: construct socket(a open pipe as entrance) 
2. ```bind()```: bind socket and server ip together(so data in socket will be passed to server)

### client side
1. ```socket()```:construct socket(a open pipe as entrance)

now you can use```sendto()```and```recvfrom()``` to communicate!  
however since they are not ```connect()```, you have to specifiy sending destination's IP in```sendto()```, and you can get sender info in```recvfrom()``` 
```c=
recvfrom(...,...,...,...,(struct sockaddr *)&peeraddr, &peerlen); // sender info can be stored in peeraddr struct
```
### when to close UDP socket?
UDP dosen't like TCP, it won't get return 0 from ```recvfrom``` to indicate that data transfer finished, you should set TIMEOUT for UPD socket in order to make it close automatically after socket receive no data for a centain amount of time.

```c=
struct timeval timeOut;//time struct defined in <time.h>
timeOut.tv_sec = 5;// 5 sec timeout
timeOut.tv_usec = 0;

setsockopt(socketd, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut))
```


## supplementary
### PF_INET == AF_INET?
long time ago  
AF = Address Family  
PF = Protocol Family  

However in ```socket.h```, line 204 of Linux kernel 3.2.21 tree.  
```c=
#define PF_INET     AF_INET // its all the same know
```
In old design good coding habit is like:
```c=
...
servaddr.sin_family = AF_INET; //address assignment should us AF
...
socket = socket(PF_INET,...,...); // construct socket should use PF
...
```
### Useful Tips
* ```inet_addr()``` was defined under ```<arpa/inet.h>```

## Reference
[sys/socket.h doc](https://pubs.opengroup.org/onlinepubs/7908799/xns/syssocket.h.html)  
[TCP basic knowledge](https://snsd0805.github.io/jekyll/update/2019/05/27/%E7%AD%86%E8%A8%98-Linux%E7%92%B0%E5%A2%83%E7%94%A8c++%E5%BB%BA%E7%AB%8BSocket%E9%80%A3%E7%B7%9A.html)  
[TCP file transfer in c example](https://www.itread01.com/p/1380962.html)
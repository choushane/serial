#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <linux/tcp.h> 
#include "tcp_states.h"

static int connfd[4] = {-1};
int open_tcp_client(int x,char *ip_addr, unsigned short int port,unsigned short int local_port)
{
    int sockfd = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr,client_addr; 

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
#ifdef DEBUG 
        printf("\n Error : Could not create socket \n");
#endif        
        return -1;
    } 

    if(local_port > 0)
    {
        memset(&client_addr, '0', sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = INADDR_ANY;
        client_addr.sin_port = htons(local_port); 
        bind(sockfd , (struct sockaddr *) &client_addr , sizeof(client_addr) );
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port); 

    if(inet_pton(AF_INET, ip_addr, &serv_addr.sin_addr)<=0)
    {
#ifdef DEBUG 
        printf("\n inet_pton error occured\n");
#endif        
        return -2;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
#ifdef DEBUG 
        printf("\n Error : Connect Failed \n");
#endif        
        return -3;
    } 
    connfd[x] = sockfd;
    return sockfd;
}

int socket_check(int sock)
{
     if(sock<=0)
         return 0;
     struct tcp_info info;
     int len = sizeof(info); 
     getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
     if((info.tcpi_state == TCP_ESTABLISHED)) {
        return 1; 
     }else{
        return 0; 
     }

    return 1;
}

int close_tcp_client(int x,int clifd) {
    if(clifd < 1) {
#ifdef DEBUG 
        printf("Not a volid connection socket:%d\n", clifd);
#endif        
        return -1;
    }
    close(clifd);
    connfd[x] = -1;
    return 0;
}

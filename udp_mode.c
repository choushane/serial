#include <stdio.h>  
#include <stdlib.h>  
#include <string.h> 
#include <unistd.h> 
  
#include <arpa/inet.h>  
#include <sys/socket.h>

#include "serial.h"
#include "udp.h"

struct udp_user *udp = NULL;

int open_udp_client(void)
{
    int client_pid;

#ifdef DEBUG
    printf("UDP Client Starting.....\n");
#endif
    client_pid = socket( AF_INET, SOCK_DGRAM, 0 );

    if(client_pid < 0)
    {
#ifdef DEBUG
        printf("UDP Client start Error!!\n");
#endif
        return -2;
    }    

#ifdef DEBUG
    printf("UDP Client start Success!! [PID : %d ]\n",client_pid);
#endif

    return client_pid;
}

void assign_user(int pid)
{
    struct udp_user new,*now;
    struct sockaddr_in cliaddr;
    int n,len;
    char buf[BUF_LEN];

    len = sizeof(cliaddr);
    n = recvfrom(pid,buf,sizeof(buf),0,(struct sockaddr *)&cliaddr,&len);

    if(n < 0) return;

    now = udp;
    while(now != NULL)
    {
        if(!strcmp(now->ipaddr,inet_ntoa(cliaddr.sin_addr)))
            return;        
        now = now->next;
    }

    strcpy(new.ipaddr,inet_ntoa(cliaddr.sin_addr));
    new.port = cliaddr.sin_port;

    if(udp == NULL)
        new.next = NULL;
    else
        new.next = udp->next;

    udp = &new;
#ifdef DEBUG
    printf("New USP User [%s:%d] !!\n",inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port);
#endif
}

void send_udp(char * str, char ipaddr[16] ,unsigned short port, int pid)
{
    struct sockaddr_in addr_to;
    socklen_t len;
    int r;

    if( ipaddr[0] == 0 || port == 0)return;

    memset(&addr_to,0,sizeof(addr_to));
    addr_to.sin_family = AF_INET;
    addr_to.sin_port = htons(port);
    addr_to.sin_addr.s_addr = inet_addr(ipaddr);

    len = sizeof(addr_to);

    r = sendto( pid ,str,sizeof(str),0,(struct sockaddr*)&addr_to,len);
#ifdef DEBUG
    if( r > 0)
        printf("Send Success!! [ ip : %s port : %d] \n",ipaddr,port);
#endif        
}

int open_udp_server(unsigned short int port)
{
    struct sockaddr_in saddr;

    int server_pid;

#ifdef DEBUG
    printf("UDP Server Starting.....\n");
#endif
    server_pid = socket( AF_INET, SOCK_DGRAM, 0 );

    if(server_pid < 0)
    {
#ifdef DEBUG
        printf("UDP Server start Error!!\n");
#endif
        return -2;
    }

#ifdef DEBUG
    printf("UDP Server start Success!! [PID : %d ]\n",server_pid);
#endif

    memset( &saddr , 0 , sizeof(saddr));

#ifdef DEBUG
    printf("UDP Server setting....\n");
#endif

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl( INADDR_ANY );

    if (bind( server_pid , (struct sockaddr *)&saddr , sizeof(saddr) ) < 0)
    {
#ifdef DEBUG
        printf("UDP Server set fail\n");
#endif
        return -2;
    }

#ifdef DEBUG
    printf( "UDP server success!![%d]\n",port);
#endif

    return server_pid;
}

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h> 
#include <unistd.h> 
  
#include <arpa/inet.h>  
#include <sys/socket.h>  

#include "serial.h"

const char *error_max_user = "Too many User!!\n";

int user = 0;
const char *ISP[]={"isp1","isp2","isp3","isp4"};

int open_server (unsigned short int server_port)
{
    struct sockaddr_in saddr;

    char buf[BUF_LEN] = {0};

    int server_pid = -1;

#ifdef DEBUG
    printf("TCP Server Starting.....\n");
#endif
    server_pid = socket( AF_INET, SOCK_STREAM, 0 );

    if(server_pid < 0)
    {
#ifdef DEBUG
        printf("TCP Server start Error!!\n");
#endif
        return;
    }    

#ifdef DEBUG
    printf("TCP Server start Success!! [PID : %d ]\n",server_pid);
#endif

    memset( &saddr , 0 , sizeof(saddr));
    memset( buf , 0 , sizeof(buf));

#ifdef DEBUG
    printf("TCP Server setting....\n");
#endif

    saddr.sin_family = AF_INET;  
    saddr.sin_port = htons( server_port );  
    saddr.sin_addr.s_addr = htonl( INADDR_ANY );

    while (bind( server_pid , (struct sockaddr *)&saddr , 16 ) < 0)
    {
#ifdef DEBUG
        printf("TCP Server set fail\n");
        printf("Tcp Server Port[ %d ] locking.. Waitting... \n",atoi(get_conf("local_tcp_port")));
#endif
        sleep(1);
    }    

#ifdef DEBUG
    printf("TCP Server set Success [Port : %d ]\n",server_port);
#endif

    if (listen(server_pid , set_max_user) < 0)
    {
#ifdef DEBUG
        printf("TCP Server listen fail !! [MAX_USER : %d ]\n",set_max_user);
#endif
        return -1;
    }

#ifdef DEBUG
    printf("TCP Server listen Success !! [MAX_USER : %d ]\n",set_max_user);
#endif

#ifdef DEBUG
    printf( "Accepting connections ... \n" );
#endif

    return server_pid;
}

void user_del(void)
{
    int x,tmp;
    for( x = 1; x <= set_max_user;x++)
    {
        if(gl_ip_pid[x] < 0)
        {
            tmp = gl_ip_pid[x];
            gl_ip_pid[x] = gl_ip_pid[x + 1];
            gl_ip_pid[x + 1] = tmp;
        }
    }
}

void user_connect (int server)
{
    int addr_len = sizeof(struct sockaddr_in),check;

    struct sockaddr_in caddr;

    char *group = NULL;

    char path[1024]={0};
    int y = 0,x = 0;

    group = get_conf("group");
    x = accept( server , (struct sockaddr*)&caddr , &addr_len );
    if (!strcmp(inet_ntoa(caddr.sin_addr),"0.0.0.0")){;user--;return;}

    for (y = 0; y < set_max_user;y++ )
    {
#ifdef DEBUG
        printf("Y : %d  C : %d\n",y,gl_ip_pid[y]);
#endif
        if(gl_ip_pid[y] > 0 ) continue;
        gl_ip_pid[y] = x;
        break;
    }    
    if( user > set_max_user ){
#ifdef DEBUG
        printf("Add Uers fail !!\nUser Number : %d  Max User : %d\n",user,set_max_user);
#endif
        write( gl_ip_pid[y] , error_max_user , strlen(error_max_user));
        close(gl_ip_pid[y]);
        gl_ip_pid[y] = -1;

        //sprintf(path,"nvram replace attr als_status_rule %s %s 1",group,ISP[user]);
        //command(path);
        user--;
    }else{
            sprintf(path,"nvram replace attr als_status_rule %s %s %s",group,ISP[y], inet_ntoa(caddr.sin_addr));
            printf("%s\n",path);
            command(path);
#ifdef DEBUG
        printf("User : %d cnnect from : %s : %d  pid : %d\n",y,inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port),gl_ip_pid[user]);
#endif                
                
    }
}

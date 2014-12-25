#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h> 
#include <arpa/inet.h>
#include <sys/socket.h>

#include "serial.h"
#include "udp.h"

int gl_ip_pid[MAX_USER] = {-1};
int set_max_user = 4;
unsigned long Rx = 0;
unsigned long Tx = 0;
int count = 0;
extern const char *ISP[];
int sd_enable = 0;
int usb_enable = 0;
int record = 0;

extern const char *gl_port[];
extern const char *gl_ip_addr[];

extern pthread_mutex_t mutex;

int main( int argc, char **argv )
{
    int con = -1, mode = -1;
    struct termios info;
    char **i = NULL , **p = NULL;
    char *device = NULL;
    char *dir = NULL;
    char buf[BUF_LEN] = {0};
    char path[BUF_LEN] = {0};
    int x = 0, server = -1 , maxfd = 0 , y = 0;
    int len = 0, change = 0,file_mode = 0;
    int udp_server = -1;
    int timer = 0;
    struct udp_user *now = NULL;
    char *group = NULL;
    fd_set fds;
    FILE *fp = NULL;

#ifdef DEBUG
    printf("Start...\n");
#endif

    configure_init( argv[1] );

    set_max_user = (atoi(get_conf("max_connect")) > 0) ? atoi(get_conf("max_connect")) : 4;

    group = get_conf("group");
    
    device = get_conf("device");
    dir = get_conf("alias");

    sd_enable = atoi(get_conf("sd"));
    usb_enable = atoi(get_conf("usb"));

    file_mode = atoi(get_conf("file_mode"));
    mode = atoi(get_conf("mode"));

    record = atoi(get_conf("record"));

    sprintf(path,"nvram replace attr als_status_rule %s enable 1",group );
    command(path);

    sprintf(path,"nvram replace attr als_status_rule %s mode %d",group,mode);
    command(path);

    for(x = 1; x <= set_max_user;x++)
    {
        sprintf(path,"nvram replace attr als_status_rule %s isp%d 2",group,x );
        command(path);
    }

    do{
        con = open_console(device);
        if( con > 0){
            tcgetattr(con, &info);
            con_setting(con);
        }else{
#ifdef DEBUG
            printf("Console Port[ %d ] locking.. Waitting... \n",atoi(get_conf("local_tcp_port")));
#endif
        }    
    }while(con <= 0);

    switch (mode){
        case 0:
#ifdef DEBUG
            printf("Mode : TCP Server !!\n");
#endif
            server = open_server(atoi(get_conf("local_tcp_port")));
            break;
        case 1:
#ifdef DEBUG
            printf("Mode : TCP Client !!\n");
            printf("Start Open socket !!\n");
#endif            
            connect_to_server();
#ifdef DEBUG
            printf("Open socket End!!\n");
#endif                
            break;
        case 2:
#ifdef DEBUG            
            printf("Mode : UDP Mode!!\n");
#endif                
            gl_ip_pid[0] = open_udp_client();
            udp_server = open_udp_server(atoi(get_conf("local_listen_port")));
            break;
        default:
#ifdef DEBUG
            printf("Mode : ERROR !!\n");
#endif                
            break;
    }
    
#ifdef DEBUG
    printf("Start Loop...\n");
#endif                

    for(x = 1; x <= set_max_user;x++)
    {
        sprintf(path,"nvram replace attr als_status_rule %s isp%d 1",group,x );
        command(path);
    }

    thread_init();

    while(1)
    {
        FD_ZERO(&fds);
        maxfd = 0;
        change = 0;
        FD_SET(con, &fds);
        maxfd = MAXNO(con, maxfd);

        if( udp_server > 0)
        {
            FD_SET(udp_server, &fds);
            maxfd = MAXNO(udp_server, maxfd);
        }

        if( server > 0 ){
            FD_SET(server, &fds);
            maxfd = MAXNO(server, maxfd);
            for (y = 0; y <= set_max_user;y++ )
            {
                if( gl_ip_pid[y] <= 0 ) continue;
                if(!socket_check(gl_ip_pid[y]))
                {
                    change = 1;
#ifdef DEBUG
                    printf("User : %d [ %d ]  Disconnect !!\n",y,gl_ip_pid[y]);
#endif
                    close(gl_ip_pid[y]);
                    sprintf(path,"nvram replace attr als_status_rule %s %s 1",group,ISP[user]);
                    command(path);
                    gl_ip_pid[y] = -1;
                    user--;
                    continue;
                }
                FD_SET( gl_ip_pid[y] , &fds);
                maxfd = MAXNO( gl_ip_pid[y] , maxfd);
            }
        }

        if( mode != 2 && change )user_del();

        if(!select(maxfd+1, &fds, NULL, NULL, NULL)) continue; 

        if(FD_ISSET(con, &fds)) {
            memset(buf,0,sizeof(buf));
            len = read(con, buf, sizeof(buf));
            buf[len] = 0;
            if (len <= 0)
                continue;
            count++;
            Rx+=len;

            if(len > (1024 * 500000)) len = 0;
            sprintf(path,"/tmp/%s-Rx",dir);
            fp = fopen(path,"w");
            if (fp){
                fprintf(fp,"%d",Rx);
                fclose(fp);
            }

            if(file_mode > 0)
                save_file(buf,len,dir);

            if(mode == 2)
            {
                for (i = gl_ip_addr,p = gl_port; i[0] != NULL && p[0] != NULL; *i++,*p++){
                    send_udp(buf,get_conf(i[0]),atoi(get_conf(p[0])),gl_ip_pid[0]);
                }
                if( udp_server > 0 ){ 
                    now = udp;
                    while(now != NULL){
                        send_udp(buf,now->ipaddr,now->port,udp_server); 
                        now = now->next;
                    }
                }
            }else{
                if(mode == 1 && timer > 10000)
                {
                    if (connect_to_server_again() != 0)
                    {
#ifdef DEBUG
                        printf("Thread create error!!\n");
#endif
                    }

                    timer = 0;
                }
                
                pthread_mutex_lock(&mutex);
                write_data(buf,len);
                pthread_mutex_unlock(&mutex);
                timer++;
            }
        }

        if(udp_server > 0)
        {   
            if(FD_ISSET(udp_server, &fds)){
                assign_user(udp_server);}
        }

        if(server > 0)
        {    
            if(FD_ISSET(server, &fds))
                user_connect(server);
            for (y = 0; y <= set_max_user;y++ )
            {
                if( gl_ip_pid[y] <= 0 ) continue;
                if(FD_ISSET(gl_ip_pid[y], &fds))
                    forwarding(con, gl_ip_pid[y]);
            }
        }
    }

    con_close(con,&info);
#ifdef DEBUG
    printf("Close..");
#endif                
    return 0;
}

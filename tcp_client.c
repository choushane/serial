#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h> 

#include "serial.h"

extern const char *ISP[];

void read_data(int fd)
{
    int res;
    char buf[BUF_LEN];

    memset(buf,0,sizeof(buf));

    res = read(fd, buf, sizeof(buf));
    if( res == 0 || res < 0)
	    return;

    buf[res] = 0;

}

void write_data(char *data,int len)
{
    int x;
    char path[BUF_LEN]={0};
    char *group = NULL;

    if(!len || len == 0)
        len = strlen(data);

    group=get_conf("group");

    for(x = 0; x <= set_max_user;x++){
        if(gl_ip_pid[x] <= 0)continue;
        if(!socket_check(gl_ip_pid[x]))
        {
#ifdef DEBUG
            printf("User : %d [ %d ]  Disconnect !!\n",x,gl_ip_pid[x]);
#endif
            close(gl_ip_pid[x]);
            gl_ip_pid[x] = -1;
            sprintf(path,"nvram replace attr als_status_rule %s %s 1",group,ISP[user]);
            command(path);
            user--;
            continue;
        }
        write(gl_ip_pid[x],data,len);
    }
    //user_del();
}

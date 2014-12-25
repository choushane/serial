#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <fcntl.h> 
#include <sys/ioctl.h>

#include <linux/usbdevice_fs.h>

#define BAUDRATE B57600
#define DEVICE_NUM 5
#define BUF_LEN 1024
#define ROW_NUM 30
#define COL_NUM 30
#define MAXNO(__A__, __B__) ((__A__ > __B__) ? __A__ : __B__)

const char *DEVICE[] = {"/dev/ttyS0","/dev/ttyS1","/dev/ttyUSB0","/dev/tty",NULL};
const char data_buf[DEVICE_NUM][BUF_LEN];

void reset_usb(int fd, char *device)
{
    int rc;

    printf("Resetting USB device %s\n", device);

    rc = ioctl(fd, USBDEVFS_RESET, 0);
    if (rc < 0) {
        printf("Error in ioctl\n");
    }
    printf("Reset Success!!\n");
}

void con_close(int fd,struct termios *info)
{
    close(fd);
    tcsetattr(fd, TCSANOW, info);
}

int open_console (char *device )
{
    int result;

    if(!*device)   
	return -1; 

    result = open( device , O_RDWR | O_NOCTTY | O_NDELAY );

    if ( result < 0 ){
	printf("Error : %s fail!!\n",device);
	return -1;
    }
    printf("open %s success\n",device);   
    return result;
}

void con_setting(int fd)
{
    struct termios newtio;
    memset(&newtio,0,sizeof(newtio));

    newtio.c_cflag = BAUDRATE | CS8 | 0 | 0 | 0 | CLOCAL | CREAD;

    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;

    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &newtio);
}

char * str_replace(char *buf)
{
    int x;
        for( x = 0; x < strlen(buf);x++,*(buf++))
    	{
	    if((buf - 0x0a) == 0 )
	    {
		*buf = 0x06;
	    }
	}
    return buf;
}

char * read_data(int fd,char *interface)
{
    int res,x;
    char buf[BUF_LEN];

    memset(buf,0,sizeof(buf));

    res = read(fd, &buf, sizeof(buf));
    if( res == 0 || res < 0)
	return;

    buf[res] = 0;

    //return str_replace(&buf);
    return &buf;
/*
    for(x = 0 ; x < res; x++)
	printf("%c",buf[x]);

    if( buf[0] == '@')return -1;
    return 1;
*/
}

int write_to_con(int fd,char *interface)
{
    int res,x;
    char buf[BUF_LEN];

    memset(buf,0,sizeof(buf));

    res = read(fd, &buf, sizeof(buf));
    if( res == 0 || res < 0)
	return 0;

    //buf[res] = 0;

    if (!strncmp(buf,"exit",4) || !strncmp(buf,"EXIT",4))
        return -1;

    write(fd,buf,sizeof(buf));
}

void painting_row(char * data,int len)
{
    int x;

    printf("|%s",data);

    for(x = 0; x < (ROW_NUM - len); x++)
    	printf(" ");
}

void painting (int fd)
{
    char **p = DEVICE;
    int x,y,len;
    char buff[BUF_LEN];
    char tmp[BUF_LEN];

    for(x = 0;x < COL_NUM ;x++)
    { 
	if(x == 0)
	{
    	    for(; p[0] != NULL; *p++)
	    {
   		//memset(buff,0,sizeof(buff));
		sprintf(buff,"DEVICE : %s",p[0]);
		painting_row(buff,strlen(buff));
	    }
	}else
	{
	    for(y = 0;y < (DEVICE_NUM - 1);y++)
	    {
		if(data_buf[y] != NULL && strlen(data_buf[y]) > 0)
		{
		    len = (strlen(data_buf[y]) > ROW_NUM) ? ROW_NUM : strlen(data_buf[y]);
		    strncpy(tmp,(data_buf[y] + (x - 1) * ROW_NUM ),len);
	    	    painting_row(tmp,len);
		}
		else
	    	    painting_row("",0);
		    //printf("%d  size %d  str %d",y,sizeof(d[y]),strlen(d[y]));
		    //printf("%s",d[y]);
	    }
	}
        printf("|\n");
    }
}

int fds_get_num(int *a,int b)
{
    int result;
    result=MAXNO(*a,b);   
    if(*(a++) <= 0)
	return result; 
    result = fds_get_num(a,result);
    return result; 
}

int main( int argc, char **argv )
{
    int con[DEVICE_NUM]={0};
    int activefd , maxfd ;
    int input = 0,x;
    int count = 0;
    char **p = DEVICE;
    char tmp[BUF_LEN];
    struct termios info[DEVICE_NUM];
    fd_set fds;

    printf("Start...\n");

    FD_ZERO(&fds);
    for(x = 0; p[0] != NULL; *p++,x++)
    {
        con[x] = open_console(p[0]);
	if( con[x] > 0){

	    if( x != 3)
	    {
	    	tcgetattr(con[x], &info[x]);
            	con_setting(con[x]);
	    }
            FD_SET(con[x], &fds);

	}
    }

    maxfd = fds_get_num(&con,0);
 
    printf("Start Read...\n");

    while(1)
    {
	if ((activefd = select(maxfd+1, &fds, NULL, NULL, NULL)) > 0)
	{
    	    for(x = 0; x < DEVICE_NUM; x++)
	    {
		if(con[x] > 0)
		{
	            if (FD_ISSET(con[x], &fds))
		    {
		    	if(x == 3)
		    	    input = write_to_con(con[x],&DEVICE[x]);
		    	else
			{
			    strcpy(tmp,read_data(con[x],&DEVICE[x]));
			    if(strlen(data_buf[x]) + strlen(tmp) > BUF_LEN)
				memset(data_buf[x],0,sizeof(data_buf[x]));
		            strncat(data_buf[x],tmp,strlen(tmp));
			}
	    		//painting(con[3]);
		    }
		}
	    }
	    if(input < 0) break;
	}

        maxfd = fds_get_num(&con,0);
    }

    for(x = 0; x < DEVICE_NUM; x++)
        con_close(con[x],&info[x]);
    printf("Close..\n");

    return 0;
}

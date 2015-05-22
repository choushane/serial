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
const char *data_buf;

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

void read_data(int fd,char *interface)
{
    int res;
    char buf[BUF_LEN];

    memset(buf,0,sizeof(buf));

    res = read(fd, &buf, sizeof(buf));
    if( res == 0 || res < 0)
	return;

    buf[res] = 0;

    printf("Input Data : %s \n",buf);
    //write_data(fd,&buf);
}

void write_data(int fd,char *data)
{
    char info[BUF_LEN] = "Output Data : ";
    strcat(info,data);
    strcat(info,"\n");
    write(fd,info,strlen(info));

}

int main( int argc, char **argv )
{
    int con;
    char *tmp;
    struct termios info;

    printf("Start...\n");

    if(!argv[1])
    {
	printf("Input Error!!");
	exit(0);
    }

    con = open_console(argv[1]);
    if( con > 0){
	    tcgetattr(con, &info);
	    con_setting(con);
    }
 
    printf("Start Read...\n");
    while(1)
	read_data(con,argv[1]);

    con_close(con,&info);
    printf("Close..\n");
    return 0;
}

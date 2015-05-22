#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "serial.h"


void con_close(int fd,struct termios *info)
{
    close(fd);
    tcsetattr(fd, TCSANOW, info);
}

void set_baudrate(struct termios *newtio,char *data)
{
    switch(atoi(data))
    {
        case 110:
            cfsetispeed(newtio, B110);  
            cfsetospeed(newtio, B110); 
        break;
        case 134:
            cfsetispeed(newtio, B134);  
            cfsetospeed(newtio, B134); 
        break;
        case 150:
            cfsetispeed(newtio, B150);  
            cfsetospeed(newtio, B150); 
        break;
        case 300:
            cfsetispeed(newtio, B300);  
            cfsetospeed(newtio, B300); 
        break;
        case 600:
            cfsetispeed(newtio, B600);  
            cfsetospeed(newtio, B600); 
        break;
        case 1200:
            cfsetispeed(newtio, B1200);  
            cfsetospeed(newtio, B1200); 
        break;
        case 2400:
            cfsetispeed(newtio, B2400);  
            cfsetospeed(newtio, B2400); 
        break;
        case 4800:
            cfsetispeed(newtio, B4800);  
            cfsetospeed(newtio, B4800); 
        break;
        case 9600:
            cfsetispeed(newtio, B9600);  
            cfsetospeed(newtio, B9600); 
        break;
        case 19200:
            cfsetispeed(newtio, B19200);  
            cfsetospeed(newtio, B19200); 
        break;
        case 38400:
            cfsetispeed(newtio, B38400);  
            cfsetospeed(newtio, B38400); 
        break;
        case 57600:
            cfsetispeed(newtio, B57600);  
            cfsetospeed(newtio, B57600); 
        break;
        case 115200:
            cfsetispeed(newtio, B115200);  
            cfsetospeed(newtio, B115200); 
        break;
        case 230400:
            cfsetispeed(newtio, B230400);  
            cfsetospeed(newtio, B230400); 
        break;
        default:
            printf("Baudrate %s is wrong!!\n",data);
        break;    
    }
}

int open_console (char *device )
{
    int result = -1;

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


/* UART configure 
 * Reference form 
 * "http://ulisse.elettra.trieste.it/services/doc/serial/config.html"
 * */
void con_setting(int fd)
{
    struct termios newtio;

    char *data = NULL;

    memset(&newtio,0,sizeof(newtio));

    set_baudrate(&newtio,get_conf("baudrate"));

    newtio.c_cflag |= CS8 | 0 | 0 | 0 | CLOCAL | CREAD;

    data = get_conf("data_bits");
    switch( atoi(data) ){/*Data bit*/
        case 7:
            newtio.c_cflag |= CS7;
            break;
        default :
            newtio.c_cflag |= CS8;
            break;
    }

    data = get_conf("stop_bit");
    switch( atoi(data) ){/*stop_bit*/
        case 2:
            newtio.c_cflag |=  CSTOPB;
            break;
        default :
            newtio.c_cflag &=  ~CSTOPB;
            break;
    }

    data = get_conf("parity");
    switch( atoi(data) ){/*Parity*/
        case 1: /*Odd*/
            newtio.c_cflag |= PARENB;
            newtio.c_cflag |= PARODD;
            newtio.c_cflag &= ~CSTOPB;
            newtio.c_cflag &= ~CSIZE;
            newtio.c_cflag |= CS7;
            break;
        case 2: /*even*/
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            newtio.c_cflag &= ~CSTOPB;
            newtio.c_cflag &= ~CSIZE;
            newtio.c_cflag |= CS7;
            break;
        case 3: /*mark*/
            newtio.c_cflag &= ~PARENB;
            newtio.c_cflag |= CSTOPB;
            newtio.c_cflag &= ~CSIZE;
            newtio.c_cflag |= CS7;
            break;
        case 4: /*space*/
            newtio.c_cflag &= ~PARENB;
            newtio.c_cflag &= ~CSTOPB;
            newtio.c_cflag &= ~CSIZE;
            newtio.c_cflag |= CS8;
            break;
        default :/*none*/
            newtio.c_cflag &= ~PARENB;
            newtio.c_cflag &= ~CSTOPB;
            newtio.c_cflag &= ~CSIZE;
            newtio.c_cflag |= CS8;
            break;
    }
/*
    data = get_conf("flow_control");
    if(!strcmp(data,"rts")){
        //newtio.c_cflag |= CNEW_RTSCTS;
    }else if(!strcmp(data,"xon")){
        newtio.c_iflag |= (IXON | IXOFF | IXANY);
    }else{
        //newtio.c_cflag &= ~CNEW_RTSCTS;
    }
*/

    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;

        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);
}

void forwarding(int console, int client)
{
    int len = 0;
    char buf[BUF_LEN] = {0};

    memset(buf,0,sizeof(buf));
    len = recv( client, buf, sizeof(buf), 0  );
    write(console,buf,len);
}

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


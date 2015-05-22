#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include "serial.h"
#include <errno.h>

const char *gl_ip_addr[] = {"ip_addr_1","ip_addr_2","ip_addr_3","ip_addr_4",NULL};
const char *gl_port[] = {"port_1","port_2","port_3","port_4",NULL};
const char *gl_local_port[] = {"local_port_1","local_port_2","local_port_3","local_port_4",NULL};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void thread_init(void)
{
    pthread_mutex_init(&mutex,NULL);
}

void thread_exit(void)
{
    pthread_mutex_destroy(&mutex);
}

void connect_to_server(void)
{
    char **i = NULL , **p = NULL , **l = NULL;
    int face_fd = -1 , x = 0;

    pthread_mutex_lock(&mutex);
    for (x = 0,i = gl_ip_addr,p = gl_port,l = gl_local_port; i[0] != NULL && p[0] != NULL && l[0] != NULL; *i++,*p++,*l++,x  ++){
        if (gl_ip_pid[x] > 0) continue;
        if ((face_fd = open_tcp_client(x,get_conf(i[0]),atoi(get_conf(p[0])),atoi(get_conf(l[0])))) < 0 )
        {
#ifdef DEBUG                    
            printf("Error opening socket!! [ ip : %s port : %d num : %d local_port : %d ] %d\n",get_conf(i[0]),atoi(get_conf(p[0])),x,atoi(get_conf(l[0])),face_fd);
#endif
            continue;
        }
#ifdef DEBUG               
        printf("Socket open success[ pid : %d ip : %s port : %d ]\n",face_fd,get_conf(i[0]),atoi(get_conf(p[0])));
#endif
        gl_ip_pid[x] = face_fd;
    }
    pthread_mutex_unlock(&mutex);
}

int connect_to_server_again (void)
{
    pthread_t id;
    int ret;

#ifdef DEBUG
        printf("Thread creating\n");
#endif

   if((ret = pthread_create(&id,NULL,(void *)connect_to_server,NULL)) != 0)
        return ret;
    return 0;
}

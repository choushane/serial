#include <termios.h>
#define BUF_LEN 3096
#define DEVICE_NUM 5
//#define DEBUG 1
#define MAX_USER 20
#define NAME_LEN 512
#define MAXNO(__A__, __B__) ((__A__ > __B__) ? __A__ : __B__)

extern int user;
extern int set_max_user;
extern int connfd[MAX_USER];
extern int gl_ip_pid[MAX_USER];

extern void assign_user(int);
extern int open_udp_client(void);
extern int open_udp_server(unsigned short int);
extern void user_del(void);
extern void read_data(int);
extern int socket_check(int);
extern int connect_to_server_again(void);
extern void connect_to_server(void);
extern void forwarding(int,int);
extern char *get_conf(char *);
extern void write_data(char *,int);
extern void save_file(char *,int,char *);
extern void send_udp(char *,char[],unsigned short,int);
extern int open_tcp_client(int ,char*,unsigned short int,unsigned short int);
extern int open_console(char*);
extern void con_setting(int);
extern void con_close(int,struct termios *);
extern int close_tcp_client(int,int);
extern void tcp_client(void);
extern void user_connect(int);
extern int open_server(unsigned short int);
extern void configure_init(char*);

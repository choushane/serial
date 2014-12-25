extern struct udp_user *udp;

struct udp_user {
    char ipaddr[16];
    unsigned short port;
    struct udp_user *next;
};

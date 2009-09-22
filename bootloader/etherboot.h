#include "osdep.h"

extern int network_ready;

extern int bootp P((void));
extern int rarp P((void)); 
extern void print_network_configuration P((void));
extern int arp_server_override P((const char *buf));    

extern void udp_init(void);
extern int udp_close(void);
extern int udp_send(char *,int,int,int);
extern int udp_send_withmac(char *,int,int,int);
extern int udp_send_lbs(char *,int);
extern int udp_get(char *,int *,int,int *);

typedef struct {
		    unsigned long   s_addr;
} in_addr;                                                                      

#define MAX_ARP	5
#define ARP_CLIENT	0
#define ARP_SERVER	1
#define ARP_GATEWAY	2

struct arptable_t {
    in_addr ipaddr;
    unsigned char node[6];
};                                                                              

// ADDED for POE
//

#define ARP       0x0608
#define ARP_TYPE  0x0806
#define ARP_REQ   0x0100
#define ARP_REPLY 0x0200

#define MAX_ARP_RETRIES 6

// IP & UDP datagram headers
//
#define IP      0x0008
#define IP_TYPE 0x0800

struct iphdr {
   char verhdrlen;
   char service;
   unsigned short len;
   unsigned short ident;
   unsigned short frags;
   char ttl;
   char protocol;
   unsigned short chksum;
   in_addr src;
   in_addr dest;
};

#define UDP 0x11

struct udphdr {
               unsigned short src;
               unsigned short dest;
               unsigned short len;
               unsigned short chksum;
              };

struct arprequest {
        unsigned short hwtype;
        unsigned short protocol;
        char hwlen;
        char protolen;
        unsigned short opcode;
        char shwaddr[6];
        char sipaddr[4];
        char thwaddr[6];
        char tipaddr[4];
};


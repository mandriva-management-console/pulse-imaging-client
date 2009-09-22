/*
 *  $Id: udp.c,v 1.4 2003/04/10 09:37:14 ludo Exp $
 */
/*
 *  GRUB LBS functions
 *  Copyright (C) 2002 Linbox Free & Alter Soft
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "etherboot.h"
#define FSYS_TFTP
#include <filesys.h>
#include <shared.h>
 
#include "pxe.h"

#define nic_macaddr arptable[ARP_CLIENT].node
 
// #define DEBUG

extern struct arptable_t arptable[MAX_ARP];                                                      

typedef struct s_udp_o_s
{
    short   Status;
    unsigned char src_ip[4];
} udp_o_s;
 
typedef struct s_udp_c_s
{
    short   Status;
} udp_c_s;
 
typedef struct s_udp_w_s
{
    short   Status;
    unsigned char ip[4];
    unsigned char gw[4];
    unsigned short  src_port;
    unsigned short  dst_port;
    short   buffer_size;
    short   BuffOff;
    short   BuffSeg;
} udp_w_s;
 
typedef struct s_udp_r_s
{
    short   Status;
    unsigned char src_ip[4];
    unsigned char dst_ip[4];
    unsigned short  src_port;
    unsigned short  dst_port;
    short   buffer_size;
    short   BuffOff;
    short   BuffSeg;
} udp_r_s;
 
unsigned char *udp_packet_s=(char *)0x6000;
unsigned char *udp_packet_r=(char *)0x6800;
//PASSWORD_BUF;
 
udp_w_s *udp_w=(udp_w_s*)0x7000;
udp_r_s *udp_r=(udp_r_s*)0x7100;
udp_o_s *udp_o=(udp_o_s*)0x7200;
udp_c_s *udp_c=(udp_c_s*)0x7300;

int udp_send(char *buf,int size,int s_port,int d_port)
{
    int ret,i;

    if (buf) fast_memmove(udp_packet_s,buf,size);
 
    udp_w->buffer_size=size;

#ifdef DEBUG
    for(i=0;i<size;i++) printf("%x ",udp_packet_s[i]);
    printf("\n");
#endif

    udp_w->src_port=(s_port>>8)|((s_port&255)<<8);
    udp_w->dst_port=(d_port>>8)|((d_port&255)<<8);

    ret=pxe_call(0x33,udp_w);

#ifdef DEBUG
    grub_printf("UDP Write Call : %d (",ret);
    grub_printf("Status : %d)\n",udp_w->Status);
#endif
 
    return ret;
}

/*
 * Same as udp_send but put the mac address appended to the packet
 * (needed for LBS's getClientResponse)
 */
int udp_send_withmac(char *buf,int size,int s_port,int d_port)
{
    char ip[] = "Mc:xx:xx:xx:xx:xx:xx";
    char hex[]="0123456789ABCDEF";
    char newbuf[1500];		/* LBS command strings should not be bigger than 512 chars */
    unsigned char *ptr = (char *)nic_macaddr;
    int i;
    
    if (size > 1500-21) {
	// don't append the mac address to avoid any overflow
	return (udp_send(buf,size,s_port,d_port));
    }

    // got problems with grub_memmove so do a manual copy !
    for (i=0; i<size; i++) {
	newbuf[i] = buf[i];
    }
    newbuf[i] = '\0';
    
    ip[ 3]=hex[((*ptr)&0xF0)>>4]; ip[ 4]=hex[(*ptr++)&0x0F];
    ip[ 6]=hex[((*ptr)&0xF0)>>4]; ip[ 7]=hex[(*ptr++)&0x0F];
    ip[ 9]=hex[((*ptr)&0xF0)>>4]; ip[10]=hex[(*ptr++)&0x0F];
    ip[12]=hex[((*ptr)&0xF0)>>4]; ip[13]=hex[(*ptr++)&0x0F];
    ip[15]=hex[((*ptr)&0xF0)>>4]; ip[16]=hex[(*ptr++)&0x0F];
    ip[18]=hex[((*ptr)&0xF0)>>4]; ip[19]=hex[(*ptr++)&0x0F];
    

    for (i=0; i<grub_strlen(ip); i++) {
	newbuf[size+i+1] = ip[i];
    }
    //grub_memmove (newbuf+size, ip, grub_strlen(ip));
    
    return (udp_send(newbuf,size+grub_strlen(ip)+1,s_port,d_port));
}

/* send a control packet to the LBS */
int udp_send_lbs(char *buf,int size)
{
    return (udp_send_withmac(buf, size, 1001, 1001));
}


int udp_get(char *buf,int *size,int d_port,int *s_port)
{
    int ret;
 
    udp_r->buffer_size=2000;
    udp_r->dst_port=(d_port>>8)|((d_port&255)<<8);
 
    ret=pxe_call(0x32,udp_r);

    if (udp_r->Status!=0) return udp_r->Status;
	    
    if (buf) fast_memmove(buf,udp_packet_r,udp_r->buffer_size);

#ifdef DEBUG
        grub_printf("UDP Read Call : %d (",ret);
        grub_printf("Status : %d, size %d)\n",udp_r->Status,udp_r->buffer_size);
        grub_printf("Answer from : %d.%d.%d.%d from port %d\n",
		    udp_r->src_ip[0],
		    udp_r->src_ip[1],
		    udp_r->src_ip[2],
		    udp_r->src_ip[3],
		    (udp_r->src_port>>8)|((udp_r->src_port&255)<<8) );
#endif
    *s_port=(udp_r->src_port>>8)|((udp_r->src_port&255)<<8);
 
    *size=udp_r->buffer_size;
 
    return udp_r->Status;
//    return ret;
}                                                                                         
void udp_init(void)
{
    int ret, sz=0, port;
 
    grub_memmove((char *)&(udp_o->src_ip),(char *)&arptable[ARP_CLIENT].ipaddr,sizeof(in_addr));
    ret=pxe_call(0x30,udp_o);
#ifdef DEBUG
    grub_printf("UDP Init Call : %d (",ret);
    grub_printf("Status : %d)\n",udp_o->Status);
#endif
    if (udp_o->Status) {
      /* not properly closed ? */
      udp_close();
      /* retry */
      grub_memmove((char *)&(udp_o->src_ip),(char *)&arptable[ARP_CLIENT].ipaddr,sizeof(in_addr));
      ret=pxe_call(0x30,udp_o);
    }

    // PRE-INIT UDP_W structure
    grub_memmove((char *)&(udp_w->ip),(char *)&arptable[ARP_SERVER].ipaddr,sizeof(in_addr));
    grub_memmove((char *)&(udp_w->gw),(char *)&arptable[ARP_GATEWAY].ipaddr,sizeof(in_addr));  
    udp_w->BuffOff=((unsigned long)udp_packet_s)&15;
    udp_w->BuffSeg=((unsigned long)udp_packet_s)>>4;

    // PRE-INIT UDP_R structure
    grub_memmove((char *)&(udp_r->dst_ip),(char *)&arptable[ARP_CLIENT].ipaddr,sizeof(in_addr));
    udp_r->BuffOff=((unsigned long)udp_packet_r)&15;
    udp_r->BuffSeg=((unsigned long)udp_packet_r)>>4;

    /* Some buggy PXE bioses need a read before sending... (SMC cards) 
       (or is it our fault ?) */    
    udp_get(NULL, &sz, 1001, &port);
}

int udp_close(void)
{
 int ret;
				 
 ret=pxe_call(0x31,udp_c);
#ifdef DEBUG
 grub_printf("UDP Close Call : %d (",ret);
 grub_printf("Status : %d)\n",udp_c->Status);
#endif
 return ret;
}                                                                                                                     


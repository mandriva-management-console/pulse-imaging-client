/*
 * (c) 2003-2007 Linbox FAS, http://linbox.com
 * (c) 2008-2009 Mandriva, http://www.mandriva.com
 *
 * $Id$
 *
 * This file is part of Pulse 2, http://pulse2.mandriva.org
 *
 * Pulse 2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Pulse 2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pulse 2; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "etherboot.h"
#include "generic.h"
#ifndef FSYS_TFTP
#define FSYS_TFTP
#endif
#include <filesys.h>
#include <shared.h>
#include "pxe.h"

//#define DEBUG 1
//#define DEBUG_PXE_STRUCT
//#define DEBUG_DHCP
// #define USE_PXE_TFTP
//#define DEBUG_PXE_OPEN

int network_ready = 0;
int pxev2 = 0;

/* last image name restored */
extern char imgname[32];

//unsigned char nic_macaddr[6];
#define nic_macaddr arptable[ARP_CLIENT].node

struct arptable_t arptable[MAX_ARP];
static unsigned long netmask;

int pxe_detected = 0;
static int udp_open = 0;
unsigned char *PXEEntry = NULL;

#define PXE_ID    0x45585021    // !PXE
#define PXENV_ID  0x4E455850    // PXEN
#define PXENV_ID2 0x2B56        // V+

unsigned char *scan_pxe (void);
unsigned char *scan_pxenv (void);
void get_pxe_entry (void);
void parse_dhcp_options (unsigned char *ptr);

struct nic
{
  void (*reset) (struct nic *);
  int (*poll) (struct nic *);
  void (*transmit) (struct nic *, const char *d,
                    unsigned int t, unsigned int s, const char *p);
  void (*disable) (struct nic *);
  int flags;                    /* driver specific flags */
                                        /*struct rom_info */ void *rom_info;
                                        /* -> rom_info from main */
  unsigned char *node_addr;
  char *packet;
  unsigned int packetlen;
  void *priv_data;              /* driver can hang private data here */
};

struct nic *nic;

unsigned char *
scan_pxe (void)
{
  unsigned long *i;
  int fbm = 0;

#ifndef QUIET
  printf ("Trying to find !PXE , from %dKB to 640KB\n", fbm =
          *(unsigned short *) 0x413);
#endif
  for (i = (unsigned long *) (fbm * 1024); i < (unsigned long *) 0xA0000;
       i += 4)
    if (*i == PXE_ID)
      {
        unsigned char *str = (unsigned char *) i;
        int j, lg = *(str + 4);
        unsigned char chksum = 0;

        if (lg >= 8)
          {
            for (j = 0; j < lg; j++)
              chksum += *(str + j);
            if (chksum == 0)
              return str;
          }
      }
  return NULL;
}

unsigned char *
scan_pxenv (void)
{
  unsigned long *i;
  int fbm;

  printf ("Trying to find PXENV+ , from %dKB to 640KB\n", fbm =
          *(unsigned short *) 0x413);

  for (i = (unsigned long *) (fbm * 1024); i < (unsigned long *) 0xA0000;
       i += 4)
    if ((*i == PXENV_ID) && ((*(unsigned short *) (i + 1)) == PXENV_ID2))
      {
        unsigned char *str = (unsigned char *) i;
        int j, lg = *(str + 8);
        unsigned char chksum = 0;

        if (lg >= 0x26)
          {
            for (j = 0; j < lg; j++)
              chksum += *(str + j);
            if (chksum == 0)
              return str;
          }
      }
  return NULL;
}

void
get_pxe_entry (void)
{
  unsigned short v;
  unsigned char *ptr;

#ifdef DEBUG
  int i;
#endif
  pxe_detected = 1;

  // DEBUG
#ifdef DEBUG_PXE_STRUCT
  PXEEntry =
    (unsigned char *) (pxe_pointer_segment << 4) + pxe_pointer_offset;
  printf ("\nPXE Struct at ES:BX : %x\n", PXEEntry);

  ptr = (unsigned char *) (pxe_stack_segment << 4) + pxe_stack_offset;
  printf ("%x %x %x %x %x %x %x %x\n",
          *(unsigned short *) (ptr + 0),
          *(unsigned short *) (ptr + 2),
          *(unsigned short *) (ptr + 4),
          *(unsigned short *) (ptr + 6),
          *(unsigned short *) (ptr + 8),
          *(unsigned short *) (ptr + 10),
          *(unsigned short *) (ptr + 12), *(unsigned short *) (ptr + 14));
  printf ("PXE Struct (stack)  : %x:%x", pxe_stack_segment, pxe_stack_offset);
  PXEEntry = (unsigned char *) ((*(unsigned short *) (ptr + 2)) << 4) +
    *(unsigned short *) (ptr + 0);
  printf ("-> %x (%x %x %x %x)\n", PXEEntry, *(PXEEntry + 0), *(PXEEntry + 1),
          *(PXEEntry + 2), *(PXEEntry + 3));

  PXEEntry = (unsigned char *) (pxe_int1a_segment << 4) + pxe_int1a_offset;
  printf ("PXE Int1a     ES:BX : %x\n", PXEEntry);
  getkey ();
#endif

  ptr = (unsigned char *) (pxe_stack_segment << 4) + pxe_stack_offset + 4;
  PXEEntry = (unsigned char *) ((*(unsigned short *) (ptr + 2)) << 4) +
    *(unsigned short *) (ptr + 0);

  if (*(unsigned long *) PXEEntry != PXE_ID)
    {
      ptr = (unsigned char *) (pxe_pointer_segment << 4) + pxe_pointer_offset;
      PXEEntry =
        (unsigned char *) (pxe_int1a_segment << 4) + pxe_int1a_offset;
#ifdef DEBUG
      if (ptr != PXEEntry)
        printf ("Warning : ES:BS != Int1A -> trying Int1A\n");
#endif
      if ((*(unsigned long *) PXEEntry != PXENV_ID) ||
          (*(unsigned short *) (PXEEntry + 4) != PXENV_ID2))
        {
          if ((PXEEntry = scan_pxe ()) != NULL)
            goto have_pxe;
          if ((PXEEntry = scan_pxenv ()) != NULL)
            goto have_pxenv;
          printf ("PXE ROM Entry not found !");

          {
            ptr =
              (unsigned char *) (pxe_stack_segment << 4) + pxe_stack_offset;
            PXEEntry =
              (unsigned char *) ((*(unsigned short *) (ptr + 10)) << 4) +
              *(unsigned short *) (ptr + 8);

            if (*(unsigned long *) (PXEEntry + 1500) == 0xAABBCCDDL)
              {
                unsigned long depl;

                depl =
                  (unsigned char *) pxe_emulation -
                  (unsigned char *) pxe_call;
                depl -= 2;
#ifdef DEBUG
                printf ("Jmp Depl : %d\n", depl);
#endif
                ptr = (unsigned char *) pxe_call;
                *(unsigned char *) ptr = 0xEB;
                *(unsigned char *) (ptr + 1) = depl;
#ifdef DEBUG
                printf ("MAGIC: %x\n", *(unsigned long *) (PXEEntry + 1500));
                printf ("NIC @: %x\n", *(unsigned long *) (PXEEntry + 1504));
#endif
                nic = (struct nic *) *(unsigned long *) (PXEEntry + 1504);
                return;
              }
          }

        nel:
          goto nel;
        }
      else
        {
        have_pxenv:
          printf ("PXENV+ : API : %x\n", v =
                  *(unsigned short *) (PXEEntry + 6));
          if (v >= 0x201)
            {
              pxev2 = 1;
              PXEEntry =
                (unsigned char
                 *) (((*(unsigned short *) (PXEEntry + 0x2A)) << 4) +
                     (*(unsigned short *) (PXEEntry + 0x28)));
            }
          if (*(unsigned long *) (PXEEntry) != PXE_ID)
            {

              if ((PXEEntry = scan_pxe ()) != NULL)
                goto have_pxe;

              PXEEntry =
                (unsigned char *) (pxe_int1a_segment << 4) + pxe_int1a_offset;
#ifdef DEBUG

              for (i = 0; i < 0x30; i++)
                {
                  if ((i & 15) == 0)
                    printf ("\n%x : ", i);
                  printf ("%x ", *(unsigned char *) (PXEEntry + i));
                }
              printf ("\n");
#endif
              printf ("Old PXENV+ Struct via %x:%x\n",
                      *(unsigned short *) (PXEEntry + 0x0C),
                      *(unsigned short *) (PXEEntry + 0x0A));
              set_pxe_entry (*(unsigned short *) (PXEEntry + 0x0C),
                             *(unsigned short *) (PXEEntry + 0x0A));
              {
                unsigned long depl;

                depl = (unsigned char *) oldpxe - (unsigned char *) pxe_call;
                depl -= 2;
                printf ("Jmp Depl : %d\n", depl);
                ptr = (unsigned char *) pxe_call;
                *(unsigned char *) ptr = 0xEB;
                *(unsigned char *) (ptr + 1) = depl;
              }
#ifdef DEBUG
              for (i = 0; i < 0xF8; i++)
                {
                  if ((i & 15) == 0)
                    printf ("\n%x : ", i);
                  printf ("%x ", *(unsigned char *) (ptr + i));
                }
              printf ("\n");
              getkey ();
#endif

              return;
            }
          else
            {
              goto have_pxe;
            }
        }
    }
  else
    {
#ifndef QUIET
      printf ("Found PXEEntry, directly via pxe_pointer_segment...\n");
#endif
    }

have_pxe:
#ifdef DEBUG
  printf ("!PXE Ptr = %x\n", PXEEntry);
  printf ("PXE Entry is calling : %x:%x\n",
          *(unsigned short *) (PXEEntry + 0x12),
          *(unsigned short *) (PXEEntry + 0x10));
#endif
#ifdef DEBUG_PXE_STRUCT
  printf ("PXE 32 bits Entry : %x:%x\n",
          *(unsigned short *) (PXEEntry + 0x16),
          *(unsigned short *) (PXEEntry + 0x14));
#endif
  set_pxe_entry (*(unsigned short *) (PXEEntry + 0x12),
                 *(unsigned short *) (PXEEntry + 0x10));
}

#ifdef DEBUG_PXE_STRUCT
void
dump_pxe_struct (void)
{
  int i;
  unsigned char chksum = 0;

  for (i = 0; i < PXEEntry[4]; i++)
    chksum += PXEEntry[i];

  printf ("PXE Struct :\n Signature : %c%c%c%c\n", PXEEntry[0], PXEEntry[1],
          PXEEntry[2], PXEEntry[3]);
  printf (" StructLength : %d\n Checksum : %x (sum=%x)\n", PXEEntry[4],
          PXEEntry[5], chksum);
  for (i = 0x08; i < 0x1C; i += 4)
    printf (" SegOff16 : %x:%x\n", *(unsigned short *) (PXEEntry + i + 2),
            *(unsigned short *) (PXEEntry + i + 0));
  printf (" SegDescCnt : %x\n FirstSelector : %x\n", PXEEntry[0x1D],
          *(unsigned short *) (PXEEntry + 0x1E));
  for (i = 0x20; i < 0x58; i += 8)
    printf (" SegDesc : seg %x addr %x lg %x\n",
            *(unsigned short *) (PXEEntry + i),
            *(unsigned long *) (PXEEntry + i + 2),
            *(unsigned short *) (PXEEntry + i + 6));

  getkey ();
}
#endif

void
parse_dhcp_options (unsigned char *ptr)
{

  while (*ptr != 0)
    {
      switch (*ptr)
        {
        case 1:
          if (ptr[1] == 4)
            {
              *(in_addr *) & netmask = *(in_addr *) (ptr + 2);
#ifdef DEBUG_DHCP
              printf ("Got NetMask\n");
#endif
            }
          ptr += (2 + ptr[1]);
          break;
        case 3:
          if (ptr[1] == 4)
            {
              *(in_addr *) & arptable[ARP_GATEWAY].ipaddr =
                *(in_addr *) (ptr + 2);
#ifdef DEBUG_DHCP
              printf ("Got Gateway\n");
#endif
            }
          ptr += (2 + ptr[1]);
          break;
        default:
#ifdef DEBUG_DHCP
          {
            int i, lg;
            printf ("Option-%d : ", *ptr++);
            lg = *ptr++;
            for (i = 0; i < lg; i++)
              printf ("%x,", *ptr++);
            printf ("\n");
          }
#else
          ptr += (2 + ptr[1]);
#endif
        }
    }
}

extern int bootp
P ((void))
{
    typedef struct s {
        short Status;
        unsigned short Packet;
        unsigned short BuffSize;
        unsigned short BuffOff;
        unsigned short BuffSeg;
        unsigned short BuffLimit;
    } cache;

    cache toto;
    int ret;
    unsigned char *ptr, *cmp;

      if (!pxe_detected) {
        get_pxe_entry ();
#ifdef DEBUG_PXE_STRUCT
        dump_pxe_struct ();
#endif
    }

    if (!PXEEntry)
        return 0;

    toto.Packet = 2;
    toto.BuffSize = 0;
    toto.BuffSeg = 0;
    toto.BuffOff = 0;
    toto.BuffLimit = 2048;
    ret = pxe_call (0x71, &toto);

#ifdef DEBUG
    printf ("Getting Packet type 2 : ");
    printf ("Call : %d (", ret);
    printf ("Status  : %d)\n", toto.Status);
#ifdef DEBUG_DHCP
    printf ("Returned values : Buffer size = %d\n", toto.BuffSize);
    {
        int i;
        ptr = (unsigned char *) ((toto.BuffSeg << 4) + toto.BuffOff);
        printf ("Returned values : Buffer ptr = %x\n", ptr);
        for (i = 0; i < toto.BuffSize; i++) {
            if ((i & 15) == 0) {
                if ((i & 255) == 0)
                    getkey ();
                printf ("\n%x : ", i);
            }
            printf ("%x ", *(ptr + i));
        }
        printf ("\n");
        getkey ();
    }
#endif
#endif

    ptr = (unsigned char *) ((toto.BuffSeg << 4) + toto.BuffOff); {
        int i;
        for (i = 0; i < MAX_ARP * sizeof (struct arptable_t); i++)
            *(((char *) arptable) + i) = 0;
    }

    *(in_addr *) & arptable[ARP_CLIENT].ipaddr = *(in_addr *) (ptr + 16);
    *(in_addr *) & arptable[ARP_SERVER].ipaddr = *(in_addr *) (ptr + 20);
    *(unsigned long *) &(nic_macaddr[0]) = *(unsigned long *) (ptr + 28);
    *(unsigned short *) &(nic_macaddr[4]) = *(unsigned short *) (ptr + 32);

#ifdef DEBUG
    printf ("Server Name       : %s\n", ptr + 34);
    printf ("Filename          : %s\n", ptr + 108);
    printf ("Magic             : %x\n", *(unsigned long *) (ptr + 236));
#endif
    // guest basedir using ptr + 108, aka "filename"
    // filename max len is 128
    cmp = ptr + 108;

    // to obtain the basename, we reverse count from ptr + 235 to ptr + 109
    // if we found a '/', we replace it by a \0 to terminate the string
    // ending to 109, not 108, this way first '/' will never be replaced (to prevent NULL basename)
    {
        int i;

        for (i = 235; i >= 109; i--) {
            if (*(ptr + i) == '/' ) {
                *(ptr + i) = 0;
                if (i >= 109 + 4) {
                    // special case to ensure back compatibility with older LRS installations :
                    // if the basedir finished by '/bin', also strip it
                    if (*(ptr + i - 4) == '/'
                        &&
                        *(ptr + i - 3) == 'b'
                        &&
                        *(ptr + i - 2) == 'i'
                        &&
                        *(ptr + i - 1) == 'n'
                    ) {
                        *(ptr + i - 4) = 0;
                    }
                }
                break;
            }
        }
    }

    strcpy(basedir, ptr + 108);

    parse_dhcp_options(ptr + 240);

#if 0
    toto.Packet = 3;
    toto.BuffSize = 0;
    toto.BuffSeg = 0;
    toto.BuffOff = 0;
    toto.BuffLimit = 2048;
    ret = pxe_call (0x71, &toto);

#ifdef DEBUG
    printf ("Getting Packet type 3 : ");
    printf ("Call : %d (", ret);
    printf ("Status  : %d)\n", toto.Status);
#endif

    ptr = (unsigned char *) ((toto.BuffSeg << 4) + toto.BuffOff);
/*
    memmove((char*) &arptable[ARP_CLIENT].ipaddr,ptr+16,sizeof(in_addr));
    memmove((char*) &arptable[ARP_SERVER].ipaddr,ptr+20,sizeof(in_addr));
    memmove((char*) &nic_macaddr,ptr+28,6);
*/

    printf ("Server Name : %s\n", ptr + 34);
    printf ("Filename    : %s\n", ptr + 108);
    printf ("Magic       : %x\n", *(unsigned long *) (ptr + 236));

    parse_dhcp_options (ptr + 240);
#endif

    network_ready = 1;
    return 1;
}

extern int rarp
P ((void))
{
  printf ("rarp () : non implemented\n");
  network_ready = 1;
  return 1;
}

static const char broadcast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

extern void print_network_configuration
P ((void))
{

  static void sprint_ip_addr (char *buf, unsigned long addr)
  {
    sprintf (buf, "%d.%d.%d.%d",
             addr & 0xFF, (addr >> 8) & 0xFF,
             (addr >> 16) & 0xFF, addr >> 24);
  }

  if (!pxe_detected)
    printf ("No ethernet card found.\n");
  else if (!network_ready)
    printf ("Not initialized yet.\n");
  else
    {
      char me[16], my_mask[16], server[16], gw[16];

      sprint_ip_addr (me, arptable[ARP_CLIENT].ipaddr.s_addr);
      sprint_ip_addr (my_mask, netmask);
      sprint_ip_addr (server, arptable[ARP_SERVER].ipaddr.s_addr);
      sprint_ip_addr (gw, arptable[ARP_GATEWAY].ipaddr.s_addr);

      grub_printf ("Address        : %s\n", me);
      grub_printf ("Netmask        : %s\n", my_mask);
      grub_printf ("Server         : %s\n", server);
      grub_printf ("Gateway        : %s\n", gw);
    }
}

extern int arp_server_override
P ((const char *buf))
{
  printf ("Trying to override TFTPserver IP\n");
  return 1;
}

/* Mount the network drive. If the drive is ready, return one, otherwise
 *    return zero.  */
int
tftp_mount (void)
{
  //printf ("tftp_mount () : ");

  /* Check if the current drive is the network drive.  */
  if (current_drive != NETWORK_DRIVE)
    return 0;

  /* If the drive is not initialized yet, abort.  */
  if (!network_ready)
    return 0;

  //printf("ok\n");

  return 1;
}

#if 1
#define BUFFERSIZE 32768-2048
#define TFTPSIZE 512
#else
#define BUFFERSIZE 31504
#define TFTPSIZE 1432
#endif

unsigned char filename[128];

unsigned char *buffer = (unsigned char *) FSYS_BUF;

int bufferend;
int endoffile;
unsigned long savedpos;
unsigned long basepos;

int blksize = TFTPSIZE;

int tftpopen (void);
int tftpget (char *addr, int size);

#ifdef USE_PXE_TFTP

typedef struct s_tftp_open
{
  short Status;
  unsigned char ServerIP[4];
  unsigned char GatewayIP[4];
  unsigned char FileName[128];
  unsigned char UDP_Port[2];
  unsigned char PacketSize[2];
}
t_tftp_open;

typedef struct s_mtftp_read
{
  short Status;
  unsigned char FileName[128];
  unsigned long BufferSize;
  unsigned long Buffer;
  unsigned char ServerIP[4];
  unsigned char GatewayIP[4];
  unsigned char McastIP[4];
  unsigned char TFTPClntPort[2];
  unsigned char TFTPSrvPort[2];
  unsigned short TFTPOpenTimeOut;
  unsigned short TFTPReopenDelay;
}
t_mtftp_read;

t_tftp_open tftp_open;

int
tftpopen ()
{
  int ret;

  memmove ((char *) tftp_open.ServerIP, (char *) &arptable[ARP_SERVER].ipaddr,
           sizeof (in_addr));
  memmove ((char *) tftp_open.GatewayIP,
           (char *) &arptable[ARP_GATEWAY].ipaddr, sizeof (in_addr));
  memmove ((char *) tftp_open.FileName, filename, 128);
  tftp_open.UDP_Port[0] = 0;
  tftp_open.UDP_Port[1] = 69;
  tftp_open.PacketSize[0] = (TFTPSIZE & 255);
  tftp_open.PacketSize[1] = (TFTPSIZE >> 8);

  ret = pxe_call (0x20, &tftp_open);
#ifdef DEBUG_PXE_OPEN
  printf ("Open Call : %d for %s (", ret, filename);
  printf ("Status  : %d   ", tftp_open.Status);
  printf ("NegSize : %d %d)\n", tftp_open.PacketSize[0],
          tftp_open.PacketSize[1]);
  getkey ();
#endif
  if ((tftp_open.PacketSize[0] != (TFTPSIZE & 255)) ||
      (tftp_open.PacketSize[1] != (TFTPSIZE >> 8)))
    {
      blksize = (256 * tftp_open.PacketSize[1]) + tftp_open.PacketSize[0];
      printf ("Neg size : %d\n", blksize);
    }

  basepos = 0;
  bufferend = 0;
  endoffile = 0;
  savedpos = 0;

  return ret;
}

int
tftpget (char *addr, int size)
{
  typedef struct s
  {
    short Status;
    unsigned short PacketNumber;
    unsigned short BufferSize;
    unsigned short BuffOff;
    unsigned short BuffSeg;
  }
  tftp_r;

  tftp_r toto;
  int first = 1;
  int len = 0, ret;

  while (size != 0)
    {
      toto.BuffOff = ((unsigned long) addr) & 15;
      toto.BuffSeg = (unsigned short) (((unsigned long) addr) >> 4);
      ret = pxe_call (0x22, &toto);
#ifdef DEBUG
      printf ("Call : %d (", ret);
      printf ("Status : %d   ", toto.Status);
      printf ("Size   : %d)\n", toto.BufferSize);
      printf (".");
      if (toto.Status != 0)
        printf ("Problem : %d\n", toto.Status);
#endif
      size -= toto.BufferSize;
      addr += toto.BufferSize;
      len += toto.BufferSize;
      if (first)
        basepos = blksize * (toto.PacketNumber - 1);
      first = 0;
      if (toto.BufferSize != blksize)
        {
          endoffile = 1;
          return len;
        }
    }

  return len;
}
#else
int iport = 4567;
unsigned int nextpacket;

int
tftpopen (void)
{
  int ret, len = 2;
  char str[200];
  const char strappend[] = "octet";

  if (endoffile == 0)
    tftpclose ();

  udp_init ();
#ifdef DEBUG
  printf("In tftpopen (%s)", filename);
#endif
  str[0] = 0;
  str[1] = 1;
  memmove (str + len, filename, strlen (filename));
  len += strlen (filename);
  str[len++] = 0;

  memmove (str + len, strappend, strlen (strappend));
  len += strlen (strappend);
  str[len++] = 0;

  ret = udp_send (str, len, ++iport, 69);

  basepos = 0;
  bufferend = 0;
  endoffile = 0;
  savedpos = 0;
  nextpacket = 0;

  return 1;
}

#define TFTPBUFFER 516

int s_port;

void
senderr (void)
{
  char nack[] = "1234Abort";

  nack[0] = 0;
  nack[1] = 5;
  nack[2] = 0;
  nack[3] = 0;
  udp_send (nack, 10, iport, s_port);
}

int
tftpget (char *buff, int len)
{
  unsigned char tftpbuff[TFTPBUFFER];
  char ack[4];
  int siz = 0, size = 0, first = 1;
  int ret, to, timeout;

  while (len > 0)
    {
      to = getrtsecs ();
      timeout = 0;
      do
        {
          ret = udp_get (tftpbuff, &size, iport, &s_port);
          if (getrtsecs () != to)
            {
              timeout++;
              to = getrtsecs ();
            }
        }
      while ((ret != 0) && (timeout < 10));

      if (ret != 0)
        {
          printf ("Timeout in File TFTP OPEN...\n");
          senderr ();
          return -1;
        }                       //Timeout
#ifdef DEBUG
      printf ("Returned size = %d\n", size);
      getkey();
#endif

      size -= 4;
// DLINK Etherboot bug ?
      if (size > 512)
        size = 512;

#define SWAP(a) ((a)>>8)+((a&255)<<8)

#ifdef DEBUG
      printf ("Data : %d , Packet : %d\n",
              SWAP (*(unsigned short *) tftpbuff),
              SWAP (*(unsigned short *) (tftpbuff + 2)));
#endif
      if ((tftpbuff[0] != 0) || (tftpbuff[1] != 3))
        {
          printf ("In File TFTP OPEN : got a non-data packet\n");
          senderr ();
          return -1;
        }                       //Not a DATA Packet...

      // duplicate packet
      if (SWAP (*(unsigned short *) (tftpbuff + 2)) == nextpacket)
        {
          printf ("In File TFTP OPEN : duplicate data packet \n");
          // ack and ignore
          *(unsigned short *) (ack + 2) = *(unsigned short *) (tftpbuff + 2);
          ack[0] = 0;
          ack[1] = 4;
          ret = udp_send (ack, 4, iport, s_port);
          continue;
        }

      if (SWAP (*(unsigned short *) (tftpbuff + 2)) != (nextpacket + 1))
        {
          printf ("In File TFTP OPEN : packet number not correct\n");
          senderr ();
          return -1;
        }
      nextpacket = SWAP (*(unsigned short *) (tftpbuff + 2));

      memmove (buff, tftpbuff + 4, size);
      buff += size;
      len -= size;
      siz += size;
#ifdef DEBUG
      printf ("*** %d %d %d %d\n", *tftpbuff,
              *(tftpbuff + 1), *(tftpbuff + 2), *(tftpbuff + 3));
#endif
      *(unsigned short *) (ack + 2) = *(unsigned short *) (tftpbuff + 2);
      ack[0] = 0;
      ack[1] = 4;
      ret = udp_send (ack, 4, iport, s_port);
      if (first)
        basepos = 512 * (SWAP (*(unsigned short *) (ack + 2)) - 1);
      first = 0;
      if (size != 512)
        {
          endoffile = 1;
          return siz;
        }
    }

  return siz;
}

int
tftpclose (void)
{
  char str[100];
  const char errappend[] = "TFTP Aborted";
  int ret, len = 4;

  if (!endoffile)
    {
      str[0] = 0;
      str[1] = 5;
      str[2] = 0;
      str[3] = 0;
      memmove (str + len, errappend, strlen (errappend));
      len += strlen (errappend);
      str[len++] = 0;

      ret = udp_send (str, len, iport, s_port);
      endoffile = 1;
    }

  udp_close ();
  return 0;
}
#endif

/* Read up to SIZE bytes, returned in ADDR.  */
int
tftp_read (char *addr, int size)
{
  int len = 0, remain, pos;

  char *save_addr;
  int save_size;
  int save_pos;

  save_addr = addr;
  save_size = size;
  save_pos = filepos;
retransmit:
  addr = save_addr;
  size = save_size;
  filepos = save_pos;
#ifdef DEBUG
  printf ("Trying to get %d bytes (@0x%x) offset %d\n", size, addr, filepos);
#endif

  if (filepos < savedpos)
    {
      if (basepos <= filepos)
        {
#ifdef DEBUG
          printf ("No reopen needed : basepos=%d\n", basepos);
#endif
          // bufferend=; (no change in buffer end)
          goto next;
        }
#ifdef DEBUG
      printf ("Re-opening file @pos %d (I was @ %d, basepos =%d)\n", filepos,
              savedpos, basepos);
#endif
      tftp_close ();
      tftpopen ();
      if (savedpos < filepos)
        {
          do
            {
              if (tftpget (buffer, BUFFERSIZE) < 0)
                {
                  tftp_close ();
                  tftpopen ();
                  goto retransmit;
                }
              savedpos += BUFFERSIZE;
            }
          while (savedpos <= filepos);
          basepos = savedpos - BUFFERSIZE;
        }
#ifdef DEBUG
      printf ("Rewinded to @ [%d..%d]\n", basepos, savedpos - 1);
#endif
    }

next:
  savedpos = filepos + size;

  if (!endoffile)
    {
      do
        {
          remain = bufferend - (filepos - basepos);
          pos = (filepos - basepos);
          if (pos > BUFFERSIZE)
            {

              printf ("Error : skipping : %d\n", pos);
              return 0;
            }

          if (remain >= size)
            {
#ifdef DEBUG
              printf ("Returning after copying : %d\n", size);
#endif
              memmove (addr, buffer + pos, size);
              filepos += size;
              len += size;
              return len;
            }
          if (remain > 0)
            {
#ifdef DEBUG
              printf ("Copying : %d\n", remain);
#endif
              memmove (addr, buffer + pos, remain);
              addr += remain;
              size -= remain;
              filepos += remain;
              len += remain;
            }

          bufferend = tftpget (buffer, BUFFERSIZE);
          if (bufferend == -1)
            {
              tftp_close ();
              tftpopen ();
              goto retransmit;
            }
        }
      while (bufferend == BUFFERSIZE);

#ifdef DEBUG
      printf ("Reached EOF\n");
#endif
    }

  remain = bufferend - (filepos - basepos);
  pos = (filepos - basepos);

  if (size > remain)
    size = remain;

  if (remain > 0)
    {
#ifdef DEBUG
      printf ("EOF Returning after copying : %d\n", size);
#endif
      memmove (addr, buffer + pos, size);
      filepos += size;
      len += size;
      return len;
    }
  return 0;
}

typedef struct s
{
  short Status;
  unsigned char ServerIP[4];
  unsigned char GatewayIP[4];
  unsigned char FileName[128];
  unsigned char FileSize[4];
}
getsize;

getsize t_dir;

long new_tftpdir (char *filename) {
  char str[1000], *ptr;
  int res;
  const char strappend[] = "octet\000tsize\0000";
  int ret, len = 2, timeout = 0, sec;

  int maxtimeout = 3600;        /* 1 hour timeout */

  udp_init ();

  endoffile = 0;

  str[0] = 0;
  str[1] = 1;
  memmove (str + len, filename, strlen (filename));
  len += strlen (filename);
  str[len++] = 0;

  memmove (str + len, strappend, 13);
  len += 13;
  str[len++] = 0;

  sec = getrtsecs ();
  do
    {
      if (sec != getrtsecs ())
        {
          if ((timeout % 2) == 0)
            {
              /* Resend every 2 seconds */
              if (timeout > 0) printf (".");
              ret = udp_send (str, len, ++iport, 69);
            }
          sec = getrtsecs ();
          timeout++;
        }
      if (!udp_get (str, &len, iport, &s_port))
        {
          break;
        }
    }
  while (timeout < maxtimeout);

  tftpclose ();

  if (timeout == maxtimeout)
    return -2;
  if (str[1] == 0x05)
    return -1;

  ptr = str + 8;
  safe_parse_maxint (&ptr, &res);
#if DEBUG
  printf ("Tsize returned : %d\n", res);
#endif

  return res;
}

/* Check if the file DIRNAME really exists. Get the size and save it in
 *    FILEMAX.  */
int
tftp_dir (char *dirname)
{
  unsigned long l;
  long ret;

#ifdef DEBUG
  printf ("TFTPDIR : \n");
#endif
  if (print_possibilities)
    return 1;

  nul_terminate (dirname);
#ifdef DEBUG
  printf ("TFTPDIR : %s\n", dirname);
#endif

#if 0
  *(in_addr *) t_dir.ServerIP = *(in_addr *) & arptable[ARP_SERVER].ipaddr;
  *(in_addr *) t_dir.GatewayIP = *(in_addr *) & arptable[ARP_GATEWAY].ipaddr;
  memmove ((char *) t_dir.FileName, dirname, 128);
#ifdef DEBUG
#ifdef DEBUG_PXE_STRUCT
  dump_pxe_struct ();
#endif
  printf ("TFTPDIR : before call (struct size : %d)\n", sizeof (t_dir));
#endif
  ret = pxe_call (0x25, &t_dir);
#ifdef DEBUG
  printf ("Call (tftp dir returned : %d, ", ret);
  printf ("Status : %d)\n", t_dir.Status);
#endif

  filemax = MAXINT;
  if (t_dir.Status != 0)
    {
      errnum = ERR_FILE_NOT_FOUND;
      return 0;
    }

  l =
    t_dir.FileSize[0] + (t_dir.FileSize[1] << 8) + (t_dir.FileSize[2] << 16) +
    (t_dir.FileSize[3] << 24);
#else
  ret = new_tftpdir (dirname);

#ifdef DEBUG
  printf ("Call (tftp dir %s returned : %d, ", dirname, ret);
#endif
  if (ret < 0)
    {
      errnum = (ret == -1) ? ERR_FILE_NOT_FOUND : ERR_BOOT_FAILURE;
      return 0;
    }
  l = ret;
#endif
  memmove (filename, dirname, 128);
  ret = tftpopen ();

#ifdef DEBUG
  printf ("tftp_dir (%s) : returned : %d\n", dirname, l);
#endif

  filemax = l;

  return 1;
}

/* Close the file.  */
void
tftp_close (void)
{
#ifdef USE_PXE_TFTP
  typedef struct s
  {
    short Status;
  }
  close;

  close toto;
  int ret;

  ret = pxe_call (0x21, &toto);
#ifdef DEBUG
  printf ("tftp_close () : ");
  printf ("Call : %d  ", ret);
  printf ("Status : %d\n", toto.Status);
  getkey ();
#endif
#else
#ifdef DEBUG
  printf ("tftp_close ()\n");
#endif
  tftpclose ();
  errnum = ERR_NONE;
#endif
}

void
pxe_unload (void)
{
  typedef struct s
  {
    short Status;
    char t[10];
  }
  close;

  close toto;
  int ret;

  /* tell Pulse 2 that we have restored the partition */
  if (imgname[0] != '\0') {
    char name[64];

    udp_init ();
    grub_sprintf(name, "L3-%s", imgname);
    udp_send_to_pulse2 (name, grub_strlen(name));
    udp_close ();
  }

  toto.Status = 0;
  ret = pxe_call (0x31, &toto); //UDP CLOSE
#ifndef QUIET
  printf ("PXE UDP Close : %d (%d)\n", ret, toto.Status);
#endif
  ret = pxe_call (0x5, &toto);  //UNDI SHUTDOWN
#ifndef QUIET
  printf ("PXE UNDI Shutdown : %d (%d)\n", ret, toto.Status);
#endif
  ret = pxe_call (0x70, &toto); //UNLOAD STACK
#ifndef QUIET
  printf ("PXE Unload stack : %d (%d)\n", ret, toto.Status);
#endif
  if (pxev2)
    {
      ret = pxe_call (0x15, &toto);     //UNDI STOP
#ifndef QUIET
      printf ("PXE UNDI Stop : %d (%d)\n", ret, toto.Status);
#endif
    }
  else
    {
      /* Old API <2 */
      ret = pxe_call (0x2, &toto);      //UNDI CLEANUP
#ifndef QUIET
      printf ("PXE UNDI Cleanup : %d (%d)\n", ret, toto.Status);
#endif
    }
/* new_api_unload: PXENV_UDP_CLOSE 31, PXENV_UNDI_SHUTDOWN 5,
 PXENV_UNLOAD_STACK 70,
PXENV_STOP_UNDI 15, (new)
PXENV_UNDI_CLEANUP 02, (old api <2)
*/

}

static unsigned short
ipchksum (unsigned short *ip, int len)
{
  unsigned long sum = 0;
  len >>= 1;
  while (len--)
    {
      sum += *(ip++);
      if (sum > 0xFFFF)
        sum -= 0xFFFF;
    }
  return ((~sum) & 0x0000FFFF);
}

static void
arp_update (unsigned long *ip, unsigned char *mac)
{
  int i;

#ifdef DEBUG
  printf ("ARP update request for %X, MAC is %X %X %X\n", *ip,
          *(unsigned short *) (mac + 0), *(unsigned short *) (mac + 2),
          *(unsigned short *) (mac + 4));
#endif
  for (i = 0; i < MAX_ARP; i++)
    if (*(unsigned long *) &(arptable[i].ipaddr.s_addr) == *ip)
      {
        *(unsigned long *) &(arptable[i].node[0]) =
          *(unsigned long *) (mac + 0);
        *(unsigned short *) &(arptable[i].node[4]) =
          *(unsigned short *) (mac + 4);
        return;
      }
}

void
arp_request (unsigned long ip)
{
  struct arprequest arpreq;
  unsigned char broadcast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  int retry, timeout;

  int (*getudp) (struct nic * nic);
  int (*send) (struct nic * nic, const char *d, unsigned int t,
               unsigned int s, const char *p);

  getudp = *(int (*)()) (nic->poll);
  send = *(int (*)()) (nic->transmit);

  arpreq.hwtype = 0x0100;       // Ethernet = 0x0001 (byte swapped)
  arpreq.protocol = IP;
  arpreq.hwlen = 6;
  arpreq.protolen = 4;
  arpreq.opcode = ARP_REQ;

  *(unsigned long *) (arpreq.shwaddr + 0) =
    *(unsigned long *) (arptable[ARP_CLIENT].node + 0);
  *(unsigned short *) (arpreq.shwaddr + 4) =
    *(unsigned short *) (arptable[ARP_CLIENT].node + 4);

  *(unsigned long *) (arpreq.thwaddr + 0) = 0;
  *(unsigned short *) (arpreq.thwaddr + 4) = 0;

  *(unsigned long *) arpreq.sipaddr = arptable[ARP_CLIENT].ipaddr.s_addr;
  *(unsigned long *) arpreq.tipaddr = ip;

  for (retry = 1; retry <= MAX_ARP_RETRIES; retry++)
    {
      //printf ("ARP Request : %d\n", retry);
      (*send) (nic, broadcast, ARP_TYPE, sizeof (arpreq),
               (const char *) &arpreq);
      timeout = getrtsecs ();
      // Wait for Reply
      do
        {
          if ((*getudp) (nic))
            {                   //printf("Got Something...\n");

              if ((nic->packet[12] == 0x08) && (nic->packet[13] == 0x06) &&
                  (nic->packet[20] == 0x00) && (nic->packet[21] == 0x02))
                {
                  arp_update ((unsigned long *) &(nic->packet[28]),
                              (unsigned char *) &(nic->packet[22]));
                  return;
                }

              if ((nic->packet[12] == 0x08) && (nic->packet[13] == 0x06) &&
                  (nic->packet[20] == 0x00) && (nic->packet[21] == 0x01))
                {
                  arp_update ((unsigned long *) &(nic->packet[28]),
                              (unsigned char *) &(nic->packet[22]));
                  return;
                }
            }
        }
      while (getrtsecs () == timeout);
    }
  printf ("ARP Request failed...\n");
}

static unsigned short
htons (unsigned short a)
{
  return ((a & 0xFF00) >> 8) | ((a & 0x00FF) << 8);
}

int
pxe_oeb (int func, void *packet)
{
  if (func == 0x71)
    {
      typedef struct s
      {
        short Status;
        unsigned short Packet;
        unsigned short BuffSize;
        unsigned short BuffOff;
        unsigned short BuffSeg;
        unsigned short BuffLimit;
      }
      cache;

      cache *ptr = (cache *) packet;

      ptr->Status = 0;
      ptr->BuffSize = 1500;
      ptr->BuffOff = (unsigned long) PXEEntry & 15;
      ptr->BuffSeg = (unsigned long) PXEEntry >> 4;
      return 0;
    }

  if (func == 0x30)
    {
      if (udp_open)
        {
          *(unsigned short *) packet = 1;
          return 1;
        }
      *(unsigned short *) packet = 0;
      return 0;
    }

  if (func == 0x31)
    {
      if (udp_open)
        {
          *(unsigned short *) packet = 0;
          return 0;
        }
      *(unsigned short *) packet = 1;
      return 1;
    }

  if (func == 0x32)
    {
      int res, (*getudp) (struct nic * nic);
      int (*send) (struct nic * nic, const char *d, unsigned int t,
                   unsigned int s, const char *p);

      typedef struct s_udp_r_s
      {
        short Status;
        unsigned char src_ip[4];
        unsigned char dst_ip[4];
        unsigned short src_port;
        unsigned short dst_port;
        short buffer_size;
        short BuffOff;
        short BuffSeg;
      }
      udp_r_s;

      udp_r_s *ptr = (udp_r_s *) packet;

      getudp = *(int (*)()) (nic->poll);
      send = *(int (*)()) (nic->transmit);

      res = (*getudp) (nic);

      if (res == 0)
        {
          *(unsigned short *) packet = 1;
          return 1;
        }

      if ((*(unsigned short *) &(nic->packet[12]) == IP)        /* IP */
          && (nic->packet[14] == 0x45) && (nic->packet[23] == UDP)      /* Lg IP=std & UDP */
          && (*(unsigned long *) (&(nic->packet[30])) == *(unsigned long *) ptr->dst_ip))       /* dest=CLIENT */
        {
          int i;
          if (ptr->dst_port != 0)
            if (ptr->dst_port != *(unsigned short *) (&(nic->packet[36])))
              {
#ifdef DEBUG
                printf ("Input port not matching : discard...\n");
#endif
                *(unsigned short *) packet = 1;
                return 1;
              }
          ptr->buffer_size = nic->packetlen - 42;
          for (i = 0; i < ptr->buffer_size; i++)
            *(unsigned char *) ((ptr->BuffSeg << 4) + ptr->BuffOff + i) =
              nic->packet[i + 42];
          *(unsigned long *) &(ptr->src_ip) =
            *(unsigned long *) (nic->packet + 26);
          ptr->src_port = *(unsigned short *) (nic->packet + 34);
#ifdef DEBUG
          printf ("PTR DST : %x\n", *(unsigned long *) (ptr->dst_ip));
          printf ("PKT DST : %x\n", *(unsigned long *) (&(nic->packet[30])));
          printf ("PKT DPT : %x\n", *(unsigned short *) (&(nic->packet[34])));
          printf ("PKT SPT : %x\n", *(unsigned short *) (&(nic->packet[36])));
#endif
          *(unsigned short *) packet = 0;
          return 0;
        }

      if ((*(unsigned short *) &(nic->packet[12]) == ARP)       /* ARP */
          && (*(unsigned short *) &(nic->packet[20]) == ARP_REQ)        /* ARP Request */
          && (*(unsigned long *) (&(nic->packet[38])) == *(unsigned long *) ptr->dst_ip))       /* for ME ? */
        {
          // COPY DST MAC
          *(unsigned long *) &(nic->packet[32]) =
            *(unsigned long *) &(nic->packet[22]);
          *(unsigned short *) &(nic->packet[36]) =
            *(unsigned short *) &(nic->packet[26]);
          // COPY DST IP
          *(unsigned long *) &(nic->packet[38]) =
            *(unsigned long *) &(nic->packet[28]);

          arp_update ((unsigned long *) &(nic->packet[28]),
                      (unsigned char *) &(nic->packet[22]));

          // COPY SRC IP
          *(unsigned long *) &(nic->packet[28]) =
            *(unsigned long *) ptr->dst_ip;
          // FILL THE MISSING MAC FIELD = MY MAC ADDR
          *(unsigned long *) &(nic->packet[22]) =
            *(unsigned long *) &(nic_macaddr[0]);
          *(unsigned short *) &(nic->packet[26]) =
            *(unsigned short *) &(nic_macaddr[4]);
          // Request -> Reply
          *(unsigned short *) &(nic->packet[20]) = ARP_REPLY;   // Byte swapped !!!
          send (nic, (unsigned char *) (nic->packet + 32), ARP_TYPE, 28,
                nic->packet + 14);
#ifdef DEBUG
          printf ("Sent Reply to ARP Request\n");
#endif
          *(unsigned short *) packet = 1;
          return 1;
        }
#ifdef DEBUG
      printf ("Got a packet : %x\n", nic->packetlen);
      {
        int i;
        for (i = 0; i < nic->packetlen; i++)
          {
            printf ("%x ", (unsigned char) nic->packet[i]);
            if ((i & 15) == 15)
              printf ("\n");
          }
        printf ("\n");
      }
#endif
      *(unsigned short *) packet = 1;
      return 1;
    }

  if (func == 0x33)
    {
      struct iphdr *ip;
      struct udphdr *udp;
      int i, (*send) (struct nic * nic, const char *d, unsigned int t,
                      unsigned int s, const char *p);
      typedef struct s_udp_w_s
      {
        short Status;
        unsigned char ip[4];
        unsigned char gw[4];
        unsigned short src_port;
        unsigned short dst_port;
        short buffer_size;
        short BuffOff;
        short BuffSeg;
      }
      udp_w_s;

      udp_w_s *ptr = (udp_w_s *) packet;
      unsigned char *buf = (unsigned char *) 0x4800;
      unsigned char *destmac = NULL;
      unsigned int gwip;        /* gateway ip */
      int samesubnet;

      send = *(int (*)()) (nic->transmit);

      ip = (struct iphdr *) buf;
      udp = (struct udphdr *) ((char *) buf + sizeof (struct iphdr));
      ip->verhdrlen = 0x45;
      ip->service = 0;
      ip->len =
        htons (sizeof (struct iphdr) + sizeof (struct udphdr) +
               ptr->buffer_size);
      ip->ident = 0;
      ip->frags = 0;
      ip->ttl = 60;
      ip->protocol = UDP;
      ip->chksum = 0;
      ip->src.s_addr = arptable[ARP_CLIENT].ipaddr.s_addr;
      ip->dest.s_addr = *(unsigned long *) ptr->ip;
      ip->chksum = ipchksum ((unsigned short *) buf, sizeof (struct iphdr));

      udp->src = ptr->src_port;
      udp->dest = ptr->dst_port;
      udp->len = htons (ptr->buffer_size + sizeof (struct udphdr));
      udp->chksum = 0;

      for (i = 0; i < ptr->buffer_size; i++)
        buf[i + sizeof (struct iphdr) + sizeof (struct udphdr)] =
          *(unsigned char *) ((ptr->BuffSeg << 4) + ptr->BuffOff + i);

      /* Client and server on the same subnet ? */
      samesubnet = 1;
      if ((*(unsigned long *) ptr->ip & netmask) !=
          (arptable[ARP_CLIENT].ipaddr.s_addr & netmask))
        {
          samesubnet = 0;
        }
      /* Not on the same subnet -> go through the gateway */
      if (samesubnet)
        gwip = *(unsigned long *) ptr->ip;
      else
        gwip = *(unsigned long *) ptr->gw;

    reget_arp:
      for (i = 0; i < MAX_ARP; i++)
        {
          if (*(unsigned long *) &(arptable[i].ipaddr.s_addr) == gwip)
            {
              destmac = (unsigned char *) &(arptable[i].node);
              break;
            }
        }
      if (destmac == NULL)
        {
          printf ("No entry for IP : %X in arptable\n", gwip);
          arp_request (gwip);
          goto reget_arp;
        }
      if (!
          (destmac[0] | destmac[1] | destmac[2] | destmac[3] | destmac[4] |
           destmac[5]))
        {
          //printf ("MAC Adress for IP : %X is blank...\n", gwip);
          arp_request (gwip);
          goto reget_arp;
        }
#ifdef DEBUG
      printf ("IP : %X -> MAC :%X %X %X\n", *(unsigned long *) ptr->ip,
              *(unsigned short *) (&destmac[0]),
              *(unsigned short *) (&destmac[2]),
              *(unsigned short *) (&destmac[4]));
      printf ("Sending a packet of length : %d\n", ptr->buffer_size);
#endif


      (*send) (nic, destmac, IP_TYPE,
               ptr->buffer_size + sizeof (struct iphdr) +
               sizeof (struct udphdr), buf);
#ifdef DEBUG
      printf ("Packet sent.\n");
#endif
      *(unsigned short *) packet = 1;
      return 1;
    }

    if (func == 0x70)
    {
        return 0;
    }
    if (func == 0x5)
    {
        return 0;
    }
    if (func == 0x2)
    {
        return 0;
    }
    if (func == 0x15)
    {
    }

    printf ("Pxe emulation : %x %x\n", func, packet);

    getkey ();
    *(unsigned short *) packet = 1;
    return 1;
}

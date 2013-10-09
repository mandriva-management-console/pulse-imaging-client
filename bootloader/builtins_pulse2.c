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

#include <shared.h>
#include <filesys.h>
#include <term.h>
#include "pci.h"
#include "etherboot.h"
#include "deffunc.h"
#include "builtins_pulse2.h"
#include "builtins_pulse_txt.h"
#ifdef SUPPORT_NETBOOT
#include "zlib.h"
#define INBUFF 8192
#define OUTBUFF 24064
unsigned char *zcalloc(long toto, int nb, int size);
void zcfree(long toto, unsigned char *zone);
#include "pxe.h"
#endif

/* ugly global vars */
int fat, ntfs, files, new_len, new_type;

/* auto fill mode ?! */
int auto_fill = 1;
/* variable global pour signaler si modification du timeout du menu
si cette variable est a 1 le timeout menu est 0
*/
int modifietimeout=0;
/* total/current bytes of data to decompress */
unsigned int total_kbytes = 0;
unsigned int current_bytes = 0;

/* default = cannot access to grub's command line */
int nosecurity = 0;

/* current image name being restored */
char imgname[32];

/* proto*/
int new_get(char *file, int sect, int *endsect, int table);

/* to force a new inventory */
extern int done_inventory;

void sendACK(int block, int from, int port) {
    unsigned char buffer[4];

    buffer[0] = 0;
    buffer[1] = 4;
    buffer[2] = block >> 8;
    buffer[3] = block & 0xFF;
    udp_send((char *)buffer, 4, from, port);
}

void sendERR(const  char *str, int from, int port) {
    unsigned char buffer[40], i;

    buffer[0] = 0;
    buffer[1] = 5;
    buffer[2] = 0;
    buffer[3] = 123;
    for (i = 0; i <= strlen(str); i++)
        buffer[4 + i] = str[i];
    udp_send((char *)buffer, 4 + strlen(str) + 1, from, port);
}

#define TFTPBLOCK 1456
#define BLKSIZE "1456"
//#define TFTPBLOCK 2000
//#define BLKSIZE "2000"

typedef struct diskbuffer_ {
//              unsigned char data[OUTBUFF];    -> Direct to BUFFERADDR
    int full;
    int size;
} diskbuffer;

typedef struct tftpbuffer_ {
    unsigned char udp[4];
    unsigned char data[TFTPBLOCK + 16];
    int length;                 // 0 if available
    struct tftpbuffer_ *next;
} tftpbuffer;

#define NB_BUF 10

int get_empty(tftpbuffer * buff, int *i) {
    int j;

    for (j = 0; j < NB_BUF; j++)
        if (buff[j].length == 0) {
            *i = j;
            return 1;
        }

    return 0;
}

int tot_lg(tftpbuffer * buff) {
    int i = 0;

    do {
        i += buff->length;
        if (buff->next)
            buff = buff->next;
        else
            return i;
    }
    while (1 == 1);
}

#define TFTPSTARTPORT 8192
int tftpport = TFTPSTARTPORT;

void inc(int *ptr, int *mask) {
    *mask <<= 1;
    if (*mask == 256) {
        *mask = 1;
        *ptr = *ptr + 1;
    }
}

int val(const char *string) {
    char *ptr, *cp=NULL;
    int i;

    ptr = (char *)BUFFERADDR;

    while (*ptr) {
        cp = ptr;
        i = 0;
        while (cp[i] == string[i]) {
            i++;
        }
        if ((string[i] == 0) && (cp[i] == '=')) {
            cp = cp + i + 1;
            break;
        }
        cp = NULL;
        ptr++;
    }

    if (cp) {
        safe_parse_maxint(&cp,(int *) &i);
        return i;
    }

    return -1;
}

int exist(const char *string) {
    char *ptr, *cp;
    int i;

    ptr = (char *)BUFFERADDR;

    while (*ptr) {
        cp = ptr;
        i = 0;
        while (cp[i] == string[i]) {
            i++;
        }
        if ((string[i] == 0) && (cp[i] == '=')) {
            return 1;
        }
        ptr++;
    }
    return 0;
}

/* inc */
int inc_func(char *arg, int flags) {
    /* not used anymore. now done in the initrd */

    return 0;
}

/*
*gere le password du menu
*/
int set_master_password_func(char *arg, int flags) 
{ // function call only if added on top of the bootmenu to enable password verification against imaging server
  int lasttime, time;
  #define TICKS_PER_SEC 20
  unsigned long ctime=0;
  int ii,i1,i2;
  char buffer[60];
  char buf[60];
  bzero((void *)buffer,sizeof(buffer));
  buffer[0]=0xAF;
  buffer[1]=':';
  graphics_cls();
  int size=0;
  int port;
  char *title_prompt;
  char *password_text;
  char *erreurmsg_text;
  char *ptlang=NULL;
  char c;
  char lang[8];
  bzero((void *)lang,sizeof(lang));
  ptlang = strstr (arg, "L=");
  int timeoutpwd = SEC_TO_WAIT; // chang tine wait in builtins_pulse2.h
  if (ptlang) 
  {
    strcpy (lang,ptlang+2);
  }else
  {
    strcpy (lang,DEFAULT_LANG); // Chang DEFAULT_LANG in builtins_pulse2.h
  } 
 
  init_page ();
  // affiche  message de demande de mot de passe
  
   if (strstr(lang, "fr_FR")) 
    {
        //kbdfr_func(NULL,0); //met le clavier en azerty
        title_prompt    =  PASSWD_PROMPT_FR(SEC_TO_WAIT);// chang text prompt in builtins_pulse2.h
	password_text   =  PASSWD_FR;
	erreurmsg_text   =  PASSWD_ERROR_FR;
    } else if (strstr(lang, "pt_BR")) 
    {
       title_prompt    =  PASSWD_PROMPT_PT(SEC_TO_WAIT);// chang text prompt in builtins_pulse2.h
       password_text   =  PASSWD_PT;
       erreurmsg_text   =  PASSWD_ERROR_PT;
    } else if (strstr(lang, "C")) 
    {
       title_prompt    =  PASSWD_PROMPT_EN(SEC_TO_WAIT);  // chang text prompt in builtins_pulse2
       password_text   =  PASSWD_EN;
       erreurmsg_text   =  PASSWD_ERROR_EN;
    }else
    {
       //kbdfr_func(NULL,0); //met le clavier en azerty
       title_prompt    =  PASSWD_PROMPT_FR(SEC_TO_WAIT);
       password_text   =  PASSWD_FR;
       erreurmsg_text   =  PASSWD_ERROR_FR;
    }
        
  
 printf("%s\n", title_prompt);
  
 lasttime = getrtsecs ();
 while(1)
 {// wait 10s touch key 
    if (lasttime != (time = getrtsecs ()))    // One sec has passed...
    {// show compteur timeout
      printf(".");
      lasttime = time;
      timeoutpwd--;
    }
    if ((checkkey () != -1))
    {
       c = translate_keycode (getkey ());
       if(c==27)
       {
	 //init_bios_info();//recharge le menu
	 grub_timeout=0;
         modifietimeout=1;
         show_menu = 0;     
         return 0;
       }
       break;
    }      
   if(!timeoutpwd) 
   {
     grub_timeout=0;
     modifietimeout=1;
     show_menu = 0;     
     return 0;
   }
 }
    bzero((void *)&buffer[2],sizeof(buffer));
    get_cmdline(password_text,(char*) &buffer[2] , MAX_MENU_PASSWD_SIZE, '*', 1);
    delay_func(1,"");
    // checking password by server
    udp_init();
    udp_send_withmac(buffer,strlen(buffer)+1,1001,1001);
    // Clearing buffer
    i1 = 0;
    ctime = currticks();
    while (1){
    if ( currticks() > ctime + 500 ) break;
    // Clearing input buffer
    ii = 0;
    //if (i1++ == 
    while (buf[ii]) { buf[ii++] = 0; }
    // Try to get server response
    udp_get(buf,&size,1001,&port);
    if (grub_strcmp (buf,"ok")==0)
    {
        // Close UDP and enter the bootmenu
        udp_close();
        return 0;
    }
    else if (grub_strcmp (buf,"ko")==0)
    {
        break;    
    }
    //delay_func(3,"");
    }
    // After tries, wrong password
    udp_close();
    delay_func(2,erreurmsg_text);
    grub_timeout=0;modifietimeout=1;
    show_menu = 0;
    return 0;
}

/* setdefault */
int setdefault_func(char *arg, int flags) {
    unsigned char buffer[] = " x\0";
    int i;

    safe_parse_maxint(&arg,(int*) &i);

    buffer[0] = 0xCD;
    buffer[1] = i & 255;

    udp_init();
    udp_send_withmac((char*)buffer, 3, 1001, 1001);
    udp_close();

    return 0;
}

/* nosecurity */
int nosecurity_func(char *arg, int flags) {
    nosecurity = 1;

    return 0;
}

void drive_info(unsigned char *buffer) {
    int i, err;
    struct geometry geom;

    unsigned long partition, start, len, offset, ext_offset;
    unsigned int type, entry;
    unsigned char *buf = (unsigned char *)SCRATCHADDR;
    unsigned char disk[] = "(hdX)";
    unsigned char bnum = *(unsigned char *)0x0475;

#ifndef QUIET
    grub_printf("BIOS drives number: %d\n", bnum);
#endif
    if (bnum == 0 || bnum > 32)
        bnum = 8;               /* for buggy bios */
    for (i = '0'; i <= '0' + bnum - 1; i++) {
        disk[3] = i;

        set_device((char*)disk);
        grub_memset(&geom, 0, sizeof(geom));
        err = get_diskinfo(current_drive, &geom);
        if (err)
            continue;
        /* verify that the disk is readable */
        if (biosdisk(BIOSDISK_READ, current_drive, &geom, 0, 1, SCRATCHADDR)) {
            /*     grub_printf ("Error reading the 1st sector on disk %d\n",
               current_drive);   */
            continue;
        }

        grub_sprintf((char*)buffer, "D:%s:CHS(%d,%d,%d)=%d\n", disk, geom.cylinders,
                     geom.heads, geom.sectors, geom.total_sectors);

#ifndef QUIET
        grub_printf("\t%s: %d MB\n", disk, geom.total_sectors / 2048);
#endif
        while (*buffer)
            buffer++;

        start = 0;
        type = 0;
        len = geom.total_sectors;
        partition = 0xFFFFFF;
        errnum = 0;

        while (next_partition
               (current_drive, 0xFFFFFF, &partition,(int *) &type, &start, &len,
                &offset,(int *) &entry, &ext_offset,(char *) buf)) {
            if (((type) && (type != 5) && (type != 0xf) && (type != 0x85))) {
                grub_sprintf((char*)buffer, "P:%d,t:%x,s:%d,l:%d\n",
                             (partition >> 16), type, start, len);
#ifndef QUIET
                grub_printf("\t\tP:%d, t:%x, s:%d, l:%d\n", (partition >> 16),
                            type, start, len);
#endif
                while (*buffer)
                    buffer++;
            }
        }
    }
}

/* partcopy START NAME_PREFIX [type] */
int partcopy_func(char *arg, int flags) {
    int new_start;
    /* int start_cl, start_ch, start_dh;
       int end_cl, end_ch, end_dh; */
    int entry;
    char *mbr = (char *)SCRATCHADDR, *path;
    static int sendlog = 1;

    int i, j, curr;
    unsigned long save_drive;
    char name[64];
    int old_perc;
    int l1 = 0, l2 = '0', l3 = '0';

    /* Convert a LBA address to a CHS address in the INT 13 format.  */
    auto void lba_to_chs(int lba, int *cl, int *ch, int *dh);
    void lba_to_chs(int lba, int *cl, int *ch, int *dh) {
        int cylinder, head, sector;

        sector = lba % buf_geom.sectors + 1;
        head = (lba / buf_geom.sectors) % buf_geom.heads;
        cylinder = lba / (buf_geom.sectors * buf_geom.heads);

        if (cylinder >= buf_geom.cylinders)
            cylinder = buf_geom.cylinders - 1;

        *cl = sector | ((cylinder & 0x300) >> 2);
        *ch = cylinder & 0xFF;
        *dh = head;
    }

    /* Get the drive and the partition.  */
    if (!set_device((char*)arg))
        return 1;

    /* The drive must be a hard disk.  */
    if (!(current_drive & 0x80)) {
        errnum = ERR_BAD_ARGUMENT;
        return 1;
    }

    /* The partition must a primary partition.
       if ((current_partition >> 16) > 3
       || (current_partition & 0xFFFF) != 0xFFFF)
       {
       errnum = ERR_BAD_ARGUMENT;
       return 1;
       }
     */

    entry = current_partition >> 16;

    save_drive = current_drive;

    /* update the disk geometry */
    get_diskinfo(current_drive, &buf_geom);
    if (biosdisk(BIOSDISK_READ, current_drive, &buf_geom, 0, 1, SCRATCHSEG)) {
        grub_printf("Error reading the 1st sector on disk %d\n", current_drive);
        //return 1;
    }
#ifdef DEBUG
    grub_printf("GEO : %d, %d, %d, %d E: %d DR: %d err=%d\n",
                buf_geom.total_sectors, buf_geom.cylinders, buf_geom.heads,
                buf_geom.sectors, entry, current_drive, 0);
#endif

    /* Get the new partition start.  */
    arg = skip_to(0, arg);

    if (entry <= 3) {
        // old code ? remove it ?
        if (grub_memcmp(arg, "-first", 6) == 0)
            new_start = buf_geom.sectors;
        else if (grub_memcmp(arg, "-next", 5) == 0)
            new_start =
                buf_geom.sectors *
                (((PC_SLICE_START(mbr, (entry - 1))) +
                  (PC_SLICE_LENGTH(mbr, (entry - 1))) + (buf_geom.sectors -
                                                         1)) /
                 buf_geom.sectors);
        else if (!safe_parse_maxint(&arg,(int*) &new_start))
            return 1;
    } else if (!safe_parse_maxint(&arg,(int*) &new_start))
        return 1;

    /* get file path */
    path = skip_to(0, arg);

    /* find the image name */
    for (i = grub_strlen(path); i > 0; i--) {
        if (path[i] == '/')
            break;
    }
    for (j = i - 1; j > 0; j--) {
        if (path[j] == '/')
            break;
    }
    grub_memmove(imgname, &path[j + 1], i - j);
    imgname[i - j - 1] = '\0';

    /* try to get image size */
    if (total_kbytes == 0) {
        int i, olddrive = current_drive;

        strcpy((char *)name, path);
        for (i = grub_strlen(name); i > 0; i--) {
            if (name[i] == '/')
                break;
        }
        strcpy((char *)&name[i + 1], "size.txt");

        if (new_tftpdir(name) < 0) {
            total_kbytes = 0;
        } else {
            if (grub_open(name)) {
                char buf[17], *ptr = buf;

                grub_read(ptr, 16);
                safe_parse_maxint(&ptr,(int*) &total_kbytes);
                grub_printf("KB to download: %d\n", total_kbytes);
                grub_close();
            }
        }
        current_drive = olddrive;
    }

    grub_printf("\nCopy from sector : %d\n", new_start);

    fat = 0;
    ntfs = 0;
    files = 1;
    new_len = -1;
    new_type = -1;
    curr = new_start;

    /* tell Pulse 2 that we will restore a partition */
    if (sendlog) {
        udp_init();
        grub_sprintf((char*)name, "L2-%s", imgname);
        udp_send_to_pulse2(name, grub_strlen(name));
        udp_close();
        sendlog = 0;
    }

    for (i = 0; i < files; i++) {
        //    grub_printf("-> %d/%d : \r",i+1,files);
        old_perc = -1;

        grub_sprintf((char*)name, "%s%d%c%c", path, l1, l2, l3);
        l3++;
        if (l3 > '9') {
            l2++;
            l3 = '0';
            if (l2 > '9') {
                l1++;
                l2 = '0';
            }
        }

        if (new_get(name, curr, &curr, 0)) {
            errnum = ERR_FILE_NOT_FOUND;
            return 1;
        }
    }
    grub_printf("\nOne partition successfully restored.\n\n");

    return 0;
}
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
/* ptabs */
int ptabs_func(char *arg, int flags) {
    char buf[516];
    char *secbuf = (char *)SCRATCHADDR;
    unsigned long sect;
    int save_drive, i, nb = 0;
    char *sep =
        "\n================================================================================\n";

    if (!set_device((char*)arg))
        return 1;

    save_drive = current_drive;

    // warning message
    grub_sprintf((char*)buf, "%s/etc/warning.txt", basedir);
    if (new_tftpdir(buf) >= 0) {
        if (grub_open(buf)) {
            char c;
            grub_printf(sep);
            while (grub_read(&c, 1))
                grub_putchar(c);
            grub_printf(sep);
            grub_close();
        }
    }

    arg = skip_to(0, arg);

    if (!grub_open(arg))
        return 1;

    while (grub_read(buf, 516)) {
        sect = *(unsigned long *)buf;

//    printf("Writing sect : %d\n",sect);

        for (i = 0; i < 512; i++)
            secbuf[i] = buf[i + 4];

        current_drive = save_drive;
        buf_track = -1;
        if (biosdisk
            (BIOSDISK_WRITE, current_drive, &buf_geom, sect, 1, SCRATCHSEG)) {
            errnum = ERR_WRITE;
            return 1;
        }
        nb++;
    }

    printf("Wrote %d sectors\n", nb);

    grub_close();

    return 0;
}

/* send a fatal error */
void fatal(void) {
    udp_init();
    udp_send_to_pulse2("L8", 2);
    udp_close();
}

/* get and decompress tftp files */
int new_get(char *file, int sect, int *endsect, int table) {

    unsigned long in_sect = sect;

    unsigned char buffer[2100], alloc[24064 - 2048], *cp;
    unsigned char compressed[INBUFF], compressed_full, *ptr;
    int compr_lg, firstblock, sectptr=0, sectmask=1, nb, maxpack;
    int i, size, src, to, block, op, tftpend, end, newp, oldp, lastblk;
    int resendack = 0;
    unsigned char progr, progress[] = "/-\\|";
    int save_sect;

    z_stream zptr;
    int state;
    int ret;
    int ltime;
    int timeout;
 restart:
    save_sect = in_sect;
    sect = in_sect;
    compr_lg = 0;
    firstblock = 1;
    maxpack = 0;
    to = 0;
    tftpend = 0;
    end = 0;
    newp = -1;
    oldp = -1;
    progr = 0;
    state = Z_SYNC_FLUSH;

    tftpbuffer tbuf[NB_BUF], *first, *last, *curr;
    diskbuffer dbuf;
    dbuf.size=0;
    dbuf.full = 0;
    for (i = 0; i < NB_BUF; i++) {
        tbuf[i].length = 0;
        tbuf[i].next = NULL;
    }
    first = last = curr = NULL;
    compressed_full = 0;

    zptr.zalloc = Z_NULL;       //zcalloc;
    zptr.zfree = Z_NULL;        //zcfree;

    zptr.avail_in = 0;
    zptr.next_out = (unsigned char *)BUFFERADDR; // was dbuf.data;
    zptr.avail_out = OUTBUFF;

    inflateInit(&zptr);

    for (i = 0, ret = 0; i < strlen(file); i++)
        if (file[i] == '/')
            ret = i;
    grub_printf
        ("\r                                                                              "
         "\rGet %s : ", file + ret + 1);

    buffer[0] = 0;
    buffer[1] = 1;
    strcpy((char*)buffer + 2, file);
    i = 2 + strlen(file) + 1;
    strcpy((char*)buffer + i, "octet");
    strcpy((char*)buffer + i + 6, "tsize");
    strcpy((char*)buffer + i + 12, "0");
    strcpy((char*)buffer + i + 14, "blksize");
    strcpy((char*)buffer + i + 22, BLKSIZE);

    udp_init();

    lastblk = -1;
    tftpport++;
    if (tftpport > TFTPSTARTPORT + 127)
        tftpport = TFTPSTARTPORT;       /* helper for traffic shaping */

    udp_send((char *)buffer, i + 27, tftpport, 69);     /* Request */
    to = 69;
    ltime = getrtsecs();
    timeout = 0;

    do {
        do {                    /* while udp_get */

            if (dbuf.full) {
                if (firstblock) {
                    fast_memmove(alloc, (char *)BUFFERADDR + 2048,
                                 24064 - 2048);
//                      grub_printf("%s",(char *)BUFFERADDR);
                    firstblock = 0;
                    sectptr = 0;
                    sectmask = 1;

                    save_sect += val("ALLOCTABLELG") * 8;

                    if (exist("FAT32"))
                        fat = 1;
                    if (exist("NTFS"))
                        ntfs = 1;

                    if (exist("BLOCKS")) {
                        unsigned short xypos;
                        files = val("BLOCKS");
#ifdef DEBUG
                        grub_printf("\n%d blocks\n", files);
#endif
                        xypos = getxy();
                        gotoxy(50, (xypos & 255) - 1);
                        grub_printf("Blocks to get: %d", files);
                        gotoxy(xypos >> 8, xypos & 255);
                    }
                    if (exist("SECTORS")) {
                        new_len =val("SECTORS");
#ifdef DEBUG
                        grub_printf("\n%d sectors\n", new_len);
#endif
                    }
                    if (exist("TYPE")) {
                        new_type = val("TYPE");
#ifdef DEBUG
                        grub_printf("\nType : %d\n", new_type);
#endif
                    }
                } else {
                    for (i = 0; i < dbuf.size / 512;) {
                        while (!(alloc[sectptr] & sectmask)) {
                            sect++;
                            inc(&sectptr, &sectmask);
                        }
                        nb = 0;
                        while ((alloc[sectptr] & sectmask)
                               && ((i + nb) < dbuf.size / 512)) {
                            nb++;
                            inc(&sectptr, &sectmask);
                        }

                        if (sect < buf_geom.total_sectors) {
                            if ((sect + nb) > buf_geom.total_sectors) {
                                nb = buf_geom.total_sectors - sect;
                                printf("End of disk, flushing %d sectors\n",
                                       nb);
                            }
                            buf_track = -1;
                            ret =
                                biosdisk(BIOSDISK_WRITE, current_drive,
                                         &buf_geom, sect, nb,
                                         BUFFERSEG + i * 32);
                            if (ret) {
                                grub_printf
                                    ("\n!!! Disk Write Error (%d) sector %d, drive %d\nPress a key\n",
                                     ret, sect, current_drive);
#if 0
                                grub_printf
                                    ("GEO : %d, %d, %d, %d E: %d DR: %d err=%d\n",
                                     buf_geom.total_sectors, buf_geom.cylinders,
                                     buf_geom.heads, buf_geom.sectors, 0,
                                     current_drive, 0);
#endif
                                fatal();
                               
                            }
                        } else {
                            printf
                                ("\n!!! Writing after end of disk (%d>=%d)\nPress a Key\n",
                                 sect, buf_geom.total_sectors);
                            fatal();
                            getkey();
                        }
                        if (maxpack) {
                            if (oldp != newp) {
                                int i;
                                int percent = 0;

                                grub_printf(" [");
                                for (i = 0; i < 32; i++)
                                    if (i < newp)
                                        grub_printf("=");
                                    else
                                        grub_printf(" ");

                                if (total_kbytes != 0)
                                    percent =
                                        (((current_bytes / 1024) * 100) /
                                         total_kbytes);
                                if (percent > 99)
                                    percent = 99;

                                grub_printf("] %c  %d%%", progress[progr],
                                            percent);
                                progr = (progr + 1) & 3;
                                for (i = 0; i < 41; i++)
                                    grub_printf("\b");
                                if (percent >= 10)
                                    grub_printf("\b");
                                oldp = newp;
                            }
                        } else {
                            grub_printf("%c\b", progress[progr]);
                            progr = (progr + 1) & 3;
                        }

                        i += nb;
                        sect += nb;
                    }
                }

                zptr.next_out = (unsigned char *)BUFFERADDR;     // was dbuf.data;
                zptr.avail_out = OUTBUFF;
                dbuf.full = 0;
                if (end == 1) {
                    end = 2;
                }
                goto endturn;
            }

            if (!end && (compressed_full)) {
                if (zptr.avail_in == 0) {
                    zptr.avail_in = compr_lg;
                    zptr.next_in = compressed;
                }
//                              else grub_printf("b");
            }

            if ((zptr.avail_out > 0) && (zptr.avail_in > 0)) {
//               grub_printf("Pre : avin = %d , avout = %d\n",zptr.avail_in,zptr.avail_out);
                ret = inflate(&zptr, state);

                if ((ret == Z_BUF_ERROR) && (zptr.avail_out == 0))
                    ret = Z_OK;

                if (ret == Z_STREAM_END) {
//                      grub_printf("Post: avin = %d , avout = %d (ret=%d)\n",zptr.avail_in,zptr.avail_out,ret);
                    state = Z_FINISH;
                    ret = Z_OK;
                    end = 1;
                    dbuf.full = 1;
                    dbuf.size = OUTBUFF - zptr.avail_out;
                    goto endturn;
                }

                if (ret != Z_OK) {
                    grub_printf("Post: avin = %d , avout = %d (ret=%d)\n",
                                zptr.avail_in, zptr.avail_out, ret);
                    inflateEnd(&zptr);
                    sendERR("Error ZLIB", tftpport, to);
                    udp_close();
                    goto restart;
                }

                if (zptr.avail_in == 0)
                    compressed_full = 0;

                if (zptr.avail_out == 0) {
                    dbuf.full = 1;
                    dbuf.size = OUTBUFF - zptr.avail_out;
                }

                goto endturn;
            }

            if ((first && (tot_lg(first) >= INBUFF))
                || (tftpend && first && tot_lg(first) > 0)) {
                if (compressed_full)
                    grub_printf("B");
                else {
                    if (tftpend)
                        i = tot_lg(first);
                    else
                        i = INBUFF;
                    if (i > INBUFF)
                        i = INBUFF;
                    ptr = compressed;
                    compr_lg = i;
                    while (i > 0) {
                        if (first->length <= i) {
//                 grub_printf("F");
                            fast_memmove(ptr, first->data, first->length);
                            i -= first->length;
                            ptr += first->length;
                            curr = first->next;
                            first->length = 0;
                            first->next = NULL;
                            if (last == first)
                                last = NULL;
                            first = curr;
                        } else {
//                 grub_printf("h");
                            fast_memmove(ptr, first->data, i);
                            ptr += i;
                            fast_memmove(first->data, first->data + i,
                                         first->length - i);
                            first->length -= i;
                            i = 0;
                        }
                    }
                    compressed_full = 1;
                }
            }
 endturn:
            if (getrtsecs() != ltime) {
#if 0
                int val;
 asm("movl %1, %%eax;" "movl %%ebp, %0;":"=&r"(val)
                    /* y is output operand, note the  & constraint modifier. */
 :                 "r"(val)    /* x is input operand */
 :                 "%eax");    /* %eax is clobbered register */
                grub_printf("%x ", val);
#endif
                // called every second
                timeout++;
                ltime = getrtsecs();
                if (timeout > 10) {
                    grub_printf("TFTP Timeout...\n");
                    inflateEnd(&zptr);
                    sendERR("Error Timeout", tftpport, to);
                    udp_close();
                    goto restart;
                }
            }
        }
        while (dbuf.full || !get_empty(tbuf, &i)
               || (!tftpend && udp_get((char *)tbuf[i].udp, &size, tftpport, &src)));

//      grub_printf("-> %d %d : ",size,src);
//      grub_printf("%x %x\n",buffer[0],buffer[1]);
        if (!tftpend) {
            unsigned char *buffer;
            ltime = getrtsecs();
            timeout = 0;
            buffer = tbuf[i].udp;
            op = buffer[1] + 256 * buffer[0];

            switch (op) {
            case 0x06:
                grub_printf("...");
                if (lastblk != -1) {
                    inflateEnd(&zptr);
                    sendERR("Got unexpected OACK", tftpport, to);
                    udp_close();
                    goto restart;
                }
                lastblk = 0;
                cp = buffer + 8;
                safe_parse_maxint((char **)&cp,(int*) &maxpack);
                maxpack /= TFTPBLOCK;
                to = src;
                sendACK(0, tftpport, to);
                break;
            case 0x03:
                block = buffer[3] + 256 * buffer[2];
                if (block == lastblk) {
                    /* The Server retransmitted the packet: Let's ACK but throw away data */
                    grub_printf("Got same packet number...%x\n", block);
                    /* If we send an ACK the tftp server must not have the Appentice Sorcerer Syndrome
                     * described in RFC 1123, or the network will blow !
                     */
                    inflateEnd(&zptr);
                    udp_close();
                    goto restart;

                    //resendack = block;
                    //break;
                } else if (block != (lastblk + 1)) {
                    grub_printf("Got wrong packet number...%d should be %d\n",
                                block, lastblk + 1);
                    inflateEnd(&zptr);
                    sendERR("Got wrong packet number", tftpport, to);
                    udp_close();
                    goto restart;
                } else {
                    resendack = 0;
                    lastblk = block;
                    //grub_printf("DATA : %x (%d)\r",block,size);
                    if (maxpack > 0)
                        newp = (32 * block) / maxpack;
                    else
                        newp = oldp;
                    sendACK(block, tftpport, to);
                    //if (!get_empty(tbuf,&i)) {grub_printf("No TFTP buffer free...\n");goto nel;}
                    if (last)
                        last->next = &tbuf[i];
                    else
                        first = &tbuf[i];
                    //grub_memmove(tbuf[i].data,buffer+4,size-4);
                    tbuf[i].length =
                        ((size - 4) < TFTPBLOCK) ? size - 4 : TFTPBLOCK;
                    current_bytes += tbuf[i].length;
                    last = &tbuf[i];
                    if (size - 4 < TFTPBLOCK) {
#ifdef DEBUG
                        grub_printf("End (%d)", size - 4);
#endif
                        tftpend = 1;
                    }
                }
                break;
            default:
                grub_printf("\nDEF : %x %x \n", buffer[0], buffer[1]);
                if (buffer[1] == 5) {
                    grub_printf("File not found...\n");
                    inflateEnd(&zptr);
                    udp_close();
                    return 1;
                }
                for (i = 0; i < NB_BUF; i++)
                    grub_printf("%d %d -> %x\n", i, tbuf[i].length,
                                tbuf[i].next);
                inflateEnd(&zptr);
                sendERR("Error TFTP", tftpport, to);
                udp_close();
                goto restart;
            }
//        if (first&&tftpend) grub_printf("Remain : %d\n",tot_lg(first));
        }
    }
    while (end != 2);

    grub_printf(".");

    inflateEnd(&zptr);
    udp_close();
    *endsect = save_sect;
    return 0;
}
#ifdef DEBUG
void typestructure(smbios_struct *base,int nbmaxstructure)
{
  smbios_struct *tmp=base,*next;
  grub_printf("nb de structure %d type SMBios: %x \n",nbmaxstructure, tmp->type);
  while(nbmaxstructure>0)
  {
    nbmaxstructure--;
    next = smbios_next_struct (tmp);
    grub_printf("nb structure %d type SMBios: %x\n",nbmaxstructure);
    tmp=next;
  }
}

smbios_struct *
smbios_next_struct (smbios_struct * struct_ptr)
{
    unsigned char *ptr = (unsigned char *) struct_ptr + struct_ptr->length;
    grub_printf("\nlen %d len %d\n",struct_ptr->length, (unsigned char *) struct_ptr + struct_ptr->length);
    /* search end string 0 0 et end bloc*/
    while (ptr[0] != 0x00 || ptr[1] != 0x00)
        ptr++;
    ptr += 2;			/* terminating 0x0000 should be included */
grub_printf("\nnext %d \n",ptr);
    return (smbios_struct *) ptr ;
}

int
smbios_get_struct_length (smbios_struct * struct_ptr)
{
    /* jump to string list */
    unsigned char *ptr = (unsigned char *) struct_ptr + struct_ptr->length;

    /* search for the end of string list */
    while (ptr[0] != 0x00 || ptr[1] != 0x00)
        ptr++;
    ptr += 2;			/* terminating 0x0000 should be included */

    return (int) ptr - (int) struct_ptr;
}
smbios_struct *section;
#endif
/* SMBios */
char *smbios_addr = NULL;
__u16 smbios_len = 0;
__u16 smbios_num = 0;
__u8 *smbios_base = NULL;
__u8 smbios_ver = 0;

/*
 * find smbios area
 * ret: 0=nothing found, 1=ok
 */
int smbios_init(void) {
    char *check;

    for (check = (char *)0xf0000; check <= (char *)0xfffe0; check += 16) {
        if (grub_memcmp(check, "_SM_", 4) == 0)
            if (grub_memcmp(check + 16, "_DMI_", 5) == 0) {
                smbios_addr = check;
                smbios_len = *(__u16 *) (check + 0x16);
                smbios_num = *(__u16 *) (check + 0x1C);
		smbios_base = (__u8 *)(*(__u32 *) (check + 0x18));
#ifdef DEBUG		
		section=(smbios_struct *)smbios_base;
#endif		
		//typestructure((smbios_struct *)smbios_base,smbios_num);
                if (smbios_base == 0 || smbios_num == 0 || smbios_len == 0)
                    continue;
#ifdef DEBUG
                grub_printf
                    ("SMBios         : version %d.%d len %d  num %d %x  nbstruct%d\n",
                     *(check + 6), *(check + 7), smbios_len, smbios_num,
                     smbios_base,smbios_num);
                getkey();
#else
                grub_printf("SMBios         : %d.%d\n", *(check + 6),
                            *(check + 7));
                smbios_ver = (*(check + 6)) * 10 + (*(check + 7));
#endif
                return 1;
            }
    }
    return 0;
}
#ifdef DEBUG
/* affiche une table de smbios si elle existe */
/* n'affiche pas les strings associés à la table*/
void affiche_table_smbios_hexa(unsigned char type, char* buffer)
{
  char hex[]="0123456789ABCDEF";
  smbios_get(-1, NULL);
  unsigned char *smbios_table=NULL;
  int nb=1;	
	while ((smbios_table=smbios_get(type, NULL))!=NULL)
	{
	  buffer += grub_sprintf(buffer,"\naffiche %d table  type %d:",nb,type);
	  nb++;
	  unsigned char *tt;
	  for(tt=smbios_table;tt < smbios_table + smbios_table[1];tt++)
	  {
	    buffer += grub_sprintf(buffer,"0x");
	    *buffer++ = hex[*tt>>4];
	    *buffer++ = hex[*tt&0x0f];
	    buffer += grub_sprintf(buffer," ");
	  }
	}
}
#endif
void smbios_sum(void) {
    /* TODO */
}
// attention cette fonction fonctionne pour smbios 2.0
// pour systeme information cette fonction peut etre utilise pour recupérer 
// manufacture, product name,Version et serial number.
// ne pas utiliser cette fonction pour recupérer les valeurs decrites ci-dessous
// la version 2.1 à 2.3.4 inclut UUID et Wakon_land 
// la version 2.4 et possibilite ajouter des informations propre a un produit.
char *smbios_string(__u8 * dm, __u8 s) {
    char *bp = (char *)dm;
    int i;
    if (s == 0)
        return "-";
    bp += dm[1];
    while (s > 1 && *bp) {
        bp += strlen(bp);
        bp++;
        s--;
    }
    if (!*bp)
        return "badindex";

    for (i = 0; i < strlen(bp); i++)
        if (bp[i] < 32 || bp[i] == 127)
            bp[i] = '.';
    return bp;
}


__u8 *smbios_get(int rtype, __u8 ** rnext) {
    static int i = 0;
    static __u8 *next;
    static __u8 *ptr = NULL;

    if (rtype == -1) {
        ptr = NULL;
        return NULL;
    }
    if (ptr == NULL) {
        ptr = smbios_base;
        i = 0;
    }

    while (i < smbios_num) {
        int type, len, handle;

        type = ptr[0];
        len = ptr[1];
        handle = *(__u16 *) (ptr + 2);

        next = ptr + len;
        while ((next - smbios_base + 1) < smbios_len && (next[0] || next[1]))
            next++;
#ifdef DEBUG
        grub_printf("type %d  len %d  handle %x\n", type, len, handle);
        getkey();
#endif

        if (type == rtype) {
            __u8 *oldptr;

            oldptr = ptr;
            next += 2;
            ptr = next;
            i++;
            return oldptr;
        }
        next += 2;
        ptr = next;
        i++;
    }
    //if (*rnext != NULL) *rnext = NULL;
    ptr = NULL;
    return NULL;
}

/*
 * Returns pointers to: Manufacturer, Product Name, Version, Serial Number, UUID
 */
void smbios_get_sysinfo(char **p1, char **p2, char **p3, char **p4, char **p5) {
    __u8 *ptr;

    ptr = smbios_get(1, NULL);
    if (ptr == NULL)
        return;
    *p1 = smbios_string(ptr, ptr[0x4]);
    *p2 = smbios_string(ptr, ptr[0x5]);
    *p3 = smbios_string(ptr, ptr[0x6]);
    *p4 = smbios_string(ptr, ptr[0x7]);
    printf("Manufacturer   : %s\n", *p1);
    printf("Product        : %s\n", *p2);
    printf("Version        : %s\n", *p3);
    printf("Serial         : %s\n", *p4);
    /* in smbios 2.1+ only */    
    //*p5 = &ptr[8];
    *p5 = (char*)&ptr[8];
 
}

/*
 * Returns pointers to: Vendor, Version, Release
 */
void smbios_get_biosinfo(char **p1, char **p2, char **p3) {
    __u8 *ptr;

    ptr = smbios_get(0, NULL);
    if (ptr == NULL)
        return;
    *p1 = smbios_string(ptr, ptr[0x4]);
    *p2 = smbios_string(ptr, ptr[0x5]);
    *p3 = smbios_string(ptr, ptr[0x8]);
#ifdef DEBUG
    printf("Vendor: %s\n", *p1);
    printf("Version: %s\n", *p2);
    printf("Date: %s\n", *p3);
    getkey();
#endif
}

/*
 * Returns pointers to: Vendor, Type
 */
void smbios_get_enclosure(char **p1, char **p2) {
    __u8 *ptr;

    ptr = smbios_get(3, NULL);
    if (ptr == NULL)
        return;
    *p1 = smbios_string(ptr, ptr[0x4]);
    //*p2 = &ptr[0x5];
     *p2 =(char*) &ptr[0x5];
}

/*
 * Returns pointers to: Size in MB, Form factor, Location, Type, Speed in MHZ
 */
int smbios_get_memory(int *size, int *form, char **location, int *type,
                      int *speed) {
    __u8 *ptr;

    ptr = smbios_get(17, NULL);
    if (ptr == NULL)
        return 0;
    *size = *(__u16 *) (ptr + 0xC);
    if (*size & 0x8000) {
        *size &= 0x7FFF;
        *size *= -1;
    }
    *form = ptr[0xE];
    *location = smbios_string(ptr, ptr[0x10]);
    *type = ptr[0x12] | (ptr[0x14] << 8) | (ptr[0x13] << 16);
#ifdef DEBUG
    printf("Size: %d\n", *size);
    printf("Form: %d\n", *form);
    printf("Location: %s\n", *location);
    printf("Type: %X\n", *type);
    printf("Type: %d\n", *type);
#endif
    if (smbios_ver >= 23) {
        *speed = *(__u16 *) (ptr + 0x15);
#ifdef DEBUG
        printf("Speed: %d\n", *speed);
#endif
    } else {
        *speed = 0;
    }
    return 1;
}

/* Get the number of CPUs (count the number of type 4 structures) */
int smbios_get_numcpu(void) {
    int num = 0;

    smbios_get(-1, NULL);
    while (smbios_get(4, NULL))
        num++;
    if (num == 0)
        num++;
    return num;
}

/* Translate a special key to a common ascii code.  */
int translate_keycode(int c) {
    {
        switch (c) {
        case KEY_LEFT:
            c = 2;
            break;
        case KEY_RIGHT:
            c = 6;
            break;
        case KEY_UP:
            c = 16;
            break;
        case KEY_DOWN:
            c = 14;
            break;
        case KEY_HOME:
            c = 1;
            break;
        case KEY_END:
            c = 5;
            break;
        case KEY_DC:
            c = 4;
            break;
        case KEY_BACKSPACE:
            c = 8;
            break;
        }
    }

    return ASCII_CHAR(c);
}
/* delay func with a progress bar (delay in seconds) */
 void delay_func(int delay,const char *label )
{
    #define TICKS_PER_SEC 20
    unsigned long current=0;
    int i;
    printf(label);
    for(i=0; i < delay; i++) 
    {
      current = currticks()+TICKS_PER_SEC;
       while ( current >= currticks()) {}
       printf(".");
    }
}

int identify_func(char *arg, int flags) {
    unsigned char buffer[52];
    char *title_prompt;
    char *login_prompt;
    char *password_prompt;
    int i;

    if (strstr(arg, "L=fr_FR")) {
        title_prompt = "\n\
ÍÍÍÍÍÍÍÍµ  D‚claration d'un poste client au serveur Pulse Imaging  ÆÍÍÍÍÍÍÍÍÍ\n\
\n\
    Lors de la d‚claration du poste, si l'identifiant respecte le format\n\
    suivant :\n\
\n\
        <profil>:/<entit‚_A>/<entit‚_B>/<nom-de-l'ordinateur>\n\
\n\
    Le poste sera directement ajout‚ au profil <profil> et … l'entit‚\n\
    /entit‚_A/entit‚_B.\n\
\n\
    Attention : si possible, utilisez le nom du poste de travail comme\n\
    identifiant.\
";
        login_prompt =    "  Entrez l'identifiant de ce poste ¯ ";
        password_prompt = "  Entrez vos identifiants Pulse  ¯ ";
    } else if (strstr(arg, "L=pt_BR")) {
        title_prompt = "\n\
ÍÍÍÍÍÍÍÍÍÍÍÍµ  Registre um computador com o Servidor de Imagem Pulse  ÆÍÍÍÍÍÍÍÍÍÍÍÍÍ\n\
\n\
    Quando registrar o computador cliente, o ID respeita o\n\
    seguinte formato:\n\
\n\
        <profile>:/<entity_A>/<entity_B>/<computer-name>\n\
\n\
    O computador irá automaticamente ser adicionado ao <profile> perfil\n\
    e a <entity_A>/<entity_B> entitade.\n\
\n\
    Atençao: Quando possivel, use o nome da maquina com ID.\
";
        login_prompt =    "  Por favor entre com o nome do computador ¯ ";
        password_prompt = "  Por favor entre sua credenciais Pulse ¯ ";
    } else if (strstr(arg, "L=C")) {
        title_prompt = "\n\
ÍÍÍÍÍÍÍÍÍÍÍÍµ  Register a computer with a Pulse Imaging Server  ÆÍÍÍÍÍÍÍÍÍÍÍÍÍ\n\
\n\
    When registering the client computer, if the ID respects the\n\
    following format :\n\
\n\
        <profile>:/<entity_A>/<entity_B>/<computer-name>\n\
\n\
    The computer will automatically be added to the <profile> profile\n\
    and the <entity_A>/<entity_B> entity.\n\
\n\
    Warning : when possible, uses the station name as ID.\
";
        login_prompt =    "  Please enter the name of this computer ¯ ";
        password_prompt = "  Please enter your Pulse credentials  ¯ ";
    } else {
        title_prompt = NULL;
        login_prompt =    "  CLIENT NAME ¯ ";
        password_prompt = "  PASSWORD    ¯ ";
    }

    if (title_prompt)
        printf("%s\n\n", title_prompt);

    buffer[0] = 0xAD;
    buffer[1] = 'I';
    buffer[2] = 'D';
    for (i = 3; i < 52; i++)
        buffer[i] = 0;

    get_cmdline(login_prompt,(char*) buffer + 03, 40, 0, 1);
    i = 3;
    while (buffer[i]) {
        if (buffer[i] == ' ')
            buffer[i] = '_';
        i++;
    }
    buffer[i] = ':';
    i++;
    if (strstr(arg, "P=none") == NULL) {
        get_cmdline(password_prompt,(char*) buffer + i, 10, '*', 1);
        while (buffer[i])
            i++;
    }

    udp_init();
    udp_send_withmac((char*)buffer, i + 1, 1001, 1001);

    i = currticks();
    udp_close();
     #define LABEL  "\nSending registration to the server :"
    delay_func(10,LABEL); // wait sometime (it's definetely not seconds...)
    done_inventory = 0;
    init_bios_info();

    return 0;
}

int identifyauto_func(char *arg, int flags) {
    unsigned char buffer[] = " ID+:+\0";
    buffer[0] = 0xAD;

    udp_init();
    udp_send_withmac((char*)buffer, 7, 1001, 1001);
    udp_close();
    return 0;
}

/* azerty keymap switching */
int kbdfr_func(char *arg, int flags) {

#include "frkbd.h"

    int i;
    unsigned char *ptr = (unsigned char*) ascii_key_map;
    for (i = 0; i < 256; i++) {
        if (i != keyremap[i]) {
            *ptr++ = i;
            *ptr++ = keyremap[i];
        }
    }
    *ptr++ = 0;
    *ptr++ = 0;

    return 0;
}

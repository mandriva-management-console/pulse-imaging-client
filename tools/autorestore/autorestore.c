/*
 * (c) 2003-2007 Ludovic Drolez, Linbox FAS, http://linbox.com
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

/*
 * How it works:
 * - opens and interprets /revosave/conf.txt
 * - logs and info are taken from /revoinfo
 */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <dirent.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/sockios.h>

//#include "autosave.h"
#include "client.h"
#include "image.h"

#define DEBUG(a)
//#define TEST 1
//#define TEST_PARTONLY 1

#include <zlib.h>

#define INSIZE 2048

static unsigned char *BUFFER;
static unsigned char *Bitmap;
static unsigned char *IN;
static unsigned char zero[512];

/* hd space checks enabled ? */
static int revonospc = 0;
/* hd fix the NT boot loader ? */
static int revontblfix = 0;
/* cdrom restoration ? */
static int cdrom = 0;
/* mtftp restoration ? */
static int mtftp = 0;
/* do not run LRS specific code */
static int standalone = 0;
/* use LRS behavior regarding TFTP path */
static int mode_lrs = 0;

static unsigned char buf[512];

/* paths mainly used  by mtftp restoration */
static unsigned char servip[40] = "127.0.0.1";
static unsigned char servprefix[80] = "/";
static unsigned char storagedir[80] = "/";
static char imagename[256] = "";
static char hostname[32] = "";

/* paths */
static const char *revosave = "/revosave";
static const char *revoinfo = "/revoinfo";
static const char *revobin = "/revobin";
static const char *outdir = NULL;
static char tmppath[1024];
static char logtxt[1024];

/* do we have the bios HD map ? */
static char *hdmap[65536];
static int nonewt = 0;

/* */
typedef struct params_ {
    int bitindex;
    int fo;
    __u64 offset;
} PARAMS;

/* Q&D IO abstraction layer */
struct fops_ {
    int (*get) (char *fname, int filenum);
    FILE *(*open) (char *fname, int filenum);
    int (*close) (FILE * stream);
} fops;

/* KB decompressed */
static unsigned int todo = 0;
static unsigned int done = 0;
static char todos[32];

/*
 * printf() func with logging
 */
static void myprintf(const char *format_str, ...)
{
    va_list ap;
    FILE *foerr;

    /* write some info */
    foerr = fopen(logtxt, "a");
    fprintf(foerr, "\n==== misc ====\n");
    va_start(ap, format_str);
    vfprintf(foerr, format_str, ap);
    va_end(ap);
    fclose(foerr);
}

/*
 * system() func with logging
 */
static int mysystem(const char *s)
{
    char cmd[1025];
    FILE *foerr;

    strncpy(cmd, s, 1024 - strlen(logtxt) - 9 - 1);
    strcat(cmd, " >> ");
    strcat(cmd, logtxt);
    strcat(cmd, " 2>&1");


    /* write some info */
    foerr = fopen(logtxt, "a");
    // ctime()
    fprintf(foerr, "\n==== %s ====\n", s);
    fclose(foerr);
#ifdef TEST
    return 0;
#endif
    return (system(cmd));

}

/*
 * system() func with logging (logs stdout not stderr)
 */
static int mysystem1(const char *s)
{
    char cmd[1024];
    char *redir = " 1>> ";
    FILE *foerr;

    strncpy(cmd, s, 1024 - strlen(logtxt) - 5 - 1);
    strcat(cmd, redir);
    strcat(cmd, logtxt);

    /* write some info */
    foerr = fopen(logtxt, "a");
    // ctime()
    fprintf(foerr, "\n==== %s ====\n", s);
    fclose(foerr);

    return (system(cmd));

}

/*
 * snprintf to tmppath global var
 */
static char *tmprintf(const char *format_str, ...)
{
    va_list ap;

    va_start(ap, format_str);
    vsnprintf(tmppath, 1023, format_str, ap);
    va_end(ap);

    return tmppath;
}

/*
 * Fatal error
 */
static void fatal(void)
{
    if (!standalone)
        return;
    system("revosendlog 8");
    exit(EXIT_FAILURE);
}

/* Update the current file label */
static void update_file(char *f, int n, int max, char *dev)
{
    char ns[32], maxs[32];

    sprintf(ns, "%d", n);
    sprintf(maxs, "%d", max);
    ui_send("refresh_file", 4, f, ns, maxs, dev);
}

/* fatal write error */
static void ui_seek_error(char *s, int l, int err, int fd, off64_t seek)
{
    char tmp[256];
    int bs=0, mb=0;
    off64_t offset;

    /* get device block size */
    ioctl(fd, BLKBSZGET, &bs);
    /* get current offset */
    offset = lseek64(fd, 0, SEEK_CUR);

    if (seek != 0) {
        offset = seek;
    }
    mb = (int)(offset/1024/1024);
    snprintf(tmp, 255, "Hard Disk Write Error ! Bad or too small hard disk !\n"
        "errno %d (%s)\n"
        "file %s, line %d \n"
        "bs=%d  %s=%08lx%08lx (%d MiB)",
        err, strerror(err), s, l, bs, (seek!=0?"seek":"offset"),
        (long)((long long)offset>>32), (long)offset, mb);
    fprintf(stderr, "ERROR: %s\n", tmp);

    /* fatal error */
    system("/bin/revosendlog 8");
    ui_send("misc_error", 2, "HD Write error", tmp);
}

static void ui_write_error(char *s, int l, int err, int fd)
{
    ui_seek_error(s, l, err, fd, 0);
}


/* zlib error */
static void ui_zlib_error(int err)
{
    char tmp[32];

    sprintf(tmp, "%d", err);
    ui_send("zlib_error", 1, tmp);
}

/*
 * Restore a raw partition file to 'device'x
 */
static void restore_raw(char *device, char *fname)
{
    char buffer[1024];
    __u32 sect;
    int fo, fp;

    // log
    myprintf("restore raw: %s, %s\n", device, fname);

    fo = open(device, O_WRONLY | O_LARGEFILE);
    tmprintf("%s/%s", revosave, fname);
    fp = open(tmppath, O_RDONLY | O_LARGEFILE);

    while (1) {
        if (read(fp, buffer, 516) == 0) {
            break;
        }
        sect = *(__u32 *) buffer;
        if (lseek64(fo, (__u64) 512 * (__off64_t) sect, SEEK_SET) == -1) {
            DEBUG(printf("restore_raw: seek error\n"));
            ui_seek_error(device, __LINE__, errno, fo,
                          (__u64) 512 * (__off64_t) sect);
        }
        if (write(fo, buffer + 4, 512) != 512) {
            DEBUG(printf("restore_raw: write error\n"));
            ui_write_error(device, __LINE__, errno, fo);
        }
    }
    close(fp);
    if (ioctl(fo, BLKRRPART) < 0) {
        myprintf("Reloading partition table failed\n");
    }
    close(fo);
}


/*
 * File ops
 */
static int file_get(char *fname, int filenum)
{
    char f[64];
    struct stat st;
    int ret;

    sprintf(f, "%s%03d", fname, filenum);
    DEBUG(printf("** File: %s **", f));
  retry:
    chdir(revosave);
    ret = stat(f, &st);
    if (ret == -1 && cdrom) {
        /* ask to swap the media */
        chdir("/");
        /* hardcoded paths because this feature is only used in the LRS-CD */
        system("umount /revosave");
        /* wait */
        ui_send("misc_error", 2, "Media change", "Please insert the next CD, and press the 'c' key");
        system("mount -t iso9660 /dev/cdrom /revosave");
        goto retry;
    }
    return (ret);
}

static FILE *file_open(char *fname, int filenum)
{
    char f[64];
    FILE *fid;

    sprintf(f, "%s%03d", fname, filenum);
    fid = fopen(f, "r");
    return (fid);
}

static int file_close(FILE * stream)
{
    return fclose(stream);
}

/*
 * Tftp ops
 */
static int tftp_get(char *fname, int filenum)
{
    char f[64], cmd[512];
    struct stat st;

    sprintf(f, "%s%03d", fname, filenum);
    DEBUG(printf("** File: %s **", f));
    chdir("/tmpfs");
    if (!nonewt)
        update_file(fname, filenum, -2, "Waiting");
    sprintf(cmd, "revowait %s", f);
    system(cmd);
    if (!nonewt)
        update_file(fname, filenum, -2, "Downloading");
    /* get files */
    do {
        system("rm * >/dev/null 2>&1");
        if (!mode_lrs) {
            sprintf(cmd,
                    "atftp --tftp-timeout 10 --option \"blksize 4096\" --option multicast -g -r %s/%s/%s %s 69 2>/tmp/atftp.log",
                    storagedir, imagename, f, servip);
        } else {
            sprintf(cmd,
                    "atftp --tftp-timeout 10 --option \"blksize 4096\" --option multicast -g -r %s/%s/%s %s 69 2>/tmp/atftp.log",
                    servprefix, storagedir, f, servip);
        }
    } while (system(cmd));

    return (stat(f, &st));
}

static FILE *tftp_open(char *fname, int filenum)
{
    char f[64];

    sprintf(f, "%s%03d", fname, filenum);
    return (fopen(f, "r"));
}

static int tftp_close(FILE * stream)
{
    int ret = fclose(stream);

    return ret;
}

/*
 *
 */
static void flushToDisk(unsigned char *buff, unsigned char *bit, PARAMS * cp, int lg)
{
    unsigned char *ptr = buff;
    unsigned char mask[] =
        { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
    int indx = cp->bitindex;


// printf("Enter : bitindex -> %d\n",indx);
    while (lg > 0) {
        __u64 s = 0;
        while (!(bit[indx >> 3] & mask[indx & 7])) {
            indx++;
            cp->offset += 512;
            s += 512;
        }
        if ((s != 0) && (lseek64(cp->fo, s, SEEK_CUR) < 0)) {
            ui_write_error(__FILE__, __LINE__, errno, cp->fo);
        }
//      printf("Write @offset : %lld\t",cp->offset);
//      {int i; for(i=0;i<15;i++) printf("%02x ",ptr[i]); printf("\n");}
        if (cp->fo) {
            if (write(cp->fo, ptr, 512) != 512) {
                ui_write_error(__FILE__, __LINE__, errno, cp->fo);
            }
        }
        cp->offset += 512;
        ptr += 512;
        indx++;
        lg -= 512;
    }
// printf("Exit  : bitindex -> %d\n",indx);
    cp->bitindex = indx;
}


/*
 *
 */
static void restore(char *device, unsigned int sect, char *fname)
{
    int fo;                     /* output device */
    z_stream zptr;
    int state, filenum, fmax = -1, upcnt;
    int ret, firstpass, bitmaplg;
    FILE *fi;
    PARAMS currentparams;
    __u64 i, starto;

    currentparams.offset = 512 * (__u64) sect;
    currentparams.bitindex = 0;
    memset(zero, 0, 512);

    // log
    myprintf("restore: %s, offset %d sectors, %s\n", device, sect, fname);

    // open the output device
    fo = open(device, O_WRONLY | O_LARGEFILE);
    currentparams.fo = fo;
    DEBUG(printf("Seeking to : %lld\n", currentparams.offset));
    i = lseek64(currentparams.fo, currentparams.offset, SEEK_SET);
    if (i != currentparams.offset) {
        ui_seek_error(__FILE__, __LINE__, errno, currentparams.fo,
                      currentparams.offset);
    }
    // open the data directory
    filenum = 0;
    while (!fops.get(fname, filenum)) {
        /*      name = ep->d_name;
           l = strlen(name);
           if (strncmp(name, fname, strlen(fname))) continue;
           if (l < 4) continue;
           if (!isdigit(name[l-1]) || !isdigit(name[l-2]) || !isdigit(name[l-3]) ) continue;
           DEBUG (printf ("** File: %s **", fname));
         */
      start:
        if (!nonewt)
            update_file(fname, filenum, fmax, device);

        state = Z_SYNC_FLUSH;
        firstpass = 1;
        bitmaplg = 0;

        zptr.zalloc = NULL;
        zptr.zfree = NULL;

        starto = lseek64(currentparams.fo, 0, SEEK_CUR);        /* save the current offset */

        fi = fops.open(fname, filenum);
        if (fi == NULL) {
            ui_send("misc_error", 2, "Error", "Cannot open input file");
            fatal();
        }

        zptr.avail_in = fread(IN, 1, INSIZE, fi);

        currentparams.offset = 0;
        currentparams.bitindex = 0;

        zptr.next_in = (unsigned char *) IN;
        zptr.next_out = (unsigned char *) BUFFER;       // was dbuf.data;
        zptr.avail_out = TOTALLG;

        inflateInit(&zptr);

        upcnt = 0;
        do {
            /* decompress */
            ret = inflate(&zptr, state);

            if ( upcnt++ % 100 == 0 ) {
                /* update status */
                char tmp1[32], tmp2[32];
                sprintf(tmp1, "%llu", ((long long unsigned)done)*1024 + zptr.total_in);
                sprintf(tmp2, "%llu", ((long long unsigned)done)*1024 + zptr.total_in);
                ui_send("refresh_backup_progress", 2, tmp1, tmp2);
            }
            if ((ret == Z_OK) && (zptr.avail_out == 0)) {
                if (firstpass) {
                    DEBUG(printf("Params : *%s\n", BUFFER));
                    if (strstr(BUFFER, "BLOCKS=")) {
                        int i = 0;
                        if (sscanf(strstr(BUFFER, "BLOCKS=") + 7, "%d", &i)
                            == 1) {
                            fmax = i;
                        }
                    }
                    if (strstr(BUFFER, "ALLOCTABLELG="))
                        sscanf(strstr(BUFFER, "ALLOCTABLELG=") + 13, "%d",
                               &bitmaplg);
                    memcpy(Bitmap, BUFFER + HEADERLG, ALLOCLG);
                    currentparams.bitindex = 0;
                    firstpass = 0;
                } else {
                    flushToDisk(BUFFER, Bitmap, &currentparams, TOTALLG);
                }

                zptr.next_out = (unsigned char *) BUFFER;
                zptr.avail_out = TOTALLG;
            }

            if ((ret == Z_OK) && (zptr.avail_in == 0)) {
                zptr.avail_in = fread(IN, 1, INSIZE, fi);
                zptr.next_in = (unsigned char *) IN;
            }
        }
        while (ret == Z_OK);

        if (ret == Z_STREAM_END) {
            {
                if (firstpass) {
                    DEBUG(printf("Params : *%s*\n", BUFFER));
                    if (strstr(BUFFER, "BLOCKS=")) {
                        int i = 0;
                        if (sscanf(strstr(BUFFER, "BLOCKS=") + 7, "%d", &i)
                            == 1) {
                            fmax = i;
                        }
                    }
                    if (strstr(BUFFER, "ALLOCTABLELG="))
                        sscanf(strstr(BUFFER, "ALLOCTABLELG=") + 13, "%d",
                               &bitmaplg);
                    memcpy(Bitmap, BUFFER + HEADERLG, ALLOCLG);
                    zptr.next_out = (unsigned char *) BUFFER;
                    zptr.avail_out = TOTALLG;
                }
            }

            //printf ("Flushing to EOF ... (%d bytes)\n",
            //      24064 - zptr.avail_out);
            flushToDisk(BUFFER, Bitmap, &currentparams,
                        TOTALLG - zptr.avail_out);
            zptr.next_out = (unsigned char *) BUFFER;
            zptr.avail_out = TOTALLG;
        }

        ret = inflate(&zptr, Z_FINISH);
        inflateEnd(&zptr);

        if (ret < 0) {
            /*printf ("Returned : %d\t", ret);
               printf ("(AvailIn : %d / ", zptr.avail_in);
               printf ("AvailOut: %d)\n", zptr.avail_out);
               printf ("(TotalIn : %ld / ", zptr.total_in);
               printf ("TotalOut: %ld)\n", zptr.total_out); */
            ui_zlib_error(ret);
            fops.close(fi);
            fops.get(fname, filenum);   /* reget file */
            /* return to the correct offset */
            lseek64(currentparams.fo, starto, SEEK_SET);

            goto start;
        }

        /*printf ("Offset : %lld\n", currentparams.offset);
           printf ("Bitmap index : %d\n", currentparams.bitindex); */

        if (bitmaplg) {
            if (bitmaplg * 8 > currentparams.bitindex) {
                currentparams.offset +=
                    (__u64) 512 *(bitmaplg * 8 - currentparams.bitindex);
                if (currentparams.fo) {
                    /* no error check for the last fill */
                    /* bounds will be checked by the following write */
                    lseek64(currentparams.fo,
                            (__u64) 512 * (bitmaplg * 8 -
                                           currentparams.bitindex),
                            SEEK_CUR);
                }
            }
        }

        filenum++;
        fops.close(fi);
        done += zptr.total_in / 1024;
        if ((fmax != -1) && (filenum >= fmax))
            break;
    }
    close(fo);
}

/*
 */
static char *find(const char *str, const char *fname)
{
    FILE *f;

    f = fopen(fname, "r");
    if (f == NULL)
        return NULL;
    while (fgets((char *) buf, 256, f)) {
        if (strstr(buf, str)) {
            fclose(f);
            return strstr(buf, str) + strlen(str);
        }
    }
    fclose(f);
    return NULL;
}

/*
 * Get NFS server informations
 */
static void netinfo(void)
{
    char *ptr, *ptr2;

    if ((ptr = find("Next server: ", "/etc/netinfo.log"))) {
        ptr2 = ptr;
        while (*ptr2) {
            if (*ptr2 < ' ') {
                *ptr2 = 0;
                break;
            } else
                ptr2++;
        }
        //printf ("*%s*\n", ptr);
        strcpy(servip, ptr);
    }

    if ((ptr = find("Boot file: ", "/etc/netinfo.log"))) {
        ptr2 = strstr(ptr, "/bin");
        if (ptr2)
            *ptr2 = 0;
        //printf ("*%s*\n", ptr);
        strcpy(servprefix, ptr);
    }
}

/*
 * Get the LBS host name
 */
static void gethost(void)
{
    FILE *f;

    hostname[0] = 0;

    f = fopen("/etc/lbxname", "r");
    if (f != NULL) {
        fscanf(f, "%31s", hostname);
        fclose(f);
    }

    tmprintf("%s/hostname", revoinfo);
    f = fopen(tmppath, "r");
    if (f == NULL)
        return;
    fscanf(f, "%31s", hostname);
    fclose(f);
}


static void setdefault(char *v)
{
    char buf[256];

    if (standalone)
        return;

    sprintf(buf, "revosetdefault %s", v != NULL ? v : "0");
    system(buf);
}

/*
 * interprets the conf.txt file
 */
static void restoreimage(void)
{
    FILE *f;
    char buf[255], buf2[255], lvm[255];
    int vgscan = 0;
    char *conftxt;

    if (cdrom) {
        conftxt = "/tmp/conf.txt";
    } else {
        tmprintf("%s/conf.txt", revosave);
        conftxt = tmppath;
    }
    if ((f = fopen(conftxt, "r")) == NULL)
        return;
    while (!feof(f)) {
        fgets(buf, 250, f);
        if (sscanf(buf, "%s", buf2) == 1) {
            /* buf=full line, buf2=1st keyword */
            DEBUG(printf("%s\n", buf2));
            if (!strcmp("ptabs", buf2)) {
                // ptabs command
                unsigned int d1;

                if (sscanf(buf, " ptabs (hd%u) (nd)PATH/%s", &d1, buf2) ==
                    2) {
                    DEBUG(printf("%d,%s\n", d1, buf2));
                    // restore the files to the device
#ifdef TEST
                    //restore_raw("/revoinfo/PTABS", buf2);
#else
                    myprintf("grub cmdline: %s\n", buf);
                    restore_raw(hdmap[d1], buf2);
#endif
                }

            } else if (!strcmp("partcopy", buf2)) {
                // partcopy command
                unsigned int d1, d2, sect;
#ifdef TEST_PARTONLY
                continue;
#endif
                if (sscanf(buf, " partcopy (hd%u,%u) %u PATH/%s",
                           &d1, &d2, &sect, buf2) == 4) {
                    DEBUG(printf("%d,%d,%d,%s\n", d1, d2, sect, buf2));
                    // convert the BIOS hd number to a Linux device
                    // and restore the files to the device
                    if (d1 >= 3968 && d2 == -1) {
                        // lvm: no hdmap necessary
                        strncpy(lvm, "/dev/", 5);
                        if (sscanf
                            (buf, " partcopy (hd%*u,%*u) %*u PATH/%*s %s",
                             &lvm[5]) != 1) {
                            myprintf("syntax error in conf.txt: %s\n",
                                     buf);
                            exit(1);
                        }
                        if (vgscan == 0) {
                            system
                                ("lvm vgscan >/dev/null 2>&1; lvm vgchange -ay >/dev/null 2>&1");
                            vgscan = 1;
                        }
                        DEBUG(printf("lvm : %s\n", lvm));
                        restore(lvm, sect, buf2);

                    } else {
#ifdef TEST
                        // restore("/revoinfo/P1", sect, buf2);
#else
                        myprintf("grub cmdline: %s\n", buf);
                        restore(hdmap[d1], sect, buf2);
#endif
                        // fix the NT Bootloader if needed
                        if (sect == 63 && revontblfix == 1) {
                            char command[255];
                            sprintf(command, "ntblfix %s %d", hdmap[d1],
                                    d1 + 1);
                            mysystem(command);
                        }
                    }
                }
            } else if (!strcmp("setdefault", buf2)) {
                strtok(buf, " ");
                setdefault(strtok(NULL, " "));
            } else if (!strcmp("chainloader", buf2)) {
                setdefault("0");
            }
        }
    }
    fclose(f);
}

/*
 * Check if the image can fit
 */
static void checkhdspace(__u32 major, __u32 minor, __u32 sect)
{
    FILE *f;
    char command[256];
    __u32 orig = 0;

    if (revonospc)
        return;

    tmprintf("%s/size%02x%02x.txt", revosave, major, minor);
    f = fopen(tmppath, "r");
    if (f == NULL)
        return;
    fscanf(f, "%u", &orig);
    if (orig > sect) {
        /* problem : the disk seems to be too small */
        if (!standalone)
            system("revosendlog 8");
        sprintf(command, "Your hard disk seems to be to small to restore this image (%u vs %u KB).\n\nIf you want to restore anyway, you can disable the disk space checks in the client's options panel.", sect, orig);
        ui_send("misc_error", 2, "Error", command);
        while (1)
            sleep(1);
    }
    fclose(f);
}

/*
 * Build a BIOS number to device map
 * (should use the 'hdmap' file if present)
 */
static void makehdmap(void)
{
    FILE *fp;
    int i = 0;
    unsigned int d, major, minor, sec;
    char line[256], buf[256];

    for (i = 0; i < 256; i++)
        hdmap[i] = NULL;

    d = 0;
    fp = fopen("/proc/partitions", "r");
    if (fp == NULL)
        return;
    while (!feof(fp)) {
        fgets(line, 255, fp);
        if (sscanf(line, " %u %u %u %s\n", &major, &minor, &sec, buf) == 4) {
            if ((((major == 3) || (major == 22) || (major == 33)
                  || (major == 34)) && !(minor & 0x3F)) ||
                (((major == 8) || (major == 65)
                  || (major >= 72 && major <= 79)
                  || (major >= 104 && major <= 111))
                 && !(minor & 0xF))) {
                char *str;

                /* check that the HD is big enough for the restore */
                checkhdspace(major, minor, sec);
                /* fill the hdmap */
                str = malloc(strlen(buf)*2);
                if (outdir) {
                        strcpy(str, outdir);
                        strcat(str, "/");
                } else {
                        strcpy(str, "/dev/");
                }
                strcat(str, buf);
                hdmap[d] = str;
                myprintf("hdmap: %d %d %d %s\n", minor, major, d, str);
                d++;
            }
        }
    }
    fclose(fp);

    return;

}


/*
 * Get the total image size (from /revosave/size.txt)
 */
static int getbytes(void)
{
    int kb = 0;
    FILE *f;

    tmprintf("%s/size.txt", revosave);
    f = fopen(tmppath, "r");
    if (f == NULL) {
        tmprintf("du -k %s", revosave);
        f = popen(tmppath, "r");
    }
    if (f == NULL) {
        return kb;
    }
    fscanf(f, "%d", &kb);
    fclose(f);
    return kb;
}

/*
 * command line parsing
 */
static void commandline(int argc, char *argv[])
{
    char *ptr, *ptr2;
    int c;

    while (1) {
        static struct option long_options[] = {
            /* These options set a flag. */
            {"nospc", no_argument, &revonospc, 1},
            {"ntblfix", no_argument, &revontblfix, 1},
            {"mtftp", no_argument, &mtftp, 1},
            {"standalone", no_argument, &standalone, 1},
            {"nolrs", no_argument, &standalone, 1},
            /* These options don't set a flag.
               We distinguish them by their indices. */
            {"save", required_argument, 0, 's'},
            {"info", required_argument, 0, 'i'},
            {"bin", required_argument, 0, 'b'},
            {"outdir", required_argument, 0, 'o'},
            {"mode", required_argument, 0, 'm'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 's':
            revosave = optarg;
            break;
        case 'i':
            revoinfo = optarg;
            break;
        case 'b':
            revobin = optarg;
            break;
        case 'o':
            outdir = optarg;
            revonospc = 1;
            break;
        case 'm':
            if (strcasecmp(optarg, "lrs") == 0) {
                mode_lrs = 1;
            } else if ((strcasecmp(optarg, "pulse") == 0) ||
                       (strcasecmp(optarg, "pulse2") == 0)) {
                mode_lrs = 0;
            } else {
                fprintf(stderr, "Invalid mode '%s' requested\n", optarg);
                exit(1);
            }
            break;
        case '?':
            printf
                ("usage: autorestore [--nospc] [--standalone] [--ntblfix] [--mtftp]\n"
                 "      [--save /revosave] [--info /revoinfo] [--bin /revobin]\n"
                 "      [--outdir restore_dir] [--mode lrs|pulse2]\n");
            exit(1);
        }
    }

    tmprintf("%s/log.restore", revoinfo);
    strcpy(logtxt, tmppath);

    if ((ptr = find("revosavedir=", "/etc/cmdline"))) {
        ptr2 = ptr;
        while (*ptr2 != ' ')
            ptr2++;
        *ptr2 = 0;
        //printf ("*%s*\n", ptr);
        strcpy(storagedir, ptr);
    }

    /* search for image UUID on command line */
    if ((ptr = find("revoimage=", "/etc/cmdline"))) {
        ptr2 = ptr;
        while (*ptr2 != ' ')
            ptr2++;
        *ptr2 = 0;
        //printf ("*%s*\n", ptr);
        strcpy(imagename, ptr);
    }

    /* default: mtftp restore */
    fops.open = tftp_open;
    fops.close = tftp_close;
    fops.get = tftp_get;

    if (!mtftp) {
        /* nfs restore */
        fops.open = file_open;
        fops.close = file_close;
        fops.get = file_get;
    }
#ifdef TEST
    fops.open = file_open;
    fops.close = file_close;
    fops.get = file_get;
#endif
}

/*
 * MAIN
 */
int main(int argc, char *argv[])
{
    /* init */
    FILE *f;
    BUFFER = malloc(TOTALLG);
    Bitmap = malloc(ALLOCLG);
    IN = malloc(INSIZE);

    netinfo();
    commandline(argc, argv);

    // mount nfs dirs
    if (strcmp(storagedir, "/cdrom")) {
        // now mounted by mount.sh
    } else {
        cdrom = 1;
    }

    // some logging
    f = fopen(logtxt, "w");     /* truncate the log file */
    if (f != NULL)
        fclose(f);
    mysystem1("cat /etc/cmdline");
    mysystem1("cat /proc/cmdline");
    mysystem1("cat /proc/version");
    mysystem1("cat /proc/partitions");
    mysystem1("cat /proc/bus/pci/devices");
    mysystem1("cat /proc/modules");
    mysystem1("cat /var/log/messages");

    // now we can use config files from the nfs server
    makehdmap();

    // debug info
    gethost();
    todo = getbytes();
    snprintf(todos, 32, "%u", todo);

    if (!mode_lrs) {
        ui_send("init_restore", 4, servip, storagedir, hostname, todos);
    } else {
        ui_send("init_restore", 4, servip, servprefix, hostname, todos);
    }

    if (!standalone)
        system("revosendlog 2");

    system(tmprintf("echo \"\">%s/progress.txt", revoinfo));
    restoreimage();

    ui_send("close", 0);

    if (!standalone)
        system("revosendlog 3");

    system(tmprintf("echo \"\">%s/progress.txt", revoinfo));
    mysystem1("cat /var/log/messages");

    return 0;
}

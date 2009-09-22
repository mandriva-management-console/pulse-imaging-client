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

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <newt.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "ui_newt.h"

void suspend(void * d) {
    newtSuspend();
    raise(SIGTSTP);
    newtResume();
}


newtComponent sc1, sc2, f;
newtComponent t, i1,i2,l1,l2,l3,l4,t1 ;
newtComponent time1,time2,bitrate ;

time_t start,now;
int old_curr,old_nb;

unsigned long olddiff;
unsigned long olddone;
extern unsigned long done,todo;

void init_newt(char *device, char *savedir, char *argv0, char *hn, int tot_sec)
{
    char name[256];

    newtInit();
    newtCls();

    newtSetSuspendCallback(suspend, NULL);

    newtDrawRootText(0, 0, "Linbox Rescue Server");

    sprintf(name, "LBLImage v%s", LBLIMAGEVER);
    newtOpenWindow(2, 2, 72, 20, name);

    f = newtForm(NULL, NULL, 0);

    t1=newtTextbox(1,1,70,5, NEWT_FLAG_WRAP);

    sprintf(name, " %s:%s -> %s", device, savedir, hn);
    l1=newtLabel(3,20,name);

    l3=newtLabel(37,12,"%");
    sc1=newtScale(13,11,54,100);
    newtScaleSet(sc1,0);
    l4=newtLabel(3,11,"Percent : ");
    //sc2=newtScale(13,12,54,100);
    //newtScaleSet(sc2,0);

    //    todo = tot_sec;
    sprintf (name, "- Compressed data : %d MiB\n", tot_sec/1024);
    i1=newtLabel(3,7,name);
    sprintf (name, "- Restoring : \n" );
    i2=newtLabel(3,8,name);

    time1=newtLabel(3,15,"Elapsed time   : ..H..M..");
    time2=newtLabel(3,16,"Remaining time : ..H..M..");
    bitrate=newtLabel(3,17,"Bitrate        : ...... KBps");

    read_update_head();

    newtFormAddComponents(f, sc1, i1, i2, l1, l3,l4,time1,time2,bitrate,t1,NULL);
    newtRefresh();
    newtDrawForm(f);

    start=time(NULL);
    olddiff=0;
    olddone=0;

}

void close_newt(void)
{
    newtFormDestroy(f);
    newtFinished();
}

/* Update the current file label */
void update_file(char *f, int n, int max, char *dev, int kb)
{
  char buf[256];
  int fi;
  char *path = "/revoinfo/progress.txt";

  if (max == -1)
    snprintf (buf, 255, "- Restoring : %s%03d  -> %s\n       ", f, n, dev);
  else if (max == -2)
    snprintf (buf, 255, "- Restoring : %s%03d (%s)  ", f, n, dev);
  else
    snprintf (buf, 255, "- Restoring : %s%03d (/%d) -> %s\n     ", f, n, max-1, dev);
  newtLabelSetText(i2, buf);
  newtRefresh();

  /* only update the restore progress info if the file exists */
  if ((fi = open(path, O_TRUNC|O_WRONLY)) != -1)
    {
      int p = 100*kb/todo;

      if (p > 100) p = 100;
      snprintf(buf, 255, "%d%%", p);
      write(fi, buf, strlen(buf));
      close(fi);
    }

}

/* update the progress bar */
void update_progress(int kb)
{
    char buf[80];
    unsigned long diff,remain;
    int h,m,s;
    static int bps = 0;
    float p = (100.0*kb)/todo;

    if (p > 100) p = 100;

    now=time(NULL);
    diff=(unsigned long)difftime(now,start);
    if (diff==olddiff) return;

    newtScaleSet(sc1,p);
    sprintf(buf,"%3.1f %%", p);
    newtLabelSetText(l3,buf);

    h=diff/3600;
    m=(diff/60)%60;
    s=diff%60;
    sprintf(buf,"Elapsed time   : %02dh%02dm%02d",h,m,s);
    newtLabelSetText(time1,buf);

    bps=((9*bps)/10)+(kb-olddone)/(10*(diff-olddiff));

    sprintf(buf,"Bitrate        : %d KBps",bps);
    newtLabelSetText(bitrate,buf);

    //if (bps>0) remain=(todo-kb)/bps;
    //      else remain=99*60*60+59*60+59;
    if (diff > 10)  remain=(((double)todo/(double)kb)*(double)diff)-diff;
    else remain=99*60*60+59*60+59;

    h=remain/3600;
    m=(remain/60)%60;
    s=remain%60;
    sprintf(buf,"Remaining time : %02dh%02dm%02d",h,m,s);
    //printf("Remaining time : %d %d %d %d \n",kb,remain,todo,bps);
    //printf("%d\n",bps);
    newtLabelSetText(time2,buf);

    olddone= kb;
    olddiff=diff;

    newtRefresh();
}

void update_head(char *msg)
{
    newtTextboxSetText(t1, msg);
    newtRefresh();
}

void read_update_head(void)
{
    char *path = "/etc/warning.txt";
    char msg[512];
    int f;

    msg[0] = 0;
    if ((f = open(path, O_RDONLY)) != -1) {
    int sz = read(f, msg, 511);
    close(f);

    msg[sz+1] = 0;
    update_head(msg);
    }
}

/* zlib error */
void ui_zlib_error(int err)
{
    char tmp[256];
    newtComponent myForm, l;

    newtCenteredWindow(60, 3, "Decompression Error");

    myForm = newtForm(NULL, NULL, 0);
    sprintf(tmp, "ZLib decompression error %d. Retrying....", err);
    l = newtLabel(1, 1, tmp);

    newtFormAddComponents(myForm, l, NULL);
    newtDrawForm(myForm);
    newtRefresh();
    sleep(30);
    newtPopWindow();
}

/*
 * fatal write error
 */
void ui_seek_error(char *s, int l, int err, int fd, off64_t seek)
{
    char tmp[256];
    int bs=0, mb=0;
    off64_t offset;
    newtComponent myForm, t1;

    /* get device block size */
    ioctl(fd, BLKBSZGET, &bs);
    /* get current offset */
    offset = lseek64(fd, 0, SEEK_CUR);

    newtCenteredWindow(60, 5, "HD Write Error");

    myForm = newtForm(NULL, NULL, 0);
    t1 = newtTextbox(1, 0, 55, 4, 0);

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

    newtTextboxSetText(t1, tmp);
    newtFormAddComponents(myForm, t1, NULL);
    newtDrawForm(myForm);
    newtRefresh();

    /* fatal error : wait forever */
    system("/bin/revosendlog 8");
    getchar();
    //while (1) sleep(1);
}


void ui_write_error(char *s, int l, int err, int fd)
{
    ui_seek_error(s, l, err, fd, 0);
}

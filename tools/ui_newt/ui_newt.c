/*
 * (c) 2003-2007 Ludovic Drolez, Linbox FAS, http://linbox.com
 * (c) 2008-2009 Nicolas Rueff, Mandriva, http://www.mandriva.com
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <newt.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fs.h>
#include <unistd.h>
#include <fcntl.h>

#include "ui_newt.h"
#include "server.h"

newtComponent newtProgressScale, f;
newtComponent t, i1, i2, l1, l2, l3, l4, t1;
newtComponent newtElapsedTime, newtRemainingTime, newtBitrate, newtNetrate;

time_t start;
int g_old_curr, g_old_nb, g_partnum;

unsigned long gDiskIoBps, gNetIoBps;
unsigned long gPreviousElapsedTime;
unsigned long long gDiskIODone, gPreviousDiskIODone, gDiskIOTodo;
unsigned long long gNetIODone, gPreviousNetIODone, gNetIOTodo;

char *humanReadable(float num, char* unit, int base, int padded ) {
    char *units[8] = {"Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi"};
    char *defaultUnit = "B";
    int defaultBase = 1024;
    char* output = malloc(256);
    int i;

    if (base == 0)
        base = defaultBase;

    if (unit == NULL)
        unit = defaultUnit;

    if (num < defaultBase) {
        if (padded)
            snprintf(output, 256, "%5.0f   %s", num, unit);
        else
            snprintf(output, 256, "%.0f %s", num, unit);
    } else {
        for (i = 0; i < 8; i++) {
            num /=  base;
            if (num < defaultBase) {
                if (padded)
                    snprintf(output, 256, "%5.1f %s%s", num, units[i], unit);
                else
                    snprintf(output, 256, "%.1f %s%s", num, units[i], unit);
                break;
            }
        }
    }

    return output;
}

void suspend(void *d) {
    newtSuspend();
    raise(SIGTSTP);
    newtResume();
}

/* update the bitrate, times */
void update_misc(void) {
    char buf[80];
    unsigned long elapsedTime, remainingTime;

    elapsedTime = (unsigned long)difftime(time(NULL), start);
    if (elapsedTime == gPreviousElapsedTime)
        return;

    // Bps ponderation :
    // we sum 90% from the previous (average) computed value
    // with 10% from the current (instantaneous) value
    gDiskIoBps = (9 * gDiskIoBps / 10) + (gDiskIODone - gPreviousDiskIODone) / (10 * (elapsedTime - gPreviousElapsedTime));
    gNetIoBps  = (9 * gNetIoBps / 10)  + (gNetIODone  - gPreviousNetIODone)  / (10 * (elapsedTime - gPreviousElapsedTime));

    sprintf(buf, "Elapsed time   : %5d min %02d sec", (int)(elapsedTime / 60), (int)(elapsedTime % 60));
    newtLabelSetText(newtElapsedTime, buf);
    sprintf(buf, "Disk bitrate   : %s", humanReadable(gDiskIoBps, "B/s", 1024, 1));
    newtLabelSetText(newtBitrate, buf);
    //MDV/NR sprintf(buf, "Storage I/O    : %s", humanReadable(gNetIoBps, "B/s", 1024, 1));
    //MDV/NR newtLabelSetText(newtNetrate, buf);

    if (elapsedTime > 9)
        remainingTime = (((double)gDiskIOTodo / (double)gDiskIODone) * (double)elapsedTime) - elapsedTime;
    else
        remainingTime = 0;

    if (remainingTime < 0)
        remainingTime = 0;

    if (remainingTime) {
        sprintf(buf, "Remaining time : %5d min %02d sec", (int)(remainingTime / 60), (int)(remainingTime % 60));
        newtLabelSetText(newtRemainingTime, buf);
    }

    gPreviousDiskIODone = gDiskIODone;
    gPreviousNetIODone = gNetIODone;
    gPreviousElapsedTime = elapsedTime;
}

/* update the partition number */
void update_part(char *dev) {
    struct stat st;

    if (stat(dev, &st) == 0) {
        g_partnum = (minor(st.st_rdev) & 15) - 1;
        if (major(st.st_rdev) == 254) {
            g_partnum |= 0x100;
        }
    } else {
        g_partnum = -1;
    }
}

/* write the progress to a file */
void update_file(int perc) {
    int f;
    char *path = "/revosave/progress.txt";
    /* only update if the file exists */

    if ((f = open(path, O_TRUNC | O_WRONLY)) != -1) {
        char buf[256];

        snprintf(buf, 255, "%s%d: %d%%", (g_partnum > 255) ? "Lvm " : "",
                 g_partnum & 255, perc);
        write(f, buf, strlen(buf));
        close(f);
    }
}

/* show the backup/restore message */
void update_head(char *msg) {
    newtTextboxSetText(t1, msg);
    newtRefresh();
}

void read_update_head(void) {
    char *path = "/etc/warning.txt";
    char msg[512];
    int f;

    msg[0] = 0;
    if ((f = open(path, O_RDONLY)) != -1) {
        int sz = read(f, msg, 511);
        close(f);

        msg[sz] = 0;
        update_head(msg);
    }
}

/*
 *  Show the main newt based interface
 *
 *  args: device, savedir, total sectors, used sectors, image type
 */
char *init_newt(int argc, char **argv) {
    char name[256];

    char *device = argv[0];
    char *savedir = argv[1];
    unsigned long tot_sec;
    unsigned long used_sec;
    char *argv0 = argv[4];

    sscanf(argv[2], "%lu", &tot_sec);
    sscanf(argv[3], "%lu", &used_sec);

    newtInit();
    newtCls();

    newtSetSuspendCallback(suspend, NULL);

    newtDrawRootText(0, 0, "Pulse 2 Imaging Client");

    sprintf(name, "Pulse 2 Imager %s v%s", rindex(argv0, '_') + 1, LBLIMAGEVER);
    newtOpenWindow(2, 2, 72, 20, name);

    f = newtForm(NULL, NULL, 0);

    t1 = newtTextbox(1, 1, 70, 5, NEWT_FLAG_WRAP);

    sprintf(name, " %s ==> %s ", device, savedir);
    l1 = newtLabel(3, 20, name);

    l3 = newtLabel(37, 12, "%");
    newtProgressScale = newtScale(13, 11, 54, 100);
    newtScaleSet(newtProgressScale, 0);
    l4 = newtLabel(3, 11, "Progress: ");

    sprintf(name, "Total size : %s\n",humanReadable((float)tot_sec * 512, "B", 1024, 0));
    i1 = newtLabel(3, 7, name);
    sprintf(name, "Total data : %s\n",humanReadable((float)used_sec * 512, "B", 1024, 0));
    i2 = newtLabel(3, 8, name);

    newtElapsedTime =   newtLabel(3, 14, "Elapsed time   :       min    sec");
    newtRemainingTime = newtLabel(3, 15, "Remaining time :       min    sec");
    newtBitrate =       newtLabel(3, 16, "Disk bitrate   :");
    //MDV/NR newtNetrate =       newtLabel(3, 17, "Storage I/O    :");

    //MDV/NR newtFormAddComponents(f, newtProgressScale, i1, i2, l1, l3, l4, newtElapsedTime, newtRemainingTime,
                          //MDV/NR newtBitrate, newtNetrate, t1, NULL);
    newtFormAddComponents(f, newtProgressScale, i1, i2, l1, l3, l4, newtElapsedTime, newtRemainingTime,
                          newtBitrate, t1, NULL);

    read_update_head();
    update_part(device);

    newtDrawForm(f);
    newtRefresh();

    start = time(NULL);
    gPreviousElapsedTime = 0;
    gPreviousDiskIODone = 0;
    gDiskIoBps = 0;

    gDiskIOTodo = used_sec * (unsigned long long)512;
    gDiskIODone = 0;

    return "OK";
}

/*
 * Show the main newt interface for the restoration
 *
 * (char *device, char *savedir, char *hn, unsigned int size)
 */
char *init_newt_restore(int argc, char **argv) {
    char *oargv[] = { argv[0], argv[1], argv[3], argv[3], "_restore" };

    char name[80];

    char *device = argv[0];
    char *savedir = argv[1];
    char *hn = argv[2];
    unsigned long long size = atoll(argv[3]);

    init_newt(5, oargv);

    snprintf(name, 63,
             " %s:%s -> %s                                                 ",
             device, savedir, hn);
    newtLabelSetText(l1, name);
    snprintf(name, 63,
             "- Compressed data : %llu MiB                                 ",
             size / 1024);
    newtLabelSetText(i1, name);
    snprintf(name, 63,
             "- Restoring :                                                ");
    newtLabelSetText(i2, name);
    newtRefresh();

    gDiskIOTodo = size * 1024;
    gDiskIODone = 0;

    return "OK";
}

/*
 *  Close newt, no arguments
 *
 *
 */
char *close_newt(int argc, char **argv) {
    newtFormDestroy(f);
    newtFinished();

    return "OK";
}

/*
* fatal write error
*
* no arguments
*/
char *backup_write_error(int argc, char **argv) {
    newtComponent myForm, l;

    newtCenteredWindow(60, 3, "Server Write Error");

    myForm = newtForm(NULL, NULL, 0);
    l = newtLabel(1, 1, "Write error ! Server's disk might be full.");

    newtFormAddComponents(myForm, l, NULL);
    newtDrawForm(myForm);
    newtRefresh();

    return "OK";
}

/*
* update the scale and bitrate calcs
 *
 * arguments: number of sectors read
*/
char *update_progress(int argc, char **argv) {
    char buf[80];
    // only use the amount of real data read
    float progressPercentile;
    static int oldProgressPercentile = 0;

    gDiskIODone = atoll(argv[0]);
    gNetIODone = atoll(argv[1]);

    progressPercentile = (100.0 * (float)gDiskIODone) / (float)gDiskIOTodo;
    if (progressPercentile > 100)
        progressPercentile = 100;

    newtScaleSet(newtProgressScale, progressPercentile);
    sprintf(buf, "%3.2f%%", progressPercentile);
    newtLabelSetText(l3, buf);
    update_misc();
    newtRefresh();

    if (progressPercentile >= oldProgressPercentile + 1)
        update_file(progressPercentile);

    oldProgressPercentile = progressPercentile;
    return "OK";
}

/*
 * Update the current file label
 *
 * arguments: filename, current block, max block, device
 */
char *update_file_restore(int argc, char **argv) {
    char buf[256];
    int fi;
    char *path = "/revoinfo/progress.txt";

    char *f = argv[0];
    int n = atoi(argv[1]);
    int max = atoi(argv[2]);
    char *dev = argv[3];

    if (max == -1)
        sprintf(buf, "- Restoring : %s%03d  => %s\n       ", f, n, dev);
    else if (max == -2)
        sprintf(buf, "- Restoring : %s%03d (%s)  ", f, n, dev);
    else
        sprintf(buf, "- Restoring : %s%03d (/%d) => %s\n     ", f, n, max - 1,
                dev);
    newtLabelSetText(i2, buf);
    newtRefresh();

    /* only update the restore progress info if the file exists */
    if ((fi = open(path, O_TRUNC | O_WRONLY)) != -1) {
        int p = 100 * gDiskIODone / gDiskIOTodo;

        if (p > 100)
            p = 100;
        snprintf(buf, 255, "%d%%", p);
        write(fi, buf, strlen(buf));
        close(fi);
    }
    return "OK";
}

/*
 * display the zlib error message (and wait 30 seconds)
 *
 * argument: error number
 */
char *zlib_error(int argc, char **argv) {
    char tmp[256];
    newtComponent myForm, l;

    newtCenteredWindow(60, 3, "Decompression Error");

    myForm = newtForm(NULL, NULL, 0);
    sprintf(tmp, "ZLib decompression error %s. Retrying....", argv[0]);
    l = newtLabel(1, 1, tmp);

    newtFormAddComponents(myForm, l, NULL);
    newtDrawForm(myForm);
    newtRefresh();
    sleep(30);
    newtPopWindow();

    return "OK";
}

/*
 * Display an error message
 *
 * arguments: title, message
 */
char *misc_error(int argc, char **argv) {
    newtComponent myForm, l;

    newtInit();
    newtCls();

    // FIXME. Really needed?
    newtDrawRootText(0, 0, "Pulse 2 Imaging Client");

    newtOpenWindow(2, 2, 72, 20, "Pulse 2 Imager v" LBLIMAGEVER);

    newtRefresh();

    newtCenteredWindow(60, 10, argv[0]);

    myForm = newtForm(NULL, NULL, 0);
    l = newtTextbox(1, 1, 58, 8, NEWT_FLAG_WRAP);
    newtTextboxSetText(l, argv[1]);
    newtFormAddComponents(myForm, l, NULL);
    newtDrawForm(myForm);

    newtBell();
    newtRefresh();

    waitkey();

    newtPopWindow();
    newtFormDestroy(myForm);
    newtRefresh();

    return "OK";
}

/*
 * Wait for the 'c' key
 */
void waitkey(void) {
    int fd;

    fd = open("/dev/input/event1", O_RDONLY);
    if (fd != -1) {
        unsigned short ev[8];

        while (read(fd, &ev, sizeof(ev))) {
            if (ev[5] == 0x2e) {
                break;
            }
        }
        close(fd);
    }
    sleep(1);
}

/*
 * MAIN
 */
int main(void) {

    struct cmd_s commands[] = {
        {"init_backup", 5, init_newt},
        {"init_restore", 4, init_newt_restore},
        {"close", 0, close_newt},
        {"refresh_backup_progress", 1, update_progress},
        {"refresh_file", 5, update_file_restore},
        {"backup_write_error", 0, backup_write_error},
        {"zlib_error", 1, zlib_error},
        {"misc_error", 2, misc_error},
        {NULL, 0, NULL}
    };

    server_loop(7001, commands);
    // should not return
    return 1;
}

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

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <newt.h>
#include <time.h>

int main(int argc, char *argv[]) {
    int i;

    if (argc != 2) {
        fprintf(stderr, "Usage : image_error message-to-display\n");
        exit(1);
    }

    init_newt(argv[1]);
    // close_newt ();

    return 0;
}

/* newt */

newtComponent sc1, sc2, f;
newtComponent t, i1, i2, l1, l2, l3, l4;
newtComponent time1, time2, bitrate;

time_t start, now;
int old_curr, old_nb;

unsigned long olddiff, bps;
unsigned long long olddone;
unsigned long long done, todo;

void init_newt(unsigned char *message) {
    newtComponent myForm, l;

    newtInit();
    newtCls();

    newtDrawRootText(0, 0, "LBLImage");

    newtOpenWindow(2, 2, 72, 20, "LBLImage v" LBLIMAGEVER);

    newtRefresh();

    newtCenteredWindow(60, 10, "LBL Error");

    myForm = newtForm(NULL, NULL, 0);
    //l = newtLabel (1, 1, message);
    l = newtTextbox(1, 1, 58, 8, NEWT_FLAG_WRAP);
    newtTextboxSetText(l, message);
    newtFormAddComponents(myForm, l, NULL);
    newtDrawForm(myForm);

    newtBell();

    newtRefresh();

}

void close_newt(void) {
    newtFormDestroy(f);

    //SLsmg_refresh();
    //SLsmg_reset_smg();
    SLang_reset_tty();

    //newtFinished ();
}

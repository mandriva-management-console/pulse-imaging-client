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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>

#include "easy_sock.h"
#include "server.h"

#define BACKLOG 50
//#define DEBUG 1
#define SNIFF 1
#define MAX_VARS 100

#ifdef DEBUG
#define debug(s) printf("== %i == %s",getpid(),s)
#else
#define debug(s)
#endif

#ifdef SNIFF
#define sniff(c) putchar(c)
#else
#define sniff(c)
#endif

#define checkError(e) if (e <= 0) debug(easy_sock_err_msg);

int pid = -1;
static int sock, sock_1 = -1;

char mem_char[MAX_VARS];
short mem_short[MAX_VARS];
int mem_int[MAX_VARS];
long mem_long[MAX_VARS];
float mem_float[MAX_VARS];
double mem_double[MAX_VARS];
char *mem_string[MAX_VARS];

/**
 * Catch signals:
 *  SIGUSR1 close socket1
 *  Others signals close all sockets
 * Signals will be send to process anyway
 */
static void ctrlC(int sig)
{
    char sd[80];
    sprintf(sd, "Catch signal: %i\n", sig);
    debug(sd);

    if (sig == SIGUSR1)
	close(sock_1);
    else {
	close(sock);
	close(sock_1);
    }

    /*for (j = 0; j < MAX_VARS; j++)
       if (mem_string[j] != NULL)
       free(mem_string[j]); */

    sprintf(sd, "Raising signal: %i\n", SIGKILL);
    debug(sd);
    raise(SIGKILL);		/* send a signal to the current process */
}				/* ctrlC */

/**
 * Handles inplicit reading errors
 */
static void onError(int err)
{
    if (err != 0)
	debug(easy_sock_err_msg);
    close(sock_1);
    sock_1 = -1;
}

/*
 * Main server loop never returns
 *
 * Listen on 'port', and exec functions according to the 'commands' structure.
 *
 * Expected protocol used (PMRPC = poor's man RPC):
 * - 4 bytes: length of the next string
 * - x bytes: the command string
 * - 4 bytes: number of arguments
 * - 4 bytes: length of the next string (1st argument)
 * - x bytes: 1st arguments
 * - ... more arguments with length before.
 */
int server_loop(int port, struct cmd_s *commands)
{
    char *host = "127.0.0.1";
    int i;
    char *cmd;
    int args;
    char *arg[255];
    char *ret;
    struct cmd_s *curcmd;

    /* binding address */
    sock = easy_tcp_bind(host, port, BACKLOG);
    if (sock == -1) {
	debug("Can't bind address!\n");
	debug(easy_sock_err_msg);
	exit(EXIT_FAILURE);
    }

    /* Set signal handler */
    signal(SIGTERM, ctrlC);
    signal(SIGINT, ctrlC);
    signal(SIGUSR1, ctrlC);
    signal(SIGPIPE, SIG_IGN);

    /* Set error handler */
    easy_error(onError);

    while (1) {
	/* accepting connections */
	if (sock_1 == -1) {
	    sock_1 = accept(sock, 0, 0);
	}
	if (sock_1 == -1) {
	    //close(sock);
	    //close(sock_1);
	    continue;
	    //exit(1);
	}

	/* Reading data */
	cmd = read_string(sock_1);
	if (sock_1 == -1)
	    continue;
	args = read_int(sock_1);
	if (sock_1 == -1)
	    continue;

	/* Read arguments */
	for (i = 0; i < args; i++) {
	    arg[i] = read_string(sock_1);
	}

	/* lookup table */
	curcmd = commands;
	while (1) {
	    //printf("%s\n", curcmd->name);
	    if (curcmd->name == NULL)
		break;
	    if (!strcmp(curcmd->name, cmd)) {
		/* command found */
		ret = (curcmd->func) (args, arg);
		/* return string */
		write_string(sock_1, ret);
		break;
	    }
	    curcmd++;
	}

	/* free the strings */
	if (cmd)
	    free(cmd);
	for (i = 0; i < args; i++) {
	    if (arg[i])
		free(arg[i]);
	}

//      if (strcmp(curcmd->name, "close") == 0) {
	close(sock_1);
	sock_1 = -1;
//      }

    }
    return 1;
}

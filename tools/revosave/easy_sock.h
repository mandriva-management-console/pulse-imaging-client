
/*
 * +---------------------------------------------------------+
 * | Easy Socket library - version 0.1                       |
 * | (C) 1999-2000, Erich Roncarolo <erich@roncarolo.eu.org> |
 * +---------------------------------------------------------+
 *
 * This file is part of Easy Socket library.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#ifndef __EASY_SOCK__
#define __EASY_SOCK__

/*
 * Max lengths of every type written in chars (as string)
 */
#define ES_MAX_CHAR 1
#define ES_MAX_SHORT 8
#define ES_MAX_INT 16
#define ES_MAX_LONG 16
#define ES_MAX_FLOAT 32
#define ES_MAX_DOUBLE 32

/*
 * Max number of digits of a string length
 */
#define ES_MAX_LEN ES_MAX_INT

/*
 * Length of string containing error messages
 */
#define EASY_SOCK_ERR_MSG_LEN 50

/*
 * This the error number when an error occours
 */
int easy_sock_err;

/*
 * This string contains an error message when an error occours
 */
char easy_sock_err_msg[EASY_SOCK_ERR_MSG_LEN];

/*
 * Installs an error handler.
 */
void easy_error(void (*handler)(int));

/*
 * This is a very useful function used to connect an host on a port.
 * 'host' is the hostname
 * 'port' is the port number
 * return a socket descriptor
 */
int easy_tcp_connect(char *host, int port);

/*
 * This is a very useful function used to bind an host on a port.
 * 'host' is the hostname (if it is NULL bind any address)
 * 'port' is the port number
 * 'backlog' define the maximum length the queue of pending connections
 *  may grow to.
 * return a socket descriptor
 */
int easy_tcp_bind(char *host, int port, int backlog);


/*
 * This is a function that read a char from a socket.
 */
char read_char(int sock);

/*
 * This is a function that read a short from a socket.
 */
short read_short(int sock);

/*
 * This is a function that read an integer from a socket.
 */
int read_int(int sock);

/*
 * This is a function that read a long from a socket.
 */
long read_long(int sock);

/*
 * This is a function that read a float from a socket.
 */
float read_float(int sock);

/*
 * This is a function that read a double from a socket.
 */
double read_double(int sock);

/*
 * This is a function that read a string from a socket.
 * return a new 'mallocated' string or NULL if an error occours.
 *
 * Details: this function reads first the string length (int) then
 *          use malloc() to allocate new string so it can be read.
 *          Remember! It's your own responsibility free() the string.
 */
char* read_string(int sock);

/*
 * This is a function that write a char to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_char(int sock, char c);

/*
 * This is a function that write a short to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_short(int sock, short s);

/*
 * This is a function that write an integer to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_int(int sock, int i);

/*
 * This is a function that write a long to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_long(int sock, long l);

/*
 * This is a function that write a float to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_float(int sock, float f);

/*
 * This is a function that write a double to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_double(int sock, double d);

/*
 * This is a function that write a string to a socket.
 * return the number of written chars, or a number < 0 if an error occours.
 *
 * Details: this function writes first the string length (int) then
 *          the string itself, so read_string() can read it.
 */
int write_string(int sock, char* string);



/*
 * This is a function that read a char from a socket.
 * For compatibility only: works exactly like read_char().
 */
char read_char_c(int sock);

/*
 * This is a function that read a short (as chars) from a socket.
 */
short read_short_c(int sock);

/*
 * This is a function that read an integer (as chars) from a socket.
 */
int read_int_c(int sock);

/*
 * This is a function that read a long (as chars) from a socket.
 */
long read_long_c(int sock);

/*
 * This is a function that read a float (as chars) from a socket.
 */
float read_float_c(int sock);

/*
 * This is a function that read a double (as chars) from a socket.
 */
double read_double_c(int sock);

/*
 * This is a function that read a string from a socket.
 * return a new 'mallocated' string or NULL if an error occours.
 *
 * Details: this function reads first the string length (as char) then
 *          use malloc() to allocate new string so it can be read.
 *          Remember! It's your own responsibility free() the string.
 */
char* read_string_c(int sock);

/*
 * This is a function that write a char to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 * For compatibility only: works exactly like write_char().
 */
int write_char_c(int sock, char c);

/*
 * This is a function that write a short (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_short_c(int sock, short s);

/*
 * This is a function that write an integer (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_int_c(int sock, int i);

/*
 * This is a function that write a long (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_long_c(int sock, long l);

/*
 * This is a function that write a float (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_float_c(int sock, float f);

/*
 * This is a function that write a double (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_double_c(int sock, double d);

/*
 * This is a function that write a string to a socket.
 * return the number of written chars, or a number < 0 if an error occours.
 *
 * Details: this function writes first the string length (as chars) then
 *          the string itself, so read_string_c() can read it.
 */
int write_string_c(int sock, char* string);

#endif


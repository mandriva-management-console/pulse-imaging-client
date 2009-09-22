
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>

#include "easy_sock.h"

/*
 * Type of a error handler
 */
typedef void (*easy_error_t)(int);

/*
 * Default signal handler
 */
void easy_default_error_handler(ssize_t err) {}

/*
 * Actual error handler
 */
easy_error_t err_handler = easy_default_error_handler;

/*
 * Installs the error handler.
 */
void easy_error(easy_error_t handler) {
		err_handler = handler;
}

/*
 * This is a very useful function used to connect an host on a port.
 * 'host' is the hostname
 * 'port' is the port number
 * return a socket descriptor
 */
int easy_tcp_connect(char *host, int port) {
	//int i;
	int sock;
	struct sockaddr_in rsock;
	struct hostent * hostinfo;
	struct in_addr * addp;
	//struct sockaddr_in sockname;

	memset ((char *)&rsock,0,sizeof(rsock));

	if ( (hostinfo=gethostbyname(host)) == NULL ) {
        sprintf(easy_sock_err_msg,"Cannot find %s - %s\n",host,strerror(errno));
        return -1;
    }
	
	sock=socket(AF_INET,SOCK_STREAM,0);
	if ( sock == -1 ) {
        sprintf(easy_sock_err_msg,"Can't create socket - %s\n",strerror(errno));
        return -1;
	}
	
	addp=(struct in_addr *)*(hostinfo->h_addr_list);
	rsock.sin_addr=*addp;
	rsock.sin_family=AF_INET;
	rsock.sin_port=htons(port);

	if ( connect(sock,(struct sockaddr *)(&rsock),sizeof(rsock)) == -1 ) {
        sprintf(easy_sock_err_msg,"Can't connect %s on port %i - %s\n",host,port,strerror(errno));
        return -1;
	}
	return sock;
}

/*
 * This is a very useful function used to bind an host on a port.
 * 'host' is the hostname (if it is NULL bind any address)
 * 'port' is the port number
 * 'backlog' define the maximum length the queue of pending connections
 *  may grow to.
 * return a socket descriptor
 */
int easy_tcp_bind(char *host, int port, int backlog) {
	const int on = 1;
	int sock;
	struct hostent * hostinfo;
	struct in_addr * addp;
	struct sockaddr_in sockname;

	memset ((char *)&sockname,0,sizeof(sockname));

	if (host == NULL) {
		hostinfo = NULL;
	} else if ( (hostinfo=gethostbyname(host)) == NULL ) {
        sprintf(easy_sock_err_msg,"Cannot find %s - %s\n",host,strerror(errno));
        return -1;
    }
	
	if( (sock=socket(AF_INET,SOCK_STREAM,0)) == -1 ) {
		sprintf(easy_sock_err_msg,"Error opening socket - %s\n",strerror(errno));
		return -1;
	}
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	if (hostinfo != NULL) {
		addp=(struct in_addr *)*(hostinfo->h_addr_list);
		sockname.sin_addr=*addp;
	} else {
		sockname.sin_addr.s_addr=INADDR_ANY;
	}
	sockname.sin_family=AF_INET;
	sockname.sin_port=htons(port);

	if ( (bind(sock,(struct sockaddr *)&sockname,sizeof(sockname))) == -1 ) {
		close (sock);
		sprintf(easy_sock_err_msg,"Cannot bind port %i at %s -%s\n",port,host,strerror(errno));
		return -1;
	}
	listen (sock,backlog);
	return (sock);
}

/*
 * This is a function that read a char from a socket.
 * It expect 
 */
char read_char(int sock) {
	char c;
	ssize_t err;
	
	err = read(sock,&c,sizeof(c));

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading char.\n");
		err_handler(EOF);
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading char: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
	}

	return (c);
}

/*
 * This is a function that read a short from a socket.
 */
short read_short(int sock) {
	short s;
	ssize_t err;
	
	err = read(sock,&s,sizeof(s));

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading short.\n");
		err_handler(EOF);
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading short: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
	}
	
	return (s);
}

/*
 * This is a function that read an integer from a socket.
 */
int read_int(int sock) {
	int i;
	ssize_t err;
	
	err = read(sock,&i,sizeof(i));

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading int.\n");
		err_handler(EOF);
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading int: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
	}
	
	return (i);
}

/*
 * This is a function that read a long from a socket.
 */
long read_long(int sock) {
	long l;
	ssize_t err;
	
	err = read(sock,&l,sizeof(l));

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading long.\n");
		err_handler(EOF);
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading long: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
	}
	
	return (l);
}

/*
 * This is a function that read a float from a socket.
 */
float read_float(int sock) {
	float f;
	ssize_t err;
	
	err = read(sock,&f,sizeof(f));

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading float.\n");
		err_handler(EOF);
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading float: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
	}
	
	return (f);
}

/*
 * This is a function that read a double from a socket.
 */
double read_double(int sock) {
	double d;
	ssize_t err;
	
	err = read(sock,&d,sizeof(d));

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading double.\n");
		err_handler(EOF);
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading double: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
	}
	
	return (d);
}

/*
 * This is a function that read a string from a socket.
 * return a new 'mallocated' string or NULL if an error occours.
 *
 * Details: this function reads first the string length (int) then
 *          use malloc() to allocate new string so it can be read.
 *          Remember! It's your own responsibility free() the string.
 */
char* read_string(int sock) {
	size_t len;
	ssize_t err;
	char* string;
	
	err = read(sock,&len,sizeof(len));

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading string length.\n");
		err_handler(EOF);
		return NULL;
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading string length: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
		return NULL;
	}
	if (len == 0) return NULL;
		
	string = (char*)malloc(len+1);
	memset(string,0,len+1);
	
	err = read(sock,string,len);

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading string.\n");
		err_handler(EOF);
		return NULL;
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading string: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
		return NULL;
	}
	string[len] = (char)0;
	
	return (string);
}

/*
 * This is a function that write a char from a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_char(int sock, char c) {
	ssize_t err;
	
	err = write(sock,&c,sizeof(c));

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No char written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing char: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write a short to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_short(int sock, short s) {
	ssize_t err;
	
	err = write(sock,&s,sizeof(s));

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No short written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing short: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write an integer from a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_int(int sock, int i) {
	ssize_t err;
	
	err = write(sock,&i,sizeof(i));

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No int written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing int: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write a long from a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_long(int sock, long l) {
	ssize_t err;
	
	err = write(sock,&l,sizeof(l));

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No long written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing long: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write a float from a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_float(int sock, float f) {
	ssize_t err;
	
	err = write(sock,&f,sizeof(f));

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No float written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing float: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write a double from a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_double(int sock, double d) {
	ssize_t err;
	
	err = write(sock,&d,sizeof(d));

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No double written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing double: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write a string from a socket.
 * return the number of written chars, or a number < 0 if an error occours.
 *
 * Details: this function writes first the string length (int) then
 *          the string itself, so read_string() can read it.
 */
int write_string(int sock, char* string) {
	ssize_t err;
	size_t len = strlen(string);
	
	err = write(sock,&len,sizeof(len));

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No string length written: why?\n");
		return 0;
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing string length: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	err = write(sock,string,len);

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No string written: why?\n");
		return 0;
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing string: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that read a char from a socket.
 * For compatibility only: works exactly like read_char().
 */
char read_char_c(int sock) {
	return (read_char(sock));
}

/*
 * This is a function that read a short (as chars) from a socket.
 */
short read_short_c(int sock) {
	char s[ES_MAX_SHORT];
	char c = 'S';
	int i;
	ssize_t err;
	
	memset(s,0,ES_MAX_SHORT);

	for (i = 0; i < ES_MAX_SHORT && c != 0 && c != '\n'; i++) {
		err = read(sock,&c,ES_MAX_CHAR);
	
		if (err == 0) {
			sprintf(easy_sock_err_msg,"End of file reached reading short.\n");
			err_handler(EOF);
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error reading short: %i - %s\n",easy_sock_err,strerror(errno));
			err_handler(easy_sock_err);
		}

		if (isdigit(c))
			s[i] = c;
		else if (c != 0 && c != '\n') {
			sprintf(easy_sock_err_msg,"Error reading short: isdigit(%c) == FALSE\n",c);
			err_handler(1000+c);
		}
			
	}
	
	return (short)atoi(s);
}

/*
 * This is a function that read an integer (as chars) from a socket.
 */
int read_int_c(int sock) {
	char j[ES_MAX_INT];
	char c = 'I';
	int i;
	ssize_t err;
	
	memset(j,0,ES_MAX_INT);

	for (i = 0; i < ES_MAX_INT && c != 0 && c != '\n'; i++) {
		err = read(sock,&c,ES_MAX_CHAR);

		if (err == 0) {
			sprintf(easy_sock_err_msg,"End of file reached reading short.\n");
			err_handler(EOF);
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error reading int: %i - %s\n",easy_sock_err,strerror(errno));
			err_handler(easy_sock_err);
		}
	
		if (isdigit(c))
			j[i] = c;
		else if (c != 0 && c != '\n') {
			sprintf(easy_sock_err_msg,"Error reading int: isdigit(%c) == FALSE\n",c);
			err_handler(1000+c);
		}
			
	}
	
	return atoi(j);
}

/*
 * This is a function that read a long (as chars) from a socket.
 */
long read_long_c(int sock) {
	char l[ES_MAX_LONG];
	char c = 'L';
	int i;
	ssize_t err;
	
	memset(l,0,ES_MAX_LONG);

	for (i = 0; i < ES_MAX_LONG && c != 0 && c != '\n'; i++) {
		err = read(sock,&c,ES_MAX_CHAR);

		if (err == 0) {
			sprintf(easy_sock_err_msg,"End of file reached reading long.\n");
			err_handler(EOF);
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error reading long: %i - %s\n",easy_sock_err,strerror(errno));
			err_handler(easy_sock_err);
		}
	
		if (isdigit(c))
			l[i] = c;
		else if (c != 0 && c != '\n') {
			sprintf(easy_sock_err_msg,"Error reading long: isdigit(%c) == FALSE\n",c);
			err_handler(1000+c);
		}
			
	}
	
	return atol(l);
}

/*
 * This is a function that read a float (as chars) from a socket.
 */
float read_float_c(int sock) {
	char f[ES_MAX_FLOAT];
	char c = 'G';
	int i;
	ssize_t err;
	
	memset(f,0,ES_MAX_FLOAT);

	for (i = 0; i < ES_MAX_FLOAT && c != 0 && c != '\n'; i++) {
		err = read(sock,&c,ES_MAX_CHAR);

		if (err == 0) {
			sprintf(easy_sock_err_msg,"End of file reached reading float.\n");
			err_handler(EOF);
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error reading float: %i - %s\n",easy_sock_err,strerror(errno));
			err_handler(easy_sock_err);
		}
		
		f[i] = c;
	}
	
	return (float)atof(f);
}

/*
 * This is a function that read a double (as chars) from a socket.
 */
double read_double_c(int sock) {
	char d[ES_MAX_DOUBLE];
	char c = 'K';
	int i;
	ssize_t err;
	
	memset(d,0,ES_MAX_DOUBLE);

	for (i = 0; i < ES_MAX_DOUBLE && c != 0 && c != '\n'; i++) {
		err = read(sock,&c,ES_MAX_CHAR);

		if (err == 0) {
			sprintf(easy_sock_err_msg,"End of file reached reading double.\n");
			err_handler(EOF);
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error reading double: %i - %s\n",easy_sock_err,strerror(errno));
			err_handler(easy_sock_err);
		}
		
		d[i] = c;
	}
	
	return atof(d);
}

/*
 * This is a function that read a string (as chars) from a socket.
 * return a new 'mallocated' string or NULL if an error occours.
 *
 * Details: this function reads first the string length (as char) then
 *          use malloc() to allocate new string so it can be read.
 *          Remember! It's your own responsibility free() the string.
 */
char* read_string_c(int sock) {
	size_t len;
	char slen[ES_MAX_LEN];
	char c = 'T';
	int i;
	ssize_t err;
	char* string;
	
	memset(slen,0,ES_MAX_LEN);
	
	for (i = 0; i < ES_MAX_LEN && c != 0 && c != '\n'; i++) {
		err = read(sock,&c,ES_MAX_CHAR);

		if (err == 0) {
			sprintf(easy_sock_err_msg,"End of file reached reading string length.\n");
			err_handler(EOF);
			return NULL;
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error reading string length: %i - %s\n",easy_sock_err,strerror(errno));
			err_handler(easy_sock_err);
			return NULL;
		}

		if (isdigit(c))
			slen[i] = c;
		else if (c != 0 && c != '\n') {
			sprintf(easy_sock_err_msg,"Error reading string length: isdigit(%c) == FALSE\n",c);
			err_handler(1000+c);
		}
			
	}
	
	len = atoi(slen);
	string = (char*)malloc(len+1);
	memset(string,0,len+1);
	
	err = read(sock,string,len);

	if (err == 0) {
		sprintf(easy_sock_err_msg,"End of file reached reading string.\n");
		err_handler(EOF);
		return NULL;
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error reading string: %i - %s\n",easy_sock_err,strerror(errno));
		err_handler(easy_sock_err);
		return NULL;
	}
	string[len] = (char)0;
	
	return (string);
}

/*
 * This is a function that write a char (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 * For compatibility only: works exactly like write_char().
 */
int write_char_c(int sock, char c) {
	return (write_char(sock,c));
}

/*
 * This is a function that write a short (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_short_c(int sock, short s) {
	ssize_t err;
	char ss[ES_MAX_SHORT];
	int i;
	
	memset(ss,0,ES_MAX_SHORT);
	sprintf(ss,"%hi",s);

	for (i = 0; i < ES_MAX_SHORT && ss[i] != 0; i++) {
		err = write(sock,&ss[i],ES_MAX_CHAR);

		if (err == 0) {
			easy_sock_err = 0;
			sprintf(easy_sock_err_msg,"No short written: why?\n");
			return 0;
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error writing short: %i - %s\n",easy_sock_err,strerror(errno));
			return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
		}
	}
	err = write(sock,"\n",ES_MAX_CHAR);

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No short-end written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing short: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}

	return (err);
}

/*
 * This is a function that write an integer (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_int_c(int sock, int j) {
	ssize_t err;
	char si[ES_MAX_INT];
	int i;
	
	memset(si,0,ES_MAX_INT);
	sprintf(si,"%i",j);

	for (i = 0; i < ES_MAX_INT && si[i] != 0; i++) {
		err = write(sock,&si[i],ES_MAX_CHAR);

		if (err == 0) {
			easy_sock_err = 0;
			sprintf(easy_sock_err_msg,"No int written: why?\n");
			return 0;
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error writing int: %i - %s\n",easy_sock_err,strerror(errno));
			return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
		}
	}
	err = write(sock,"\n",ES_MAX_CHAR);

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No int-end written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing int: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}

	return (err);
}

/*
 * This is a function that write a long (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_long_c(int sock, long l) {
	ssize_t err;
	char sl[ES_MAX_LONG];
	int i;
	
	memset(sl,0,ES_MAX_LONG);
	sprintf(sl,"%li",l);

	for (i = 0; i < ES_MAX_LONG && sl[i] != 0; i++) {
		err = write(sock,&sl[i],ES_MAX_CHAR);

		if (err == 0) {
			easy_sock_err = 0;
			sprintf(easy_sock_err_msg,"No long written: why?\n");
			return 0;
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error writing long: %i - %s\n",easy_sock_err,strerror(errno));
			return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
		}
	}
	err = write(sock,"\n",ES_MAX_CHAR);

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No long-end written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing long: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write a float (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_float_c(int sock, float f) {
	ssize_t err;
	char sf[ES_MAX_FLOAT];
	int i;
	
	memset(sf,0,ES_MAX_FLOAT);
	sprintf(sf,"%f",f);

	for (i = 0; i < ES_MAX_FLOAT && sf[i] != 0; i++) {
		err = write(sock,&sf[i],ES_MAX_CHAR);

		if (err == 0) {
			easy_sock_err = 0;
			sprintf(easy_sock_err_msg,"No float written: why?\n");
			return 0;
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error writing float: %i - %s\n",easy_sock_err,strerror(errno));
			return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
		}
	}
	err = write(sock,"\n",ES_MAX_CHAR);

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No float-end written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing float: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write a double (as chars) to a socket.
 * return a number > 0 if ok, or a number <= 0 if an error occours.
 */
int write_double_c(int sock, double d) {
	ssize_t err;
	char sd[ES_MAX_DOUBLE];
	int i;
	
	memset(sd,0,ES_MAX_DOUBLE);
	sprintf(sd,"%f",d);

	for (i = 0; i < ES_MAX_DOUBLE && sd[i] != 0; i++) {
		err = write(sock,&sd[i],ES_MAX_CHAR);

		if (err == 0) {
			easy_sock_err = 0;
			sprintf(easy_sock_err_msg,"No double written: why?\n");
			return 0;
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error writing double: %i - %s\n",easy_sock_err,strerror(errno));
			return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
		}
	}
	err = write(sock,"\n",ES_MAX_CHAR);

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No double-end written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing double: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}

/*
 * This is a function that write a string (as chars) to a socket.
 * return the number of written chars, or a number < 0 if an error occours.
 *
 * Details: this function writes first the string length (as chars) then
 *          the string itself, so read_string_c() can read it.
 */
int write_string_c(int sock, char* string) {
	ssize_t err;
	size_t len = strlen(string);
	char slen[ES_MAX_LEN];
	int i;
	
	memset(slen,0,ES_MAX_LEN);
	sprintf(slen,"%i",len);

	for (i = 0; i < ES_MAX_LEN && slen[i] != 0; i++) {
		err = write(sock,&slen[i],ES_MAX_CHAR);

		if (err == 0) {
			easy_sock_err = 0;
			sprintf(easy_sock_err_msg,"No string length written: why?\n");
			return 0;
		} else if (err < 0) {
			easy_sock_err = errno;
			sprintf(easy_sock_err_msg,"Error writing string length: %i - %s\n",easy_sock_err,strerror(errno));
			return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
		}
	}
	err = write(sock,"\n",ES_MAX_CHAR);

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No string-length-end written: why?\n");
		return 0;
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing string length: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	err = write(sock,string,len);

	if (err == 0) {
		easy_sock_err = 0;
		sprintf(easy_sock_err_msg,"No string written: why?\n");
	} else if (err < 0) {
		easy_sock_err = errno;
		sprintf(easy_sock_err_msg,"Error writing string: %i - %s\n",easy_sock_err,strerror(errno));
		return (easy_sock_err <= 0 ? (easy_sock_err) : -(easy_sock_err));
	}
	
	return (err);
}




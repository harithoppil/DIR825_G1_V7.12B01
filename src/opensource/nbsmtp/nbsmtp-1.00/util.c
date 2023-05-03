/**
 * \file util.c
 * \brief Util functions for string management
 *
 * \author Fernando J. Pereda <ferdy@ferdyx.org>
 * \author Ricardo Cervera Navarro <ricardo@zonasiete.org>
 *
 * This is part of nbsmtp. nbsmtp is free software;
 * you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * nbsmtp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nbsmtp; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * See COPYING for details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include "util.h"

/**
 * \brief Wheter we should accept LOG_DEBUG messages or not. If debug = 2 we will print all
 * messages to stdout
 */
int debug = 0;

/**
 * \brief Increases memory for a string_t
 *
 * \param[in,out]	buffer	Pointer to string_t
 * \param[in]		incr	Number of bytes (chars) to increment buffer size
 */
void str_incr_space(string_t *buffer, int incr)
{
	int i = buffer->len;
	buffer->str = realloc(buffer->str, buffer->len+(incr*sizeof(char)));
	buffer->len += incr;

	for (; i < buffer->len; i++ )
	{
		buffer->str[i] = 0;
	}
}

/**
 * \brief Frees a string_t element
 *
 * \param[out] buffer Pointer to string_t
 */
void str_free(string_t *buffer)
{
	if (buffer->len > 0)
	{
		free(buffer->str);
		buffer->len = 0;
	}
}

/**
 * \brief Initialize a string_t element
 *
 * \param[out]	buffer	Pointer to string_t
 * \param[in]	size	Num of chars that will be reserved
 */
void str_init(string_t *buffer, int size)
{
	buffer->str = malloc((size+1)*sizeof(char));
	buffer->len = size;
}

/**
 * \brief Generates a date with the RFC822 (ARPA Internet Text) format
 *
 * \param[out] buf Where to store the string
 */
void arpadate(char *buf)
{
	struct tm *date;
	time_t now;
	const char *const format = "%a, %e %b %Y %H:%M:%S %z (%Z)";

	now = time(NULL);

	date = localtime((const time_t *)&now);
	strftime(buf,40,format,date);
}

/**
 * \brief Logs a message to syslog, checks for debug if the priority is LOG_DEBUG
 *
 * \param[in] priority	An int with the message priority (LOG_INFO,LOG_DEBUG,LOG_ERR...)
 * \param[in] format	A pointer to the message format
 * \param[in] ...	Variable list.
 */
void log_msg(int priority, char *format, ...)
{
	static int mask_next = 0;
	char buffer[BUFSIZ];
	char *p;
	va_list list;

	/*
	 * If we are receiving a LOG_DEBUG message, we check for debug
	 * to write it.
	 */
	if ((priority==LOG_DEBUG) && (debug==0))
	{
		return;
	}

	/*
	 * Copy in buffer the result of parsing format with list
	 * up to BUFSIZ bytes
	 */
	va_start(list,format);
	vsnprintf(buffer,BUFSIZ,format,list);
	va_end(list);

	/*
	 * Strip \r and \n so we don't mess syslog
	 */
	if ((p = strchr(buffer,'\r')) != NULL)
	{
		*p = '\0';
	}

	if ((p = strchr(buffer,'\n')) != NULL)
	{
		*p = '\0';
	}

	/*
	 * Remove passwords (they are only sent with debug>0)
	 */
	if (debug>0 && debug<3)
	{
		if (mask_next==1)
		{
			p = NULL;

			if (buffer[2]=='>')
			{
				/* Skip the ' ' */
				p = strchr(buffer,' ')+1;
				mask_next = 0;
			}
		}
		else if ((p = strstr(buffer,"AUTH PLAIN ")) != NULL)
		{
			p += strlen("AUTH PLAIN ");
		}
		else if ((p = strstr(buffer,"AUTH LOGIN ")) != NULL)
		{
			p += strlen("AUTH LOGIN ");
			mask_next = 1;
		}
		else
		{
			p = NULL;
		}

		if (p!=NULL)
		{
			int k;

			for ( k = 0 ; k < 10 && *p ; k++ , p++ )
			{
				*p='X';
			}

			*p = (char)NULL;
		}
	}

	if (debug==0 || debug==1)
	{
		openlog("nbSMTP", LOG_PID, LOG_MAIL);
		syslog(priority,"%s",buffer);
		closelog();
	}
	else /* if (debug==2) */
	{
		fprintf(stderr,"%s\n",buffer);
	}
}

/**
 * \brief Prints an usage string
 *
 * \param[in] prog A pointer to argv[0]
 */
void print_usage(char *prog)
{
	printf("Usage: %s -f from@address.com -h relayhost [OPTIONS] (use -H for help)\n",prog);
}

/**
 * \brief Prints information on how to use nbsmtp and some Copyright info.
 *
 * \param[in] prog A pointer to argv[0]
 */
void print_help(char *prog)
{
	int i = 0;

	printf("nbSMTP %s\n\n",PACKAGE_VERSION);

	printf("Features compiled-in: ");

	puts(""
#ifdef HAVE_SSL
			"SSL "
#endif
#ifdef HAVE_INET6
			"IPv6 "
#endif
#ifdef HAVE_DEBUG
			"DEBUG "
#endif
#ifdef HAVE_OSX
			"OSX "
#endif
	      );

	putchar('\n');

	printf("nbSMTP comes with ABSOLUTELY NO WARRANTY\n\n");

	do
	{
		printf("%s\n",authors[i]);
	} while (authors[++i]);

	print_usage(prog);
	printf("  -d\tdomain to send in HELO request\n");
	printf("  -f\temail address to send in MAIL FROM request\n");
	printf("  -h\thost to relay mail to\n");
	printf("  -p\tport to connect to\n");
#ifdef HAVE_SSL
	printf("  -s\tuse SSL to transfer mail\n");
	printf("  -S\tuse STARTTLS to connect to server\n");
#endif
	printf("  -U\tSASL user\n");
	printf("  -P\tSASL password\n");
	printf("  -M\t{l,p} SASL mechanism to use: l - login (default) , p - plain\n");
	printf("  -D\tenable debug to syslog (LOG_DEBUG)\n");
	printf("  -N\tdo not read system-wide config file\n");
	printf("  -n\tdo not read local config file\n");
	printf("  -V\tlog to stderr instead of using syslog (implies -D)\n");
	printf("  -v\tprint version and exit\n");
	printf("  -c\tuse an additional config file\n");
	printf("  -H\tthis help message\n");

	printf("  -A\tthis is attch file\n");
	printf("  -a\tthis is email file\n");
	printf("  -t\tto email\n");
	printf("  -T\temail subject\n");

	printf("\nSend bug reports and comments to <%s>.\n\n",PACKAGE_BUGREPORT);
}

/**
 * \file smtp.c
 * \brief This file has SMTP related code
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
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#include "fdutil.h"
#include "smtp.h"
#include "util.h"

/**
 * \brief Las message from the server
 */
char *smtp_stored_message;

/**
 * \brief Calls fd_puts appending CR LF
 *
 * \param[in] serverinfo	A pointer to a servinfo_t struct with the socket information
 * \param[in] str		A pointer to the string to be written
 * \returns We return what fd_puts returns
 * \see fd_puts()
 */
ssize_t smtp_write(servinfo_t *serverinfo, char *str)
{
	char *buf;
	ssize_t ret;

	asprintf(&buf,"%s\r\n",str);

	ret = fd_puts(serverinfo,buf,strlen(buf));
	log_msg(LOG_DEBUG,"[->] %s",buf);
	free(buf);

	return ret;
}

/**
 * \brief Returns the last message from the server
 *
 * \returns A pointer to the string with the last message from the server
 */
char *smtp_last_message()
{
	return smtp_stored_message;
}

/**
 * \brief Gets a line from a socket an returns its first digit ( borrowed from sSMTP )
 *
 * \param[in] 	serverinfo	A pointer to a servinfo_t struct with the socket information
 * \param[out] 	response	A pointer where we will store the line read
 * \returns The first digit of the line we read
 * \see fd_gets()
 */
int smtp_read(servinfo_t *serverinfo, char *response)
{
	char *p;

	do
	{
		if (fd_gets(response,BUF_SIZE,serverinfo)==NULL)
		{
			return 0;
		}

		log_msg(LOG_DEBUG,"[<-] %s",response);
            
            if(strstr(response,"PLAIN"))
                {
                    log_msg(LOG_DEBUG,"======================We get PLAIN===\n");
                    serverinfo->auth_mech=1;
                }
            else if(strstr(response,"CRAM-MD5"))
                {
                    serverinfo->auth_mech=2;
                    log_msg(LOG_DEBUG,"======================We get CRAM-MD5===\n");
                }
	} while (response[3]=='-');


	/* Free last message if neccesary */
	if (smtp_stored_message!=NULL)
	{
		free(smtp_stored_message);
	}


	/* Find the first space (numbers finish there) */
	p = strchr(response,' ');

	/* Avoid possible missbehavior */
	if (p != NULL)
	{
		p++;

		smtp_stored_message = (char *)strdup(p);
	}
	else
	{
		smtp_stored_message = (char *)strdup("");
	}

	return(atoi(response)/100); /* response[0] ? */
}

/**
 * \brief Gets a line using smtp_read and if it returns 2 we return 1 (borrowed from sSMTP)
 *
 * \param[in] serverinfo A pointer to a servinfo_t struct with the socket info
 * \return 1 if smtp_read returns 2 and 0 in other case
 * \see smtp_read()
 */
int smtp_okay(servinfo_t *serverinfo)
{
	char response[BUF_SIZE];

	return((smtp_read(serverinfo,response)==2) ? 1 : 0);
}

/**
 * \brief Writes data to the socket (actually it writes the message body and headers)
 *
 * \param[in] serverinfo	A pointer to a servinfo_t structure with the information needed
 * \param[in] msg 		A pointer to the buffer with the message
 * \param[in] length 		An integer with the number of bytes to be written
 * \return 1 in case of error, 0 if everything goes allright
 */
int smtp_write_data(servinfo_t *serverinfo, char *msg, int length)
{
	char *p;
	int i = 0; /* Avoid segfault if msg[0]=='\n' or msg[0]=='.' */
	char r = '\r';
	char dot = '.';

	for ( p = msg ; *p && i < length; p++ )
	{
		if (*p=='\n')
		{
			if (i>0 && *(p-1)!='\r')
			{
				if (fd_putc(serverinfo,&r)<1)
				{
					return 1;
				}
			}
			else if (i==0)
			{
				if (fd_putc(serverinfo,&r)<1)
				{
					return 1;
				}
			}
		}
		else if (*p=='.' && i>0 && *(p-1)=='\n')
		{
			if (fd_putc(serverinfo,&dot)<1)
			{
				return 1;
			}
		}

		if (fd_putc(serverinfo,p)<1)
		{
			return 1;
		}

		i++;
	}

	return 0;
}

/**
 * \brief Cleans up the smtp module
 */
void smtp_exit(void)
{
	if (smtp_stored_message)
	{
		free(smtp_stored_message);
	}

	return;
}

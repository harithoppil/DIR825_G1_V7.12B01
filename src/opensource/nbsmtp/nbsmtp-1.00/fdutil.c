/**
 * \file fdutil.c
 * \brief This file has support for writing and readding to and from the socket
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
#include <string.h>

#ifdef HAVE_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include "fdutil.h"
#include "servinfo.h"
#include "util.h"

/**
 * \brief Gets a character from a socket
 *
 * \param serverinfo A pointer to a servinfo_t structure with the needed info
 * \param c A pointer where it'll store what was read from the socket
 * \return What read (or SSL_read) returns
 */
ssize_t fd_getc(servinfo_t *serverinfo, char *c)
{
#ifdef HAVE_SSL
	if (serverinfo->use_tls==True && serverinfo->using_tls==True)
	{
		return(SSL_read(serverinfo->ssl,c,1));
	}
#endif
	return(read(serverinfo->sockfd,c,1));
}

/**
 * \brief Gets a number of chars from a socket
 *
 * \param[out]	buf		A pointer where we will store what we read
 * \param[in]	size		The number of chars to read
 * \param[in]	serverinfo	A pointer to a servinfo_t with the needed info
 * \return A pointer to the string read
 */
char *fd_gets(char *buf, int size, servinfo_t *serverinfo)
{
	int i;
	char c;

	memset(buf,0,size);

	for ( i=0 ; ((i < size)&&(fd_getc(serverinfo,&c)==1)) ; i++ )
	{
		if (c=='\r')
		{
			continue;
		}
		else if (c=='\n')
		{
			break;
		}
		else
		{
			buf[i] = c;
		}
	}
	buf[i] = (char)NULL;

	return buf;
}

/**
 * \brief Puts a number of chars to a socket
 *
 * \param[in] serverinfo	A pointer to a servinfo_t structure with the information of the socket
 * \param[in] buf 		A pointer to the string to be written
 * \param[in] count	Number of chars to write to the socket
 * \return We return what write (or SSL_write) returns
 */
ssize_t fd_puts(servinfo_t *serverinfo, const char *buf, size_t count)
{
#ifdef HAVE_SSL
	if(serverinfo->use_tls==True && serverinfo->using_tls==True)
	{
		return(SSL_write(serverinfo->ssl,buf,count));
	}
#endif
	return(write(serverinfo->sockfd,buf,count));
}

/**
 * \brief Write a character to the socket
 *
 * \param[in] serverinfo	A pointer to a servinfo_t sctructure with the server information
 * \param[in] c			A pointer with the character we will write
 * \return We return what write (or SSL_write) returns
 */
ssize_t fd_putc(servinfo_t *serverinfo, char *c)
{
#ifdef HAVE_SSL
	if (serverinfo->use_tls==True && serverinfo->using_tls==True)
	{
		return(SSL_write(serverinfo->ssl,c,1));
	}
#endif	
	return(write(serverinfo->sockfd,c,1));
}

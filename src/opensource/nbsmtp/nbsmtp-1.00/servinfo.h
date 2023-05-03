/**
 * \file servinfo.h
 * \brief servinfo_t struct declaration
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

#ifndef SERVINFO_H_
#define SERVINFO_H_

#ifdef HAVE_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

/* Type declaration */

typedef enum bool_e{False=0,True=1} bool_t;

/**
 * \brief Structure with data about the server, and the connection
 */
struct servinfo {
	char *host;		/**< Name of the relay host */
	char *fromaddr;		/**< Address to use in the MAIL FROM command */
	char *domain;		/**< Domain to send in HELO/EHLO command */
	char *auth_user;	/**< User to use in SASL authentication */
	char *auth_pass;	/**< Password to use in SASL authentication */
	char *to;
	int auth_mech;		/**< Mechanism to use in SASL authentication */
	int port;		/**< Port to use to connect to the server */
	int sockfd;		/**< Socket descriptor */
	int num_rcpts;		/**< Number of recipients */
#ifdef HAVE_SSL
	bool_t use_tls;		/**< Whether we should use SSL/TLS or not */
	bool_t use_starttls;	/**< Whether to use STARTTLS or not */
	bool_t using_tls;	/**< Whether we started TLS or not */
	SSL *ssl;		/**< SSL/TLS descriptor */
#endif
};

typedef struct servinfo servinfo_t;

#define SERVINFO_RELEASE_OPTION(a) if (a != NULL) \
					{ \
						free(a); \
						a = NULL; \
					}

#endif /* SERVINFO_H_ */

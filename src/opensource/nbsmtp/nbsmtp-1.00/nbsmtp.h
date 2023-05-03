/**
 * \file nbsmtp.h
 * \brief Function declaration for nbsmtp.c
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

#ifndef NBSMTP_H_
#define NBSMTP_H_

#include "servinfo.h"
#include "util.h"

/* Macro declarations */
#define BUF_SIZE 128		/** Initial buffer size */
#define BUF_INC_HDR 100 	/** How much to enlarge temporal buffer when needed */
#define MAX_MSG_LEN 1024	/** Initial max message length */
#define MAX_RCPTS 100		/** Max number of recipients */
#define MAX_RCPT_ADDR_LEN 128	/** Recipient maximun length */

#define SASL_LOGIN	0	/** AUTH LOGIN SASL mechanism */
#define SASL_PLAIN	1	/** AUTH PLAIN SASL mechanism */
#define SASL_CRAMMD5	2	/** AUTH CRAM-MD5 SASL mechanism */
#define SASL_DEFAULT	1	/** Set the default auth mechanism */

#define NBSMTP_BODY_BUF_SEND()	if (i == BUFSIZ-1) \
				{ \
					buf[i] = '\0'; \
					fd_puts(serverinfo,buf,i); \
					i = 0; \
				}

/* Function declaration */

int nbsmtp_helo(servinfo_t*);
int nbsmtp_ehlo(servinfo_t*);
int nbsmtp_auth(servinfo_t*);
int nbsmtp_rcpts(servinfo_t*,string_t*);
int nbsmtp_data(servinfo_t*,string_t*,char *,char *,char *);
int nbsmtp_data_body(servinfo_t*,char *);
int nbsmtp_header(servinfo_t*);
#ifdef HAVE_SSL
int nbsmtp_sslinit(servinfo_t*);
int nbsmtp_sslexit(servinfo_t*);
#endif

#endif /* NBSMTP_H_ */

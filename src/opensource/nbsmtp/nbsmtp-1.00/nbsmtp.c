/**
 * \file nbsmtp.c
 * \brief nbsmtp specific functions
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
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <syslog.h>
#include <pwd.h>
#ifdef HAVE_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "nbsmtp.h"
#include "util.h"
#include "base64.h"
#include "servinfo.h"
#include "smtp.h"
#include "fdutil.h"
#include "original.h"
#ifdef HAVE_SSL
#include "hmac_md5.h"
#endif

/**
 * \brief nbSMTP authors
 */
const char *const authors[] = {"Copyright (C) 2004 Fernando J. Pereda <ferdy@ferdyx.org>",
	"Copyright (C) 2004 Ricardo Cervera Navarro <ricardo@zonasiete.org>",
	"Copyright (C) 2000-2001 David Rysdam",NULL};

/**
 * \brief Send the mail body
 *
 * \param[in] serverinfo
 * \return 1 in command succes and -1 in case of error
 */
int nbsmtp_data_body(servinfo_t *serverinfo, char *sendfile)
{
	int c;				/* Current read data	*/
	int last = -1;			/* Last read data	*/
	bool_t more_data = True;	/* Continue bufferind/sending ? */
	char buf[BUFSIZ];		/* Temporal buffer to store contents of stdin */
	int i = 0;			/* Index to store chars in the buffer */
	FILE *fd;
	char file_buf[BUF_SIZE];
	
	if ((fd = fopen(sendfile,"rb"))==NULL)
	{
		/* No such file or directory */
		if (errno==ENOENT)
		{
			return 2;
		}
		perror("open");
		return 1;
	}


	while (more_data)
	{
#if (0)	
		c = fgetc(stdin);
		
		if ( c != EOF )
		{
#else
		if (fread(file_buf,1,1,fd) == 1)
			c = file_buf[0];
		else
			c = '\0';
		
		if ( c != '\0' )
		{
#endif
			/* Convert \n -> \r\n */
			if (c=='\n' && last!='\r')
			{
				buf[i++] = '\r';
			}
			/* Convert \n. -> \n.. */
			else if (c=='.' && last=='\n')
			{
				buf[i++] = '.';
			}


			/*
			 * Check if we reached the end of the buffer, write
			 * current character (c) and then check bounds again
			 */
			NBSMTP_BODY_BUF_SEND();
			buf[i++] = (char)c;
			NBSMTP_BODY_BUF_SEND();

			/* Finally save the last char in order to compare */
			last = c;
		}
		else
		{
			/*
			 * End of stream, truncate buffer and write it to
			 * the socket. Finally, break the while
			 */
			buf[i] = '\0';
			fd_puts(serverinfo,buf,i);
			more_data = False;
		}
	}

	fclose(fd);

	return 1;
}

/**
 * \brief Send the mail headers
 *
 * \param[in] serverinfo	Pointer to a servinfo_t struct with the server information
 * \param[in] msg		Pointer to the message
 * \return 1 in command success and -1 in case of error
 */
int nbsmtp_data(servinfo_t *serverinfo, string_t *msg, char *subject, char *attchfile, char *sendfile)
{
	char *local_out_buf;
	char local_in_buf[BUF_SIZE];
	char *msg_tmp;

	asprintf(&local_out_buf,"%s","DATA");

	/* 邮件 DATA 头部分 */
	if(smtp_write(serverinfo, local_out_buf)<1)
	{
		log_msg(LOG_ERR,"Error writting DATA command to the socket");
		return -1;
	}

	free(local_out_buf);
	
	/* Here the server should answer 354 so we check smtp_read to return '3' */
	if (smtp_read(serverinfo,local_in_buf)!=3)
	{
		log_msg(LOG_ERR,"An error ocurred after sending the DATA command");
		log_msg(LOG_ERR,"Server said: '%s'",smtp_last_message());

		return -1;
	}

	/* 邮件 DATA 头部分, 如时间 Received: by www.163.com (nbSMTP-1.00) for uid 500
        deeploves@163.com; Tue, 22 Jun 2010 09:43:49 +0800 (CST) */
	if (nbsmtp_header(serverinfo)<0)
	{
		return -1;
	}

#if (1)
	asprintf(&local_out_buf,"%s%s","from: ", serverinfo->fromaddr);

	/* 邮件 DATA 头部分 from: deeploves@163.com\r\n */
	if(smtp_write(serverinfo, local_out_buf)<1)
	{
		log_msg(LOG_ERR,"Error writting form: command to the socket");
		return -1;
	}

	free(local_out_buf);
#endif

#if (1)
	asprintf(&local_out_buf,"%s%s","Subject: ", subject);

	/* 邮件 DATA 头部分 subject */
	if(smtp_write(serverinfo, local_out_buf)<1)
	{
		log_msg(LOG_ERR,"Error writting DATA command to the socket");
		return -1;
	}

	free(local_out_buf);
#endif

	msg_tmp = (char *) strstr(msg->str, "\n\n");
	msg_tmp += 2;
	*msg_tmp = '\0';

	/*=========================================================*/
	printf("==%s==\n\r", msg->str);
	/*=========================================================*/

	/* 邮件 DATA 头部分, to: luyanyong@163.com */
	if ( smtp_write_data(serverinfo, msg->str, strlen(msg->str)) != 0 )
	{
		log_msg(LOG_ERR,"Error sending mail after DATA command");
		return -1;
	}

	if (nbsmtp_data_body(serverinfo, sendfile)<1)
	{
		log_msg(LOG_ERR,"Error sending mail body");

		return -1;
	}

	asprintf(&local_out_buf,"\r\n.\r\n");

	/* 邮件 DATA 尾部分, \r\n.\r\n */

	if (fd_puts(serverinfo,local_out_buf,strlen(local_out_buf))<1)
	{
		return -1;
	}

	free(local_out_buf);
	
	/* Read last 221: Ok queued as... */
	if (!smtp_okay(serverinfo))
	{
		log_msg(LOG_ERR,"Error terminating data. Server said: '%s'.",smtp_last_message());
		return -1;
	}

	return 1;
}

/**
 * \brief Send received: by header
 *
 * \param[in] serverinfo Struct with info about the connection
 * \return < 0 in case of error > 0 otherwise
 */
int nbsmtp_header(servinfo_t *serverinfo)
{
	char local_tmp_buf[BUF_SIZE];
	char *local_out_buf;
	
#ifdef HAVE_SSL
	char *tls_header_buf;
	int cipher_bits, algo_bits;
#endif
	
	arpadate(local_tmp_buf);

#ifdef HAVE_SSL
	if (serverinfo->using_tls)
	{
		cipher_bits = SSL_get_cipher_bits(serverinfo->ssl,&algo_bits);
		asprintf(&tls_header_buf,"\r\n\t(using %s with cipher %s (%d/%d bits))",
				SSL_get_cipher_version(serverinfo->ssl),
				SSL_get_cipher_name(serverinfo->ssl),
				cipher_bits,algo_bits);
	}
#endif

	asprintf(&local_out_buf,"Received: by %s (nbSMTP-%s) for uid %d%s\r\n\t%s; %s",
			serverinfo->domain,PACKAGE_VERSION,getuid(),
			
#ifdef HAVE_SSL
			(serverinfo->using_tls ? tls_header_buf : ""),
#else
			"",
#endif
			serverinfo->fromaddr,local_tmp_buf);

	printf("\n\r==%s==\n\r", local_out_buf);

	if (smtp_write(serverinfo,local_out_buf)<1)
	{
		log_msg(LOG_ERR,"Error writting Received: by header to the socket");
		return -1;
	}

	free(local_out_buf);
#ifdef HAVE_SSL
	if (serverinfo->using_tls)
	{
		free(tls_header_buf);
	}
#endif

	return 1;
}

/**
 * \brief Send Recipients to the server
 *
 * \param[in]		serverinfo	Pointer to a servinfo_t struct with the server information
 * \param[in,out]	rcpts		Pointer to a string_t matrix with all recipients
 * \return 1 in command success and -1 in case of error
 */
int nbsmtp_rcpts(servinfo_t *serverinfo, string_t *rcpts)
{
	char *local_out_buf;
	int i;
	bool_t validrcpts = False;

	for( i=0 ; i<serverinfo->num_rcpts ; i++ )
	{
		asprintf(&local_out_buf,"RCPT TO:<%s>", rcpts[i].str);

		if (smtp_write(serverinfo, local_out_buf)<1)
		{
			log_msg(LOG_ERR,"Error writting RCPT command to the socket");
			return -1;
		}

		free(local_out_buf);

		if (!smtp_okay(serverinfo))
		{
			log_msg(LOG_WARNING,"Recipient REJECTED [%s] -> '%s'",
					rcpts[i].str,smtp_last_message());
		}
		else
		{
			log_msg(LOG_INFO,"Recipient accepted [%s]",rcpts[i].str);

			validrcpts = True;
		}

		str_free(&rcpts[i]);
	}

	/* We check for, at least, one valid recipient */
	if (validrcpts==False)
	{
		log_msg(LOG_ERR,"No recipients were accepted by the server, exiting");
		return -1;
	}

	return 1;
}

/**
 * \brief Sends EHLO SMTP command
 *
 * \param[in] serverinfo Pointer to a servinfo_t struct with the server information
 * \return 1 in command success, 0 in case of error
 */
int nbsmtp_ehlo(servinfo_t *serverinfo)
{
	char *local_out_buf;

	asprintf(&local_out_buf,"EHLO %s",serverinfo->domain);

	if (smtp_write(serverinfo,local_out_buf)<1)
	{
		log_msg(LOG_ERR,"Error writing EHLO command to the socket");
		free(local_out_buf);
		return 0;
	}

	free(local_out_buf);

	if(!smtp_okay(serverinfo))
	{
		log_msg(LOG_ERR,"EHLO command failed");
		return 0;
	}

	return 1;
}

/**
 * \brief Sends HELO SMTP command
 *
 * \param[in] serverinfo Pointer to a servinfo_t struct with the server information
 * \return 1 in command success, 0 in case of error
 */
int nbsmtp_helo(servinfo_t *serverinfo)
{
	char *local_out_buf;

	asprintf(&local_out_buf,"HELO %s",serverinfo->domain);

	if(smtp_write(serverinfo,local_out_buf)<1)
	{
		log_msg(LOG_ERR,"Error writting HELO command to the socket");
		free(local_out_buf);
		return 0;
	}

	free(local_out_buf);

	if(!smtp_okay(serverinfo))
	{
		log_msg(LOG_ERR,"HELO command failed");
		return 0;
	}

	return 1;
}

/**
 * \brief Sends AUTH command to the server
 *
 * \param[in] serverinfo A pointer to a servinfo_t struct with the needed info
 * \return Returns 0 in case of error and 1 in command success
 */
int nbsmtp_auth(servinfo_t *serverinfo)
{
	char local_in_buf[BUF_SIZE];
	char local_tmp_buf[BUF_SIZE];
	char *local_out_buf;
	int len;
	int i;

	memset(local_tmp_buf,0,sizeof(local_tmp_buf));

	if (serverinfo->auth_mech==SASL_LOGIN)
	{
		to64frombits((unsigned char*)local_tmp_buf,
				(const unsigned char *)serverinfo->auth_user,
				strlen(serverinfo->auth_user));

		asprintf(&local_out_buf,"AUTH LOGIN %s",local_tmp_buf);

		if (smtp_write(serverinfo,local_out_buf)<1)
		{
			log_msg(LOG_ERR,"Error writting AUTH command to the socket");
			return 0;
		}

		free(local_out_buf);

		if (smtp_read(serverinfo,local_in_buf)!=3)
		{
			log_msg(LOG_ERR,"The server rejected the authentication method");
			log_msg(LOG_ERR,"Server said: '%s'",smtp_last_message());

			return 0;
		}

		memset(local_tmp_buf,0,sizeof(local_tmp_buf));

		to64frombits((unsigned char *)local_tmp_buf,
				(const unsigned char *)serverinfo->auth_pass,
				strlen(serverinfo->auth_pass));

		asprintf(&local_out_buf,"%s",local_tmp_buf);

		if (smtp_write(serverinfo,local_out_buf)<1)
		{
			log_msg(LOG_ERR,"Error writting the password to the socket");
			return 0;
		}

		free(local_out_buf);

		if (!smtp_okay(serverinfo))
		{
			log_msg(LOG_ERR,"The password wasn't accepted");
			log_msg(LOG_ERR,"Server said: '%s'",smtp_last_message());

			return 0;
		}
	}
	else if(serverinfo->auth_mech==SASL_PLAIN)
	{
		asprintf(&local_out_buf,"^%s^%s",serverinfo->auth_user,serverinfo->auth_pass);
		len = strlen(local_out_buf);

		for ( i = len-1 ; i >= 0 ; i-- )
		{
			if (local_out_buf[i]=='^')
			{
				local_out_buf[i]='\0';
			}
		}

		to64frombits((unsigned char *)local_tmp_buf,(const unsigned char *)local_out_buf,len);

		free(local_out_buf);

		asprintf(&local_out_buf,"AUTH PLAIN %s",local_tmp_buf);

		if (smtp_write(serverinfo,local_out_buf)<1)
		{
			log_msg(LOG_ERR,"Error writting AUTH PLAIN command to the socket");
			return 0;
		}
		
		free(local_out_buf);

		if (!smtp_okay(serverinfo))
		{
			log_msg(LOG_ERR,"Error, the authentication failed");
			log_msg(LOG_ERR,"Server said: '%s'",smtp_last_message());

			return 0;
		}
	}
#ifdef HAVE_SSL_MD5
	else if (serverinfo->auth_mech==SASL_CRAMMD5)
	{
		/*
		 * This code has been adapted from a code by Oliver Hitz <oliver@net-track.ch>
		 */
		unsigned char challenge[BUFSIZ];
		unsigned char digest[16];
		unsigned char digasc[33];
		char *decoded;
		unsigned char encoded[BUFSIZ];
		unsigned char greeting[BUFSIZ];
		static char hextab[] = "0123456789abcdef";

		local_out_buf = (char *)strdup("AUTH CRAM-MD5");

		if (smtp_write(serverinfo,local_out_buf)<1)
		{
			log_msg(LOG_ERR,"Error writting AUTH CRAM-MD5 command to the socket");
			return 0;
		}

		free(local_out_buf);

		if (smtp_read(serverinfo,local_in_buf)!=3)
		{
			/* Server rejected the auth method */
			log_msg(LOG_ERR,"The server rejected the authentication method");
			log_msg(LOG_ERR,"Server said: '%s'",smtp_last_message());

			return 0;
		}

		/* First get the greeting and decode the challenge */
		strncpy((char *)greeting,smtp_last_message(),sizeof(greeting));
		i = from64tobits((char *)challenge,(char *)greeting);

		/* Make sure challenge is '\0' ended, since from64tobits doesn't do it itself */
		challenge[i] = '\0';

		/* Perform the keyed-hashing algorithm */
		hmac_md5(challenge,strlen((char *)challenge),
				(unsigned char *)serverinfo->auth_pass,
				strlen(serverinfo->auth_pass),digest);

		/* Standard hexadecimal conversion */
		for (i = 0; i < 16; i++)
		{
			digasc[2*i] = hextab[digest[i] >> 4];
			digasc[2*i+1] = hextab[digest[i] & 0xf];
		}

		/* Always NULL-terminate digasc to avoid problems */
		digasc[32] = '\0';

		/* Create and encode the challenge response */
		asprintf(&decoded,"%s %s",serverinfo->auth_user,digasc);
		to64frombits((unsigned char*)encoded,(const unsigned char*)decoded,strlen(decoded));
		free(decoded);

		if (smtp_write(serverinfo,(char *)encoded)<1)
		{
			log_msg(LOG_ERR,"Error writting auth string to the socket");
		}

		if (!smtp_okay(serverinfo))
		{
			log_msg(LOG_ERR,"Error, the authentication failed");
			log_msg(LOG_ERR,"Server said: '%s'",smtp_last_message());

			return 0;
		}
	}
#endif

	/* Command succeded so tell the log */
	log_msg(LOG_INFO,"Authentication succeded [%s]",serverinfo->auth_user);

	return 1;
}

#ifdef HAVE_SSL
/**
 * \brief Inits SSL stuff
 *
 * \param[in,out] serverinfo A pointer to a servinfo_t struct with the needed info
 * \return Returns -1 in case of error
 */
int nbsmtp_sslinit(servinfo_t *serverinfo)
{
	SSL_CTX *ctx;
	SSL_METHOD *method;
	X509 *server_cert;

	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();
	method = SSLv23_client_method();
	ctx = SSL_CTX_new(method);

	if (!ctx)
	{
		log_msg(LOG_ERR,"Couldn't initialize SSL\n");
		return -1;
	}

	serverinfo->ssl = SSL_new(ctx);

	if (!serverinfo->ssl)
	{
		log_msg(LOG_ERR,"SSL not working");
		return -1;
	}

	SSL_set_fd(serverinfo->ssl,serverinfo->sockfd);

	if (SSL_connect(serverinfo->ssl) < 0)
	{
		log_msg(LOG_ERR,"SSL_connect failed");
		return -1;
	}

	server_cert = SSL_get_peer_certificate(serverinfo->ssl);

	if (!server_cert)
	{
		log_msg(LOG_ERR,"We didn't get a server certificate. Exiting");
		return -1;
	}

	serverinfo->using_tls = True;

	X509_free(server_cert);
	SSL_CTX_free(ctx);

	return 0;
}

/**
 * \brief Cleans up SSL stuff
 *
 * \param[in,out] serverinfo A pointer to a servinfo_t struct with the needed info
 * \return Returns -1 in case of error
 */
int nbsmtp_sslexit(servinfo_t *serverinfo)
{
	if (serverinfo->use_tls == True && serverinfo->using_tls == True)
	{
		SSL_shutdown(serverinfo->ssl);
		SSL_free(serverinfo->ssl);
		ERR_free_strings();
		ERR_remove_state(0);
		EVP_cleanup();
#ifdef HAVE_CRYPTO_CLEANUP_ALL_EX_DATA
		CRYPTO_cleanup_all_ex_data();
#endif
	}

	return 0;
}
#endif

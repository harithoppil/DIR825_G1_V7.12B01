/**
 * \file original.c
 * \brief This file has the original functions of nbSMTP coded by us.
 *
 * \author Copyright (C) 2004 Fernando J. Pereda <ferdy@ferdyx.org>
 * \author Copyright (C) 2004 Ricardo Cervera Navarro <ricardo@zonasiete.org>
 * \author Copyright (C) 2000-2001 David Rysdam
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>

#include "original.h"
#include "nbsmtp.h"
#include "smtp.h"
#include "servinfo.h"
#include "fileconfig.h"

/**
 * \brief Parses command line setting serverinfo members to its values.
 *
 * \param[in]	argc		An integer with the number of arguments
 * \param[in]	argv		A pointer to a char array holding all arguments
 * \param[out]	serverinfo	A pointer to a servinfo_t struct
 * \return Returns 0 if everything went ok, 2 if we got -H (print long_help) and 1 in case of error
 */
int parse_options(int argc,char *argv[], servinfo_t *serverinfo, char *email, char *a_file, char *s_file, char *subject)
{
	int c;
	char buffer[BUF_SIZE];
	bool_t read_syswide = True;
	bool_t read_localconf = True;

	for (c = 0 ; c < argc ; c++)
	{
		if (strncmp("-n",argv[c],strlen("-n"))==0)
		{
			read_localconf = False;
		}
		else if (strncmp("-N",argv[c],strlen("-N"))==0)
		{
			read_syswide = False;
		}
	}

	/* Parse all config files */
	if (fileconfig_parse_all(read_syswide,read_localconf,serverinfo))
	{
		perror("fileconfig_parse_all");
		return 1;
	}

	/* Then read the options */
	while ((c = getopt(argc,argv,"T:t:A:a:h:d:f:c:p:U:P:M:sSvVDHnN")) != -1)
	{
		switch(c)
		{
			case 'T':
				strcpy(subject, optarg);
		//		printf("--subject: %s\n\r", subject);
				break;
				
			case 't':
				strcpy(email, optarg);
		//		printf("--email: %s\n\r", email);
				break;

			case 'A':
				strcpy(a_file, optarg);
		//		printf("--a_file: %s\n\r", a_file);
				break;

			case 'a':
				strcpy(s_file, optarg);
		//		printf("--s_file: %s\n\r", s_file);
				break;
				
			case 'h':
				SERVINFO_RELEASE_OPTION(serverinfo->host);
				serverinfo->host = (char *)strdup(optarg);
				break;
			case 'd':
				SERVINFO_RELEASE_OPTION(serverinfo->domain);
				serverinfo->domain = (char *)strdup(optarg);
				break;
			case 'f':
				SERVINFO_RELEASE_OPTION(serverinfo->fromaddr);
				serverinfo->fromaddr = (char *)strdup(optarg);
				break;
			case 'c':
				switch (fileconfig_parse(optarg,serverinfo))
				{
					case 1:
						perror("fileconfig_parse");
						return 1;
						break;
					case 2:
						log_msg(LOG_WARNING,"Could not load '%s': File not found",optarg);
						break;
				}
//				printf("serverinfo.to=%s\n",serverinfo->to);
				strcpy(email,serverinfo->to);
//				printf("email=%s\n",email);
				break;
			case 'p':
				serverinfo->port = atoi(optarg);
				break;
			case 'D':
				debug = 1;
				break;
			case 's':
#ifdef HAVE_SSL
				serverinfo->use_tls = True;
#else
				log_msg(LOG_WARNING,"SSL support not compiled into nbSMTP, ignoring -s");
#endif
				break;
			case 'S':
#ifdef HAVE_SSL
				serverinfo->use_starttls = True;
				serverinfo->use_tls = True;
#else
				log_msg(LOG_WARNING,"SSL support not compiled into nbSMTP, ignoring -S");
#endif
				break;
			case 'U':
				SERVINFO_RELEASE_OPTION(serverinfo->auth_user);
				serverinfo->auth_user = (char *)strdup(optarg);
				break;
			case 'P':
				SERVINFO_RELEASE_OPTION(serverinfo->auth_pass);
				serverinfo->auth_pass = (char *)strdup(optarg);
				break;
			case 'M':
				switch(*optarg)
				{
					case 'l':
						serverinfo->auth_mech = SASL_LOGIN;
						break;
					case 'p':
						serverinfo->auth_mech = SASL_PLAIN;
						break;
					case 'c':
#ifdef HAVE_SSL_MD5
						serverinfo->auth_mech = SASL_CRAMMD5;
#else
						log_msg(LOG_WARNING,"CRAMMD5 only available when SSL compiled into nbSMTP, using default mechanism");
						serverinfo->auth_mech = SASL_DEFAULT;
#endif
						break;
					default:
						serverinfo->auth_mech = SASL_DEFAULT;
						break;
				}
				break;
			case 'H':
				print_help(argv[0]);
				return 2;
				break;
			case 'V':
				debug = 2;
				break;
			case 'v':
				printf("nbSMTP version %s\n",PACKAGE_VERSION);
				return 2;
				break;
			case 'n':
			case 'N':
				/* Left intentionally blank */
				break;
			case ':':
			case '?':
			default:
				print_usage(argv[0]);
				return 1;
				break;
		}
	}

	/* If domain isn't specified, use machines hostname */
	if (serverinfo->domain==NULL)
	{
		if (gethostname(buffer,MAXHOSTNAMELEN)==-1)
		{
			perror("gethostname");
			return 1;
		}
		else
		{
			serverinfo->domain = (char *)strdup(buffer);
		}
	}

	/* If from address isn't specified, build up one */
	if (serverinfo->fromaddr==NULL)
	{
		struct passwd *user = getpwuid(getuid());

		snprintf(buffer,sizeof(buffer),"%s@%s%c",user->pw_name,serverinfo->domain,'\0');

		serverinfo->fromaddr = (char *)strdup(buffer);
	}

	/* If we haven't enough info, print_usage and exit */
	if (serverinfo->domain==NULL || serverinfo->fromaddr==NULL || serverinfo->host==NULL)
	{
		print_usage(argv[0]);
		return 1;
	}

#ifdef HAVE_SSL
	if ((serverinfo->port==0)&&(serverinfo->use_tls==True)&&(serverinfo->use_starttls==False))
	{
		serverinfo->port = 465;
	}
	else if ((serverinfo->port==0)&&(serverinfo->use_tls==True)&&(serverinfo->use_starttls==True))
	{
		serverinfo->port = 25;
	}
#endif

	if (serverinfo->port==0)
	{
		serverinfo->port = 25;
	}

#ifdef HAVE_SSL
	if (serverinfo->port==465 && serverinfo->use_tls==False && serverinfo->use_starttls==False)
	{
		log_msg(LOG_WARNING,"Port specified (465) normally needs an SSL " \
				"connection but you didn't ask for it, use -s flag");
	}
#else
	if (serverinfo->port==465)
	{
		log_msg(LOG_WARNING,"Port specified (465) normally needs an SSL " \
				"connection, but SSL is not compiled in");
	}
#endif

	return 0;
}

/**
 * \brief Reads stdin into buffer (partially). more_data means message not fully read.
 *
 * \param[out] buffer A pointer to store what we read
 * \return 0 if we read the hole message 1 in other case (more data needs to be read)
 */
int buffer_mail(string_t *buffer)
{
	int i, more_data, c;

	memset(buffer->str, 0, buffer->len);

	for ( i=0; i < buffer->len-1; i++ )
	{
		c = fgetc(stdin);
		buffer->str[i] = c;
		if ( c == EOF )
		{
			buffer->str[i] = '\0'; /* End of stdin */
			return 0;
		}
	}

	more_data = (buffer->len && buffer->str[0] == '\0' ? 0 : 1); /* Empty string? */

	return more_data;
}

/**
 * \brief Finds some information we need from Message Headers and extract recipients into an array
 *
 * \param[out]	buffer		A pointer to the buffer that will hold the headers
 * \param[out]	total_rcpts	The total number of recipients
 * \return 	Pointer to a string_t array holding all recipients
 */
string_t *parse_mail(string_t *buffer, int *total_rcpts, char *vms_buffer)
{
	char *toline, *endheader;
	char *headers[] = {"to: ", "cc: ", NULL}; /* Don't include bcc cause cc and bcc are the same to stracasestr */
	char tmp_rcpt[100];
	int i=0, cur_rcpt=0, cur_pos=0, cur_header, isemladdr=False, endheaders=False, c = 0;
	int mem_arr_rcpts=5;
	string_t *rcpts;

	rcpts = malloc(mem_arr_rcpts*sizeof(string_t));

	/* Read msg from stdin and find end of headers */
	for (i=0; endheaders == False; i++)
	{
		if ( i >= buffer->len )
		{
			str_incr_space(buffer, BUF_INC_HDR);
		}

#if (0)		
		c = fgetc(stdin);
#else
		if (*vms_buffer != '\0')
			c = *vms_buffer++;
#endif

		buffer->str[i] = c;
		if (i>0 && buffer->str[i] == '\n' && buffer->str[i-1] == '\n')
		{
			endheaders = True;
		}
		if (c == EOF)
		{
			return NULL;
		}
	}

	endheader = (char *) &buffer->str[i-1];

	if(endheader == NULL)
	{
		return NULL;
	}


	for(cur_header=0; headers[cur_header] ; cur_header++)
	{
		toline = (char *)strcasestr(buffer->str, headers[cur_header]);

		if(toline)
		{
			toline+=strlen(headers[cur_header]);
		}

		for(; toline && cur_rcpt<MAX_RCPTS; toline++)
		{
			if (*toline=='\n' && *(toline-1)!=',' && *(toline-1)!=';' && *(toline-1)!=' ' && *(toline+1)!='\t')
			{
				break;
			}

			if (isemladdr == True && cur_pos >= rcpts[cur_rcpt].len-1)
			{
				str_incr_space(&rcpts[cur_rcpt], 10);
			}

			if (cur_rcpt >= mem_arr_rcpts-2)
			{
				rcpts = realloc(rcpts, (mem_arr_rcpts+5)*sizeof(string_t));
				mem_arr_rcpts += 5;
			}

			if (isemladdr == False && cur_pos == 100)
			{
				/* Recipient really long, we assume rubbish and reset the counter */
				cur_pos = 0;
			}

			switch(*toline)
			{
				case ',':
				case ';':
				case ' ':
				case '\t':
					if (isemladdr == True)
					{
						rcpts[cur_rcpt++].str[cur_pos] = '\0';
					}
					cur_pos = 0;
					isemladdr = False;
					break;
				case '<':
				case '>':
					break;
				case '"':
					do {
						toline++;
					} while (*toline != '"');
					break;
				default:
					if (*toline == '@')
					{
						isemladdr = True;

						str_init(&rcpts[cur_rcpt],cur_pos+10);
						strncpy(rcpts[cur_rcpt].str,tmp_rcpt,cur_pos);

						if (cur_pos > 64)
						{
							log_msg(LOG_WARNING,"One address violates RFC2821," \
									" local-part must be shorter"\
									" than 65 characters");
						}
					}

					if (isemladdr==True)
					{
						rcpts[cur_rcpt].str[cur_pos++] = *toline;
					}
					else
					{
						tmp_rcpt[cur_pos++] = *toline;
					}

					break;
			}
		}

		if(cur_pos>0)
		{
			if (isemladdr == True)
			{
				rcpts[cur_rcpt++].str[cur_pos] = '\0';
			}
			cur_pos = 0;
			isemladdr = False;
		}
	}

	if (cur_rcpt == 0)
	{
		log_msg(LOG_ERR,"No recipients found, exiting");
		log_record(LOG_NOTICE, "[email] No recipients found, exiting.");
		return NULL;
	}

	*total_rcpts = cur_rcpt;

	return rcpts;
}

/**
 * \brief Opens a socket and sends all data to the relayhost
 *
 * \param[in] msg		A pointer to the buffer with the headers
 * \param[in] serverinfo	A pointer to a servinfo_t struct holding all information we need
 * \param[in] rcpts		An array with all recipients
 * \return 1 if something goes wrong and 0 in other case
 */
int send_mail(string_t *msg, servinfo_t *serverinfo, string_t *rcpts, char *subject, char *attchfile, char *sendfile)
{
	int (*greet_f)(servinfo_t *);
	char *local_out_buf; 		/* Only for protocol commands */

	if(get_socket(serverinfo) <= 0)
	{
		return 1;
	}

	if(!smtp_okay(serverinfo))
	{
		log_msg(LOG_ERR,"Error after connect and before sending HELO/EHLO");
		return 1;
	}

	greet_f = (serverinfo->auth_user ? nbsmtp_ehlo : nbsmtp_helo);

	if(!greet_f(serverinfo))
	{
		/* If we wrote EHLO and it failed, we try HELO w/o auth before die */
		if (serverinfo->auth_user)
		{
			if(!nbsmtp_helo(serverinfo))
			{
				return 1;
			}
			else
			{
				/* HELO worked, so we NULL all authentication vars and warn to syslog */
				serverinfo->auth_user = (char)NULL;
				serverinfo->auth_pass = (char)NULL;

				log_msg(LOG_WARNING,"nbSMTP will continue without authentication");

#ifdef HAVE_SSL
				/* Without EHLO there is no STARTTLS posibility */
				if (serverinfo->use_starttls==True)
				{
					serverinfo->use_starttls=False;
					log_msg(LOG_WARNING,"EHLO command failed, no STARTTLS will be used");
				}
#endif
			}
		}
	}

#ifdef HAVE_SSL
	if (serverinfo->use_starttls==True)
	{
		asprintf(&local_out_buf,"STARTTLS");

		if (smtp_write(serverinfo,local_out_buf)<1)
		{
			log_msg(LOG_ERR,"Error writting STARTTLS command to the socket");
			return 1;
		}

		free(local_out_buf);

		if (!smtp_okay(serverinfo))
		{
			log_msg(LOG_ERR,"STARTTLS not supported by server, exiting");
			return 1;
		}

		nbsmtp_sslinit(serverinfo);

		if(!greet_f(serverinfo))
		{
			log_msg(LOG_DEBUG,"Error after SSL connection was stablished");
			return 1;
		}
	}
#endif

	if (serverinfo->auth_user && serverinfo->auth_pass)
	{
		if(!nbsmtp_auth(serverinfo))
		{
			return 1;
		}
	}

	asprintf(&local_out_buf,"MAIL FROM:<%s>",serverinfo->fromaddr);

	if(smtp_write(serverinfo, local_out_buf)<1)
	{
		log_msg(LOG_ERR,"Error writting MAIL command to the socket");
		return 1;
	}

	free(local_out_buf);

	if(!smtp_okay(serverinfo))
	{
		log_msg(LOG_ERR,"Error issuing MAIL command");
		log_msg(LOG_ERR,"Server said: '%s'",smtp_last_message());

		return 1;
	}

	if(nbsmtp_rcpts(serverinfo,rcpts)<0)
	{
		return 1;
	}

	if(nbsmtp_data(serverinfo,msg, subject, attchfile, sendfile)<0)
	{
		return 1;
	}

	asprintf(&local_out_buf,"%s","QUIT");
	
	if(smtp_write(serverinfo, local_out_buf)<1)
	{
		return 1;
	}

	free(local_out_buf);

	/* Read bye message to avoid warning on some MTAs */
	smtp_okay(serverinfo);

	smtp_exit();

	close(serverinfo->sockfd);

#ifdef HAVE_SSL
	nbsmtp_sslexit(serverinfo);
#endif

	return 0;
}

/**
 * \brief Opens a connection to the server
 *
 * \param[in,out] serverinfo A pointer to a servinfo_t struct with the host information
 * \return 0 if an error ocurs and the socket description if everything is allright
 */
int get_socket(servinfo_t *serverinfo)
{
#ifdef HAVE_INET6
	struct addrinfo hints,*res,*res0;
	char *servname;
	int error;
#else
	struct hostent *servp;
	struct sockaddr_in addr;
#endif

#ifdef HAVE_INET6
	memset(&hints,0,sizeof(hints));

	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	asprintf(&servname,"%d",serverinfo->port);

	error = getaddrinfo(serverinfo->host,servname,&hints,&res0);

	if(error)
	{
		log_msg(LOG_DEBUG,"Error after getaddrinfo, maybe DNS problem");
	}

	serverinfo->sockfd = -1;

	for ( res = res0 ; res ; res = res->ai_next )
	{
		serverinfo->sockfd = socket(res->ai_family,res->ai_socktype,res->ai_protocol);

		if (serverinfo->sockfd<0)
		{
			continue;
		}

		if (connect(serverinfo->sockfd,res->ai_addr,res->ai_addrlen)<0)
		{
			close(serverinfo->sockfd);
			serverinfo->sockfd = -1;
			continue;
		}

		break;
	}

	if (serverinfo->sockfd<0)
	{
		log_msg(LOG_ERR,"Couldn't connect to host");
		log_record(LOG_NOTICE, "[email] Couldn't connect to host.");
		return 0;
	}

	freeaddrinfo(res0);
	free(servname);
#else
	servp = gethostbyname(serverinfo->host);

	if(servp == NULL)
	{
		return 0;
	}

	if ((serverinfo->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		return 0;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(serverinfo->port);
	addr.sin_addr = *((struct in_addr *)servp->h_addr);
	memset(&(addr.sin_zero),0,sizeof(addr.sin_zero));

	if (connect(serverinfo->sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1)
	{
		log_msg(LOG_ERR,"Couldn't connect to host");
		log_record(LOG_NOTICE, "[email] Couldn't connect to host.");
		return 0;
	}
#endif

#ifdef HAVE_SSL
	if (serverinfo->use_tls==True && serverinfo->use_starttls==False)
	{
		if (nbsmtp_sslinit(serverinfo) < 0)
		{
			return 0;
		}
	}

	log_msg(LOG_INFO,"Creating%sconnection to host (%s:%d)",
			(serverinfo->use_tls==True?" SSL ":" "),
			serverinfo->host,serverinfo->port);
#else
	log_msg(LOG_INFO,"Creating connection to host (%s:%d)",serverinfo->host,serverinfo->port);
#endif

	return serverinfo->sockfd;
}

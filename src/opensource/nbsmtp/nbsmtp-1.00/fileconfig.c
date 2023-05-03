/**
 * \file fileconfig.c
 * \brief This file parses the nbsmtprc file
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
#include <syslog.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>

#include "servinfo.h"
#include "nbsmtp.h"
#include "util.h"
#include "fileconfig.h"

#ifdef HAVE_SSL
#include "osx.h"
#endif

/**
 * \brief Parses a list of possible configuration files, this function is meant to
 * avoid another modules (such as original) having to deal with several fileconfig
 * calls
 *
 * \param[in]	read_syswide	Whether to read system-wide config file or not
 * \param[in]	read_localconf	Whether to read local config file or not
 * \param[out]	serverinfo	servinfo_t pointer to save data
 * \return 1 in case of error, otherwise 0
 */
int fileconfig_parse_all(bool_t read_syswide, bool_t read_localconf, servinfo_t *serverinfo)
{
	struct passwd *user;
	char *relative_paths[] = {".nbsmtprc",".mutt/nbsmtprc",".nbsmtp/nbsmtprc",NULL};
	char **p;
	char *local_tmp_buf;
	int state = 2;		/* Initialize to 2 so it allows the for to be executed */
#ifdef HAVE_OSX
	char *osx_current_location;
#endif

	if (read_syswide)
	{
		/* First of all parse system-wide config file */
		asprintf(&local_tmp_buf,"%s/nbsmtprc",SYSCONFDIR);
		if (fileconfig_parse(local_tmp_buf,serverinfo)==1)
		{
			perror("fileconfig_parse");
			return 1;
		}
		free(local_tmp_buf);
	}

	if (read_localconf)
	{
		/* Initialize user info */
		user = getpwuid(getuid());

		/* Then read local config files */
		for ( p = relative_paths ; *p && state==2 ; p++ )
		{
			asprintf(&local_tmp_buf,"%s/%s",user->pw_dir,*p);
			state = fileconfig_parse(local_tmp_buf,serverinfo);
			free(local_tmp_buf);

			if (state==1)
			{
				perror("fileconfig_parse");
				return 1;
			}
		}

#ifdef HAVE_OSX
		/* Get current OSX network location */
		osx_current_location = osx_location();
		/* Build the path and parse the config */
		asprintf(&local_tmp_buf,"%s/.nbsmtp/%s",user->pw_dir,osx_current_location);
		state = fileconfig_parse(local_tmp_buf,serverinfo);
		
		/* Free the buffers */
		free(osx_current_location);
		free(local_tmp_buf);

		if (state==1)
		{
			perror("fileconfig_parse");
			return 1;
		}
		else if (state==2)
		{
			/*
			 * No such file or directory so read the default config file.
			 * build the string,
			 * parse the file
			 * and free the buffer
			 */
			asprintf(&local_tmp_buf,"%s/.nbsmtp/default",user->pw_dir);
			state = fileconfig_parse(local_tmp_buf,serverinfo);
			free(local_tmp_buf);

			if (state==1)
			{
				perror("fileconfig_parse");
			}
		}
#endif
	}

	return 0;
}

/**
 * \brief Parses the configuration file
 *
 * \param[in]	path		Where to find the config file
 * \param[out]	serverinfo	servinfo_t struct to store the values
 * \return 1 in case of error, 2 if file not found, otherwise 0
 */
int fileconfig_parse(const char *path,servinfo_t *serverinfo)
{
	const char *sep = "\n\t =";
	char buf[BUF_SIZE];
	char *tempo, *opts[2], **p;
	FILE *fd;
#ifndef HAVE_STRSEP
	char *token;
#endif

	if ((fd = fopen(path,"r"))==NULL)
	{
		/* No such file or directory */
		if (errno==ENOENT)
		{
			return 2;
		}
		perror("open");
		return 1;
	}

	while(fgets(buf,sizeof(buf),fd))
	{
		/* Strip comments and malformed lines */
		if (buf[0]=='#')
		{
			continue;
		}
		else if ((tempo = strchr(buf,'#')))
		{
			*tempo = (char)NULL;
		}
		else if(strchr(buf,'=') == (char*)NULL)
		{
			continue;
		}

		/* Set helper pointers */
		p = opts;
		tempo = buf;

		/*
		 * We store both parts (key/value) on opts[]. We are
		 * also good enough to make a version using strtok for
		 * those 'weak' OSes with no strsep().
		 */
#ifdef HAVE_STRSEP
		while((*p = strsep(&tempo,sep))!=NULL)
		{
			if (**p!='\0' && ++p >= &opts[2])
			{
				break;
			}
		}
#else
		for ( token = strtok(tempo,sep) ; token ; token = strtok(NULL,sep) )
		{
			*(p++) = token;
		}
#endif

		/* Avoid segfault in case of opts[1] being NULL */
		if (opts[1]==NULL)
		{
			log_msg(LOG_WARNING,"Empty option [%s] in %s",opts[0],path);
			continue;
		}

		/* And finally we check for the var */
		if CHECK_OPTION("relayhost")
		{
			SERVINFO_RELEASE_OPTION(serverinfo->host);
			serverinfo->host = (char*)strdup(opts[1]);
		}
		else if CHECK_OPTION("fromaddr")
		{
			SERVINFO_RELEASE_OPTION(serverinfo->fromaddr);
			serverinfo->fromaddr = (char*)strdup(opts[1]);
		}
		else if CHECK_OPTION("domain")
		{
			SERVINFO_RELEASE_OPTION(serverinfo->domain);
			serverinfo->domain = (char*)strdup(opts[1]);
		}
		else if CHECK_OPTION("auth_user")
		{
			SERVINFO_RELEASE_OPTION(serverinfo->auth_user);
			serverinfo->auth_user = (char*)strdup(opts[1]);
		}
		else if CHECK_OPTION("auth_pass")
		{
			SERVINFO_RELEASE_OPTION(serverinfo->auth_pass);
			serverinfo->auth_pass = (char*)strdup(opts[1]);
		}
		else if CHECK_OPTION("to")
		{
			SERVINFO_RELEASE_OPTION(serverinfo->to);
			serverinfo->to = (char*)strdup(opts[1]);
		}
		else if CHECK_OPTION("auth_mech")
		{
			if (strlen(opts[1])==1)
			{
				log_msg(LOG_WARNING,"Setting auth_mech with a single character is " \
						"discouraged and will be removed soon. Please take a look " \
						"at nbsmtprc(5) to see the new format.");

				switch(*opts[1])
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
					default:
						serverinfo->auth_mech = SASL_DEFAULT;
						break;
				}
			}
			else
			{
				if CHECK_VALUE("plain")
				{
					serverinfo->auth_mech = SASL_PLAIN;
				}
				else if CHECK_VALUE("login")
				{
					serverinfo->auth_mech = SASL_LOGIN;
				}
				else if (CHECK_VALUE("crammd5") || CHECK_VALUE("cram-md5"))
				{
#ifdef HAVE_SSL_MD5
					serverinfo->auth_mech = SASL_CRAMMD5;
#else
					log_msg(LOG_WARNING,
							"CRAM-MD5 only available when compiled with SSL, using " \
							"default mechanism");
					serverinfo->auth_mech = SASL_DEFAULT;
#endif
				}
				else
				{
					log_msg(LOG_WARNING,"Unknown auth_mech, setting to default value");
					serverinfo->auth_mech = SASL_DEFAULT;
				}
			}
		}
		else if CHECK_OPTION("port")
		{
			serverinfo->port = atoi(opts[1]);
		}
		else if CHECK_OPTION("use_tls")
		{
#ifdef HAVE_SSL
			if CHECK_TRUE()
			{
				serverinfo->use_tls = True;
			}
			else if CHECK_FALSE()
			{
				serverinfo->use_tls = False;
			}
#else
			log_msg(LOG_WARNING,"SSL support not compiled into nbSMTP, ignoring use_tls");
#endif
		}
		else if CHECK_OPTION("use_starttls")
		{
#ifdef HAVE_SSL
			if CHECK_TRUE()
			{
				serverinfo->use_starttls = True;
				serverinfo->use_tls = True;
			}
			else if CHECK_FALSE()
			{
				serverinfo->use_starttls = False;
			}
#else
			log_msg(LOG_WARNING,"SSL support not compiled into nbSMTP, ignoring use_starttls");
#endif
		}
		else if CHECK_OPTION("debug")
		{
			debug = atoi(opts[1]);
		}
		else
		{
			log_msg(LOG_WARNING,"Unknown option [%s]",opts[0]);
		}
	}

	fclose(fd);

	return 0;
}

/**
 * \file osx.c
 * \brief The OSX module provides some Mac OS X-only features related to network locations
 *
 * \author Fernando J. Pereda <ferdy@ferdyx.org>
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
#include <string.h>
#include <stdlib.h>

#ifdef HAVE_GROWLNOTIFY
#include <stdarg.h>
#endif

#include "osx.h"


/**
 * \brief Returns the current OSX-location
 *
 * \return Returns an allocated pointer to the network location name. The
 * programmer must free the pointer.
 */
char *osx_location()
{
	FILE *scselect;
	char buf[BUFSIZ], location[BUFSIZ];
	char *p,*lp;

	if ((scselect = popen("/usr/sbin/scselect 2>&1","r"))!=NULL)
	{
		while (fgets(buf,BUFSIZ,scselect)!=NULL)
		{
			if (buf[1]=='*')
			{
				break;
			}
		}
		(void)pclose(scselect);
	}

	p = buf;
	lp = location;

	/* Forward the pointer to the first ( */
	while (*(p++) != '(');
	/* Copy everything except last ) [the string ends in )\n] */
	for (; *(p+1) && *(p+1) != '\n' ; *(lp++) = *(p++));
	/* Close it */
	*lp = (char)NULL;

	return (char *)strdup(location);
}

#ifdef HAVE_GROWLNOTIFY
/**
 * \brief Call growlnotify to display sent mail notifications
 *
 * \param [in] title	Title of the notification
 * \param [in] format	Notification format
 * \param [in] ...	Variable list
 * \return 0 on error and > 0 on command success
 */
int osx_notify(char *title, char *format, ...)
{
	FILE *growlnotify;
	char *local_tmp_buf;
	char buffer[BUFSIZ];
	va_list list;

	va_start(list,format);
	vsnprintf(buffer,BUFSIZ,format,list);
	va_end(list);

	asprintf(&local_tmp_buf,"%s -n nbSMTP --image %s %s 2>/dev/null",
			GROWLNOTIFYPATH,GROWLICONPATH,title);

	if ((growlnotify = popen(local_tmp_buf,"w"))!=NULL)
	{
		if (fputs(buffer,growlnotify)!=0)
		{
			pclose(growlnotify);
			free(local_tmp_buf);
			return 0;
		}
	}

	free(local_tmp_buf);
	pclose(growlnotify);

	return 1;
}
#endif

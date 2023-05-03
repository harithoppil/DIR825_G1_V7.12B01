/**
 * \file fileconfig.h
 * \brief This file has fileconfig.c definitions
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
#include "servinfo.h"

#define CHECK_OPTION(a) (strncasecmp(a,opts[0],strlen(a))==0)
#define CHECK_VALUE(a) (strncasecmp(a,opts[1],strlen(a))==0)
#define CHECK_TRUE() (strncasecmp("True",opts[1],strlen("True"))==0)
#define CHECK_FALSE() (strncasecmp("False",opts[1],strlen("False"))==0)

extern int debug;

int fileconfig_parse_all(bool_t,bool_t,servinfo_t*);
int fileconfig_parse(const char*,servinfo_t*);

/**
 * \file original.h
 * \brief This file has original.c declarations
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

#include "servinfo.h"
#include "util.h"

/* Original functions */
int buffer_mail(string_t*);
string_t *parse_mail(string_t*,int*,char *);
int send_mail(string_t*,servinfo_t*,string_t*,char *,char *,char *);
int get_socket(servinfo_t*);
int parse_options(int,char*[],servinfo_t*,char *,char *,char *,char *);

/**
 * \file util.h
 * \brief Declarations for util.c
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

#ifndef UTIL_H_
#define UTIL_H_

#include <stdarg.h>

/* Type declaration */

/**
 * \brief Structure for keeping strings
 */
struct string {
	char *str;		/**< Pointer to string */
	int len;		/**< Length (num of chars) of available memory */
};

typedef struct string string_t;

extern int debug;
extern const char *const authors[];

/* Function declaration"[email] %s",  */
#define log_record(fmt,args...)    syslog(LOG_LOCAL0 | fmt,##args)

void str_incr_space(string_t*,int);
void str_free(string_t*);
void str_init(string_t*,int);
void arpadate(char*);
void log_msg(int,char*,...);
void print_usage(char*);
void print_help(char*);
#endif /* UTIL_H_ */

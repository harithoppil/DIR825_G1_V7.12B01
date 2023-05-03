/**
 * \file smtp.h
 * \brief This file has all smtp.c declarations
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

#ifndef SMTP_H_
#define SMTP_H_

/* Macro declaration */
#define BUF_SIZE 128

/* Function declaration */
ssize_t smtp_write(servinfo_t*,char*);
int smtp_read(servinfo_t*,char*);
int smtp_okay(servinfo_t*);
int smtp_write_data(servinfo_t*,char*,int);
char *smtp_last_message(void);
void smtp_exit(void);

#endif /* SMTP_H_ */

/* 
   Unix SMB/CIFS implementation.
   filename handling routines
   Copyright (C) Andrew Tridgell 1992-1998
   Copyright (C) Jeremy Allison 1999-2004
   Copyright (C) Ying Chen 2000
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 * New hash table stat cache code added by Ying Chen.
 */

#include "includes.h"

extern BOOL case_sensitive;
extern BOOL case_preserve;
extern BOOL short_case_preserve;
extern BOOL use_mangled_map;

static BOOL scan_directory(const char *path, char *name,size_t maxlength,
			   connection_struct *conn,BOOL docache);

/****************************************************************************
 Check if two filenames are equal.
 This needs to be careful about whether we are case sensitive.
****************************************************************************/

static BOOL fname_equal(const char *name1, const char *name2)
{
	/* Normal filename handling */
	/**  modify  by linwenquan  **/
	if (case_sensitive)
	{
		sync();
		return(strcmp(name1,name2) == 0);
	}
	else
	{
		sync();
		return(strequal(name1,name2));
	}
}

/****************************************************************************
 Mangle the 2nd name and check if it is then equal to the first name.
****************************************************************************/

static BOOL mangled_equal(const char *name1, const char *name2, int snum)
{
	pstring tmpname;
	
	pstrcpy(tmpname, name2);
	mangle_map(tmpname, True, False, snum);
	return strequal(name1, tmpname);
}

/****************************************************************************
This routine is called to convert names from the dos namespace to unix
namespace. It needs to handle any case conversions, mangling, format
changes etc.

We assume that we have already done a chdir() to the right "root" directory
for this service.

The function will return False if some part of the name except for the last
part cannot be resolved

If the saved_last_component != 0, then the unmodified last component
of the pathname is returned there. This is used in an exceptional
case in reply_mv (so far). If saved_last_component == 0 then nothing
is returned there.

The bad_path arg is set to True if the filename walk failed. This is
used to pick the correct error code to return between ENOENT and ENOTDIR
as Windows applications depend on ERRbadpath being returned if a component
of a pathname does not exist.

On exit from unix_convert, if *pst was not null, then the file stat
struct will be returned if the file exists and was found, if not this
stat struct will be filled with zeros (and this can be detected by checking
for nlinks = 0, which can never be true for any file).
****************************************************************************/

BOOL unix_convert(pstring name,connection_struct *conn,char *saved_last_component, 
                  BOOL *bad_path, SMB_STRUCT_STAT *pst)
{
	SMB_STRUCT_STAT st;
	char *start, *end;
	pstring dirpath;
	pstring orig_path;
	BOOL component_was_mangled = False;
	BOOL name_has_wildcard = False;

	ZERO_STRUCTP(pst);

	*dirpath = 0;
	*bad_path = False;
	if(saved_last_component)
		*saved_last_component = 0;

	if (conn->printer) {
		/* we don't ever use the filenames on a printer share as a
			filename - so don't convert them */
		return True;
	}

	DEBUG(5, ("unix_convert called on file \"%s\"\n", name));

	/* 
	 * Conversion to basic unix format is already done in check_path_syntax().
	 */

	/* 
	 * Names must be relative to the root of the service - any leading /.
	 * and trailing /'s should have been trimmed by check_path_syntax().
	 */

#ifdef DEVELOPER
	SMB_ASSERT(*name != '/');
#endif

	/*
	 * If we trimmed down to a single '\0' character
	 * then we should use the "." directory to avoid
	 * searching the cache, but not if we are in a
	 * printing share.
	 * As we know this is valid we can return true here.
	 */

	if (!*name) {
		name[0] = '.';
		name[1] = '\0';
		return(True);
	}

	/*
	 * Ensure saved_last_component is valid even if file exists.
	 */

	if(saved_last_component) {
		end = strrchr_m(name, '/');
		if(end)
			pstrcpy(saved_last_component, end + 1);
		else
			pstrcpy(saved_last_component, name);
	}

	if (!case_sensitive && (!case_preserve || (mangle_is_8_3(name, False) && !short_case_preserve)))
		strnorm(name);

	start = name;
	pstrcpy(orig_path, name);

	if(!case_sensitive && stat_cache_lookup(conn, name, dirpath, &start, &st)) {
		*pst = st;
		return True;
	}

	/* 
	 * stat the name - if it exists then we are all done!
	 */

	if (SMB_VFS_STAT(conn,name,&st) == 0) {
		stat_cache_add(orig_path, name);
		DEBUG(5,("conversion finished %s -> %s\n",orig_path, name));
		*pst = st;
		return(True);
	}

	DEBUG(5,("unix_convert begin: name = %s, dirpath = %s, start = %s\n", name, dirpath, start));

	/* 
	 * A special case - if we don't have any mangling chars and are case
	 * sensitive then searching won't help.
	 */

	if (case_sensitive && !mangle_is_mangled(name) && !use_mangled_map)
		return(False);

	name_has_wildcard = ms_has_wild(start);

	/* 
	 * is_mangled() was changed to look at an entire pathname, not 
	 * just a component. JRA.
	 */

	if (mangle_is_mangled(start))
		component_was_mangled = True;

	/* 
	 * Now we need to recursively match the name against the real 
	 * directory structure.
	 */

	/* 
	 * Match each part of the path name separately, trying the names
	 * as is first, then trying to scan the directory for matching names.
	 */

	for (; start ; start = (end?end+1:(char *)NULL)) {
		/* 
		 * Pinpoint the end of this section of the filename.
		 */
		end = strchr_m(start, '/');

		/* 
		 * Chop the name at this point.
		 */
		if (end) 
			*end = 0;

		if(saved_last_component != 0)
			pstrcpy(saved_last_component, end ? end + 1 : start);

		/* 
		 * Check if the name exists up to this point.
		 */

		if (SMB_VFS_STAT(conn,name, &st) == 0) {
			/*
			 * It exists. it must either be a directory or this must be
			 * the last part of the path for it to be OK.
			 */
			if (end && !(st.st_mode & S_IFDIR)) {
				/*
				 * An intermediate part of the name isn't a directory.
				 */
				DEBUG(5,("Not a dir %s\n",start));
				*end = '/';
				return(False);
			}

			if (!end) {
				/*
				 * We just scanned for, and found the end of the path.
				 * We must return the valid stat struct.
				 * JRA.
				 */

				*pst = st;
			}

		} else {
			pstring rest;

			/* Stat failed - ensure we don't use it. */
			ZERO_STRUCT(st);
			*rest = 0;

			/*
			 * Remember the rest of the pathname so it can be restored
			 * later.
			 */

			if (end)
				pstrcpy(rest,end+1);

			/*
			 * Try to find this part of the path in the directory.
			 */

			if (ms_has_wild(start) || 
			    !scan_directory(dirpath, start, 
					    sizeof(pstring) - 1 - (start - name), 
					    conn, 
					    end?True:False)) {
				if (end) {
					/*
					 * An intermediate part of the name can't be found.
					 */
					DEBUG(5,("Intermediate not found %s\n",start));
					*end = '/';

					/* 
					 * We need to return the fact that the intermediate
					 * name resolution failed. This is used to return an
					 * error of ERRbadpath rather than ERRbadfile. Some
					 * Windows applications depend on the difference between
					 * these two errors.
					 */
					*bad_path = True;
					return(False);
				}
	      
				/* 
				 * Just the last part of the name doesn't exist.
				 * We may need to strupper() or strlower() it in case
				 * this conversion is being used for file creation 
				 * purposes. If the filename is of mixed case then 
				 * don't normalise it.
				 */

				if (!case_preserve && (!strhasupper(start) || !strhaslower(start)))		
					strnorm(start);

				/*
				 * check on the mangled stack to see if we can recover the 
				 * base of the filename.
				 */

				if (mangle_is_mangled(start)) {
					mangle_check_cache( start );
				}

				DEBUG(5,("New file %s\n",start));

				/****  add by linwenquan  *****/
				sync();
				
				return(True); 
			}

			/* 
			 * Restore the rest of the string. If the string was mangled the size
			 * may have changed.
			 */
			if (end) {
				end = start + strlen(start);
				if (!safe_strcat(start, "/", sizeof(pstring) - 1 - (start - name)) ||
				    !safe_strcat(start, rest, sizeof(pstring) - 1 - (start - name))) {
					return False;
				}
				*end = '\0';
			} else {
				/*
				 * We just scanned for, and found the end of the path.
				 * We must return a valid stat struct if it exists.
				 * JRA.
				 */

				if (SMB_VFS_STAT(conn,name, &st) == 0) {
					*pst = st;
				} else {
					ZERO_STRUCT(st);
				}
			}
		} /* end else */

		/* 
		 * Add to the dirpath that we have resolved so far.
		 */
		if (*dirpath)
			pstrcat(dirpath,"/");

		pstrcat(dirpath,start);

		/*
		 * Don't cache a name with mangled or wildcard components
		 * as this can change the size.
		 */
		
		if(!component_was_mangled && !name_has_wildcard)
			stat_cache_add(orig_path, dirpath);
	
		/* 
		 * Restore the / that we wiped out earlier.
		 */
		if (end)
			*end = '/';
	}
  
	/*
	 * Don't cache a name with mangled or wildcard components
	 * as this can change the size.
	 */

	if(!component_was_mangled && !name_has_wildcard)
		stat_cache_add(orig_path, name);

	/* 
	 * The name has been resolved.
	 */

	DEBUG(5,("conversion finished %s -> %s\n",orig_path, name));

	/****  linwenquan  *****/
	sync();
	
	return(True);
}

/****************************************************************************
 Check a filename - possibly caling reducename.
 This is called by every routine before it allows an operation on a filename.
 It does any final confirmation necessary to ensure that the filename is
 a valid one for the user to access.
****************************************************************************/

BOOL check_name(pstring name,connection_struct *conn)
{
	BOOL ret = True;

	errno = 0;

	if (IS_VETO_PATH(conn, name))  {
		/* Is it not dot or dot dot. */
		if (!((name[0] == '.') && (!name[1] || (name[1] == '.' && !name[2])))) {
			DEBUG(5,("file path name %s vetoed\n",name));
			return False;
		}
	}

	if (!lp_widelinks(SNUM(conn))) {
		ret = reduce_name(conn,name,conn->connectpath);
	}

	/* Check if we are allowing users to follow symlinks */
	/* Patch from David Clerc <David.Clerc@cui.unige.ch>
		University of Geneva */

#ifdef S_ISLNK
	if (!lp_symlinks(SNUM(conn))) {
		SMB_STRUCT_STAT statbuf;
		if ( (SMB_VFS_LSTAT(conn,name,&statbuf) != -1) &&
				(S_ISLNK(statbuf.st_mode)) ) {
			DEBUG(3,("check_name: denied: file path name %s is a symlink\n",name));
			ret = False; 
		}
	}
#endif

	if (!ret)
		DEBUG(5,("check_name on %s failed\n",name));

	return(ret);
}

/****************************************************************************
 Scan a directory to find a filename, matching without case sensitivity.
 If the name looks like a mangled name then try via the mangling functions
****************************************************************************/

static BOOL scan_directory(const char *path, char *name, size_t maxlength, 
			   connection_struct *conn,BOOL docache)
{
	void *cur_dir;
	const char *dname;
	BOOL mangled;

	mangled = mangle_is_mangled(name);

	/* handle null paths */
	if (*path == 0)
		path = ".";

	if (docache && (dname = DirCacheCheck(path,name,SNUM(conn)))) {
		safe_strcpy(name, dname, maxlength);	
		return(True);
	}      

	/*
	 * The incoming name can be mangled, and if we de-mangle it
	 * here it will not compare correctly against the filename (name2)
	 * read from the directory and then mangled by the mangle_map()
	 * call. We need to mangle both names or neither.
	 * (JRA).
	 */
	if (mangled)
		mangled = !mangle_check_cache( name );

	/* open the directory */
	if (!(cur_dir = OpenDir(conn, path, True))) {
		DEBUG(3,("scan dir didn't open dir [%s]\n",path));
		return(False);
	}

	/* now scan for matching names */
	while ((dname = ReadDirName(cur_dir))) {

		/* Is it dot or dot dot. */
		if ((dname[0] == '.') && (!dname[1] || (dname[1] == '.' && !dname[2]))) {
			continue;
		}

		/*
		 * At this point dname is the unmangled name.
		 * name is either mangled or not, depending on the state of the "mangled"
		 * variable. JRA.
		 */

		/*
		 * Check mangled name against mangled name, or unmangled name
		 * against unmangled name.
		 */

		if ((mangled && mangled_equal(name,dname,SNUM(conn))) || fname_equal(name, dname)) {
			/* we've found the file, change it's name and return */
			if (docache)
				DirCacheAdd(path,name,dname,SNUM(conn));
			safe_strcpy(name, dname, maxlength);
			CloseDir(cur_dir);
			return(True);
		}
	}

	CloseDir(cur_dir);
	return(False);
}

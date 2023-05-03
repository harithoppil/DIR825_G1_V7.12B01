#ifndef _NTFS_MBCP_H
#define _NTFS_MBCP_H

#include "types.h"
#include "layout.h"

#define MB_MAX_CODELEN 6

extern int uni2char(const wchar_t uni,unsigned char *out, int boundlen);
extern int char2uni(const unsigned char *rawstring, int boundlen,wchar_t *uni);

#endif


/*
 * $Id: squid_mswin.h,v 1.6 2007/09/23 15:21:29 serassio Exp $
 *
 * AUTHOR: Andrey Shorin <tolsty@tushino.com>
 * AUTHOR: Guido Serassio <serassio@squid-cache.org>
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */

#define ACL WindowsACL
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#if _MSC_VER == 1400
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning( disable : 4290 )
#pragma warning( disable : 4996 )
#endif
#endif

#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64
# define __USE_FILE_OFFSET64	1
#endif

#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;

typedef long pid_t;

#if defined __USE_FILE_OFFSET64
typedef int64_t off_t;
typedef uint64_t ino_t;

#else
typedef long off_t;
typedef unsigned long ino_t;

#endif

#define INT64_MAX _I64_MAX
#define INT64_MIN _I64_MIN

//#include "default_config_file.h"
/* Some tricks for MS Compilers */
#define __STDC__ 1
#pragma include_alias(<dirent.h>, <direct.h>)
#define THREADLOCAL __declspec(thread)

#elif defined(__GNUC__) /* gcc environment */

#define THREADLOCAL __attribute__((section(".tls")))

#endif

#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define alloca _alloca
#endif
#define chdir _chdir
#define dup _dup
#define dup2 _dup2
#define fdopen _fdopen
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define fileno _fileno
#define fstat _fstati64
#endif
#define ftruncate WIN32_ftruncate
#define getcwd _getcwd
#define getpid _getpid
#define getrusage WIN32_getrusage
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define lseek _lseeki64
#define memccpy _memccpy
#define mkdir(p) _mkdir(p)
#define mktemp _mktemp
#endif
#define pclose _pclose
#define pipe WIN32_pipe
#define popen _popen
#define putenv _putenv
#define setmode _setmode
#define sleep(t) Sleep((t)*1000)
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define snprintf _snprintf
#define stat _stati64
#define strcasecmp _stricmp
#define strdup _strdup
#define strlwr _strlwr
#define strncasecmp _strnicmp
#define tempnam _tempnam
#endif
#define truncate WIN32_truncate
#define umask _umask
#define unlink _unlink
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define vsnprintf _vsnprintf
#endif

#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR
#define O_APPEND        _O_APPEND

#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL

#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_RAW           _O_BINARY
#define O_TEMPORARY     _O_TEMPORARY
#define O_NOINHERIT     _O_NOINHERIT
#define O_SEQUENTIAL    _O_SEQUENTIAL
#define O_RANDOM        _O_RANDOM
#define O_NDELAY	0

#define S_IFMT   _S_IFMT
#define S_IFDIR  _S_IFDIR
#define S_IFCHR  _S_IFCHR
#define S_IFREG  _S_IFREG
#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IEXEC  _S_IEXEC

#define S_IRWXO 007
#if defined(_MSC_VER) /* Microsoft C Compiler ONLY */
#define	S_ISDIR(m) (((m) & _S_IFDIR) == _S_IFDIR)
#endif

#define	SIGHUP	1	/* hangup */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGCHLD	20	/* to parent on child stop or exit */
#define SIGUSR1 30	/* user defined signal 1 */
#define SIGUSR2 31	/* user defined signal 2 */

typedef unsigned short int ushort;
typedef int uid_t;
typedef int gid_t;


#undef ACL

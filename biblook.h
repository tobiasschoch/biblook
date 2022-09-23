/*

Copyright (C) 1992, 1993 Jeff Erickson
Copyright (C) 1993, 1994, 1995, 1998, 2000 Bill Jones
Copyright (C) 2000 Rafael Laboissiere
Copyright (C) 2022 Tobias Schoch

This file is part of biblook.

biblook is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

biblook is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with biblook; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

/* ======================== Standard header files ====================== */

/* added by Tobias Schoch <tobias.schoch@gmail.com>, 2022-09-11 */
#ifdef WITH_COLOR
#define COL_WARN    "\x1B[35m"  /* warning: magenta */
#define COL_ERR     "\x1B[31m"  /* error: red */
#define COL_OUT     "\x1B[36m"  /* output: cyan */
#define COL_IN      "\x1B[32m"  /* input: green */
#define COL_RESET   "\x1B[0m"   /* reset color */
#define FONT_ITALIC "\e[3m"
#define FONT_BOLD   "\e[1m"
#define FONT_UNDER  "\e[4m"
#define FONT_RESET  "\e[0m"
#else
#define COL_WARN    ""
#define COL_ERR     ""
#define COL_OUT     ""
#define COL_IN      ""
#define COL_RESET   ""
#define FONT_ITALIC ""
#define FONT_BOLD   ""
#define FONT_UNDER  ""
#define FONT_RESET  ""
#endif

#include <stdio.h>
#if (__STDC__ || __cplusplus || c_plusplus || HAVE_STDLIB_H)
#include <stdlib.h>
#endif /*  (__STDC__ || __cplusplus || c_plusplus || HAVE_STDLIB_H) */
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#ifndef FILENAME_MAX	  /* defined in all Standard C implementations */
#define FILENAME_MAX 1024 /* else use common UNIX value */
/*  (DEC, NeXT, Sun: 1024, IBM, SGI: 255) */
#endif

#if FILENAME_MAX < 255 /* workaround for HP-UX bug, which sets it to 14! */
#undef FILENAME_MAX
#define FILENAME_MAX 1024
#endif

#ifndef EXIT_FAILURE /* defined in all Standard C implementations */
#define EXIT_FAILURE 1
#endif

#ifndef EXIT_SUCCESS /* defined in all Standard C implementations */
#define EXIT_SUCCESS 0
#endif

#define CHAR_HIGHBIT ((char)1 << (CHAR_BIT - 1))
typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
#if UINT_MAX > 0x7fffffffL
typedef unsigned int uint32;
typedef int int32;
#else
typedef unsigned long uint32;
typedef long int32;
#endif

/* ==================== Machine-specific definitions =================== */
#ifndef MOREPATH				 /* can override at compile time */
#define MOREPATH "/usr/bin/more" /* full path name to less */
#define MORE "more" 		     /* argv[0] */
#endif							 /* MOREPATH */

#if __NeXT__
/* NeXT lacks unistd.h and malloc.h */
#if __cplusplus
extern "C"
{
    /* these routine are defined in */
    /* libc.h, but are absent from */
    /* the C++ runtime library, sigh... */
    extern uint32 htonl(uint32 hostlong);
    extern uint16 htons(uint16 hostshort);
    extern uint32 ntohl(uint32 netlong);
    extern uint16 ntohs(uint16 netshort);
#define htonl (htonl)
#define htons (htons)
#define ntohl (ntohl)
#define ntohs (ntohs)
};
#endif

#include </usr/include/bsd/libc.h> /* NB: next absolute path to work */
                                   /* around broken g++ libc.h */
#else
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif /* HAVE_MALLOC_H */
#endif /* __NeXT__ */

#if (__STDC__ || __cplusplus || c_plusplus)
#define VOID void
#else /* NOT (__STDC__ || __cplusplus || c_plusplus) */
#define VOID
#endif /* (__STDC__ || __cplusplus || c_plusplus) */

#undef bcopy
#define bcopy(source, target, length) \
    memcpy((char *)(target), (const char *)(source), length)

#if sun
#if __cplusplus
extern "C" int _filbuf(FILE *); /* missing from stdio.h */
#if !__CLCC__
extern "C" int _flsbuf(unsigned int, FILE *); /* missing from stdio.h */
#endif
extern "C" char *tempnam(const char *, const char *);
/* not defined by acc's stdio.h */
extern "C" int waitpid(int, int *, int); /* not defined by any Sun .h file */
#else									 /* NOT __cplusplus */
int _filbuf(FILE *);					   /* missing from stdio.h */
#if !__GNUC__
int _flsbuf(unsigned char, FILE *);		   /* missing from stdio.h */
#endif /* !__GNUC__ */
char *tempnam(const char *, const char *); /* not defined by acc's stdio.h */
int waitpid(int, int *, int);			   /* not defined by any Sun .h file */
#endif /* __cplusplus */
#endif /* sun */

#if DEBUG_MALLOC
/* For dynamic memory debugging. */
/* Note that the parens around free and malloc */
/* are essential to prevent macro expansion in */
/* ANSI/ISO Standard C and C++.  Erroneous */
/* preprocessors will go into an infinite loop */
/* (e.g. SunOS /usr/CC/sun4/cpp) */
#if sun
int malloc_debug(int level);
int malloc_verify(void);
#else /* NOT sun */
#define malloc_debug(level) level
#define malloc_verify() 1
#endif /* sun */
#undef free
#undef malloc
#define free(ptr) (malloc_verify(),                                              \
                   fprintf(stderr, "line %d: free(%p)\n", (int)__LINE__, (ptr)), \
                   (free)(ptr))
static void *p__; /* for malloc() debugging */
#define malloc(s) (malloc_verify(),                                \
                   p__ = (malloc)(s),                              \
                   fprintf(stderr, "line %d: malloc(%ld) -> %p\n", \
                           (int)__LINE__, (s), (p__)),             \
                   p__)
#endif /* DEBUG_MALLOC */

#if __NeXT__
static char *p_;
static char *q_;
/* NB: This is not a general definition of tempnam(), but works for
this program! */
#define tempnam(dir, pfx) (p_ = tmpnam((char *)NULL),           \
                           q_ = (char *)malloc(strlen(p_) + 1), \
                           strcpy(q_, p_))
#include <libc.h> /* for struct rusage definition */
#define waitpid(pid, statusp, options) wait4((pid), (union wait *)(statusp), \
                                             (options), (struct rusage *)0)
#endif /* __NeXT__ */

#if ardent
/* Stardent has only simple wait-for-all-children function, sigh... */
#define waitpid(pid, statusp, options) wait((int *)0)
char *getenv(const char *name); /* missing from system header files */
#endif

#if __hppa
#include <sys/wait.h>
#endif /* __hppa */

#if __host_mips && !defined(ultrix) && !defined(__sgi)
#define waitpid(pid, statusp, options) wait((int *)0)
#include <sys/wait.h>
#endif /* __host_mips && !defined(ultrix) && !defined(__sgi) */

#if __sgi
#include <sys/wait.h>
#endif /* __sgi */

#if ultrix
#include <sys/wait.h>
#endif /* ultrix */

/*
 *  provide ntohs(), ntohl(), htonl() and htons():
 */
#if defined(unix) && !defined(HAVE_NETINET_IN_H)
#define HAVE_NETINET_IN_H unix
#endif

#if __cplusplus && __GNUC__ && ultrix
#undef HAVE_NETINET_IN_H /* prototypes wrong in <netinet/in.h>, sigh... */
#define HAVE_HTON_NTOH
extern "C"
{
    u_long htonl(u_long hostlong);
    u_short htons(u_short hostsort);
    u_long ntohl(u_long netlong);
    u_short ntohs(u_short netshort);
};
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#else
#ifndef HAVE_HTON_NTOH
/* define as no-op for standalone: */
#define htonl(x) (x)
#define htons(x) (x)
#define ntohl(x) (x)
#define ntohs(x) (x)
#endif /* HAVE_HTON_NTOH */
#endif /* HAVE_NETINET_IN_H */

    /* ====================== Program-specific stuff ====================== */

#define FILE_VERSION 4	/* file format version */
#define MAJOR_VERSION 2 /* program version     */
#define MINOR_VERSION 11

#define MAXWORD 31	   /* maximum length of word indexed */
#define MAXSTRING 4095 /* maximum length of line handled */
typedef char Word[MAXWORD + 1];
typedef char String[MAXSTRING + 1];

typedef uint16 Index_s;				  /* refs per word */
typedef uint32 Index_t;				  /* entries per file */
typedef int32 Off_t;				  /* .bix file offsets */
#define INDEX_NAN (Index_t) - 1		  /* "no such index" */
#define INDEX_BUILTIN (INDEX_NAN - 1) /* used for builtin abbrevs */

/*
 * bibindex ignores single letter words automagically. so we omit
 * "a", "e", "i", "l", "n", "o", "s", "t", "y" from this list.
 * The list should be _short_ since a common word in one language
 * can be an author name in another.
 */

#define BADWORDS                                                                    \
    { /* words not to index */                                                      \
        "also", "among", "an", "and", "are", "as", "at", "by",                      \
            "for", "from", "have", "in", "into", "is",                              \
            "of", "on", "or", "over", "so", "than", "the", "to", "under", "with", 0 \
    }

/* characters which cannot appear in keywords or abbreviations */
#define NONKEYCHARS ",\n\t \"#%'()={}" /* See LaTeX book, p.143 */

/* GNU readline */
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#else
typedef char BOOL;
#define TRUE 1
#define FALSE 0
#endif /* USE_READLINE */

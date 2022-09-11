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

/* ================================================================= *\

   biblook -- look up references in a bibindexed BibTeX file

   This program was specifically developed for use with the
   computational geometry bibliographic database.  The database
   can be obtained by anonymous ftp from cs.usask.ca in the file
   `pub/geometry/geombib.tar.Z'.

   Version 1.0 written by Jeff Erickson <jeff@ics.uci.edu>, 27 Mar 92
   Version 2.0 written by Jeff Erickson <jeff@ics.uci.edu>, 17 Jun 92


   %Make% gcc -O -o biblook biblook.c

   Usage: biblook bibfile [savefile]

   -----------------------------------------------------------------

   HOW IT WORKS:

   The user can enter any of the following commands:

   f[ind] [not] <field> <words>
    Find the entries containing the given words in any field
    with a prefix matching the <field> argument.  For example,
    `a' matches both `author' and `address', and `au' matches
    `author' only.  If the <field> argument is `-' (or any
    string with no letters or numbers), match any field.

    If `not' appears before the <field>, the sense of the search
    is reversed.  The symbols `~' and `!' can be used in place
    of `not'.

    Each word is a contiguous sequence of letters and digits.
    Case is ignored; accents should be omitted; apostrophes are
    not required.  Single characters and a few common words are
    also ignored.  There is basic support for pattern matching
    using the characters ? and *, which match respectively a
    single character and a multi-character string, including
    the null string.  Thus, `algorithm??' matches `algorithmic',
    `algorithmes', and `Algorithmen'; and `*oint*' matches `point',
    `points', `pointer', `endpoint', `disjoint', etc.  However at
    present patterns beginning with ? cannot be used, as the parser
    mistakes them for a help request.

   and [not] <field> <words>
   or [not] <field> <words>
    Intersect (resp. union) the results of the given search
    with the previous search.  Several of these commands may be
    combined on a single line.  Commands are handled in the order
    in which they appear; there is no precedence.  Unlike other
    commands, and like `not', these must be spelled out
    completely.  `&' can be used in place of `and', and `|' can
    be used in place of `or'.

   d[isplay]
    Display the results of the previous search.

   s[ave] [<filename>]
    Save the results of the previous results into the specified
    file.  If <filename> is omitted, the previous save file is
    used.  If no save file has ever been specified, results are
    saved in the file specified on the command line.  If no such
    file is specified, `save.bib' is used.  If the save file
    exists, results are appended to it.

   w[hatis] <abbrev>
    Display the definition of the abbreviation <abbrev>.

   q[uit]/EOF
    Quit.

   Several commands can be combined on a single line by separating
   them with semicolons.  For example, the following command displays
   all STOC papers cowritten by Erdo"s without `Voronoi diagrams' in
   the title:

   f b stoc* | b symp* theory comp* & au erdos & ~t voronoi diagrams ; d

   -----------------------------------------------------------------
   Version history

   1.0 <jge> 3/29/92	Initial version complete
   1.1 <jge> 4/3/92	Fixed GetToken bug.
            Prompts and feedback messages sent to stderr
            instead of stdout, so results can be
            redirected to a file.

   2.0 <jge> 6/17/92	Major change in file format and commands.
    1. Allow searching on any field or all fields.
    2. More extensive boolean queries (and, or, not)
    3. New command to save results to a file
    4. New command to display results, rather than displaying
       them automatically.
    5. Allow searching for prefixes
    6. Pipe display results through $PAGER or /usr/ucb/more
   2.1 <jge> 7/8/92	Minor bug fixes.
   2.3 Bill Jones <jones@cs.usask.ca> 93/01/29
    1. Declarations common to bibindex.c and biblook.c factored out
       to new file biblook.h.
    2. Index type of (signed) short overflows early; created typedef
       Index_t, defined as unsigned short.
   2.4 Nelson H. F. Beebe <beebe@math.utah.edu> [01-Jun-1993]
    1. Remove some mixed-mode arithmetic.
    2. Add cast to return value of fork().
    3. Correct use and type of numoffsets so that code works
       if Index_t is "unsigned int" or "unsigned long".
   2.5 Erik Schoenfelder <schoenfr@ibr.cs.tu-bs.de> [14-Aug-1993]
    1. Add support for network byte order I/O, so that index files
       can be shared between big-endian and little-endian systems.
       This option is selected when HAVE_NETINET_IN_H is defined
       at compile time (default on UNIX).
       Nelson H. F. Beebe <beebe@math.utah.edu> [14-Aug-1993]
    2. Add typecast (int) in bibindex:OutputTables() array reference
       to eliminate compiler warnings.
    3. Correct code in biblook:SetComplement() to check for zero
       setmask; otherwise, a .bib file with a number of entries
       which is an exact multiple of setsize (commonly 32) will
       result in failing searches for the last setsize entries!
   2.6 <jge> 8/29/93
    1. Simplified "help" output so it fits in one screen.
    2. Made help command act like a COMMAND -- "find help" no
       longer displays the help message followed by an error
       message.
    3. Fixed error with not -- "f not" followed by "f a erdos"
       no longer finds all entries NOT written by Erdos!
    4. Added "safemalloc" routine from bibindex.
    5. Added "whatis" command to look up @strings.  Since @strings
       are now indexed as if "@string" were a field, it's now
       trivial to look up all abbreviations containing a given set
       of words.
    6. Index lists are now read in only when they are required for
       a search.  Only the CACHESIZE most recently accessed lists
       are kept; older ones are freed.
    7. Added support for BIBLOOK environment variable, which
       stores a search path for bib/bix files.  Defaults to
       BIBINPUTS, or just the current directory if neither of
       those exist.
       Nelson H. F. Beebe <beebe@math.utah.edu> [12-Sep-1993]
    8. Restore long form of help, offering short form first,
       then long form if help is asked for a second time.  The
       "telnet biblio" service on siggraph.org needs the long
       form, since remote users cannot be expected to have a
       biblook manual page.  Telling users to go read the manual
       is unfriendly.
    9. Change "/usr/ucb/more" and "more" to compile-time settable
       MOREPATH and MORE, and set Makefile to use "less" when available.
       10. Change type of numfields from char to unsigned char, and
       use sizeof numfields instead of sizeof(type of numfields).
       11. Change all exit() calls to use Standard C EXIT_xxx symbols.
       12. Put (void) casts on printf() family calls to avoid compiler
       warnings about discarded returned values.
       13. Change safemalloc() to request non-zero size when zero size
       is requested; some systems reject a malloc(0) request (e.g.
       IBM RS/6000 AIX 3.2).
       14. Change &abbrevlocs to abbrevlocs in GetTables().  This error
       went undetected on 23 O/S-compiler combinations, but was
       caught on DECstation ULTRIX with both gcc and g++, the only
       one of these systems which is little-endian.  Network byte
       order is big-endian, so ConvertToHostOrder() is a no-op
       on such systems.
       Bill Jones <jones@cs.usask.ca> 93/09/29
       15. CACHESIZE of 8192 seems to keep runtime size within ~2.5 MB.
       16. MAXRESULTS limit on set display can be enabled if wanted.
       17. bixfile kept open all the time.
   2.7 Bill Jones <jones@cs.usask.ca> 94/02/10
    1. Changes for MSDOS use, from Guenter Rote.
   2.8 Bill Jones <jones@cs.usask.ca> 95/01/19
    1. Renovations for Index_t compression, and for 64-bit-long
       portability typedefs and casts -- see bibindex for more details.
   2.9 Bill Jones <jones@cs.usask.ca> 98/03/30
    1. Pattern matching support, from Sariel Har-Peled.
   2.10 Rafael Laboissiere <rafael@laboissiere.net> 00/04/05
        1. Changed licence to the GNU General Public Licence.
        2. Added support for history and line recalling, using the GNU
           Readline & History libraries.  If the symbol USE_READLINE is
           not defined, use code from Sariel Har-Peled (only history
           management) as a fallback.
        3. Added by Pedro Aphalo <pedro.aphalo@joensuu.fi> 2000/02/10
           3.1. Added (int) cast to size_of(Word) in if
           3.2. #include<sys/wait.h> only if unix defined (biblook.h)
        4. Patch from Otfried Cheong <otfried@cs.ust.hk> for EPOC
           (the operating system of Psion palmtops and combatibles) port.
   2.11 Tobias Schoch <tobias.schoch@gmail.com> 2022/09/10
        1. Fixed compiler warnings (gcc 7.5.0). In function hlist_free,
           the 3rd argument in the call of memset is casted to int. Removed
           variable len (of type Index_t) in function FindWord because
           it is not used.
        2. Added color to console output
\* ================================================================= */

#include "biblook.h"

#ifndef USE_READLINE
static BOOL fAutoHistory = TRUE;
#endif

static char bibfile[FILENAME_MAX + 1];
static char bixfile[FILENAME_MAX + 1];

/* ======================= UTILITY FUNCTIONS ======================= */

/* the following performs string matching with '*', '?'. It takes O(mn). In fact,
   there is a way to do it in O(m + n) time by breaking the pattern into
   str free portions. Locating each portion using KMP, with scanning from left
   to right. */
static int isStrMatchPatternExt(char *Pattern, char *Str, char caseSensative)
{
    int i, j, PtrnLen, StrLen;
    char CarryIn, DFAPos[256];

    PtrnLen = strlen(Pattern);
    StrLen = strlen(Str);

    if (PtrnLen >= 150) {
        printf(COL_WARN "Pattern too long: [%s]" COL_RESET "\n", Pattern);
        exit(-1);
    }

    CarryIn = 0;

    memset(DFAPos, 0, PtrnLen + 10);
    DFAPos[0] = 1;                      /* setting the starting state to on */
    for (i = 0; i < StrLen; i++) {
        CarryIn = 0;

        for (j = 0; j < PtrnLen; j++) {
            if (!DFAPos[j]) {
                DFAPos[j] = CarryIn;
                CarryIn = ((CarryIn) && (Pattern[j] == '*'));
                continue;
            }

            DFAPos[j] = CarryIn;

            if (Pattern[j] == '*') {
                DFAPos[j + 1] = 1;      /* epsilon move */
                DFAPos[j] = 1;
                CarryIn = 1;
            } else if ((caseSensative && (Pattern[j] == Str[i])) ||
                ((!caseSensative) && (toupper(Pattern[j]) == toupper(Str[i])))
                || (Pattern[j] == '?')) {
                CarryIn = 1;
            } else {
                /* no matching for this edge in the automata */
                CarryIn = 0;
            }
        }

        DFAPos[PtrnLen] = CarryIn;
    }

    return CarryIn;
}

static int strptrcmp(char *str, char *pattern)
{
    return !isStrMatchPatternExt(pattern, str, 0);
}

/* ----------------------------------------------------------------- *\
|  void die(const char *msg1, const char *msg2)
|
|  print an error message and die
\* ----------------------------------------------------------------- */
void die(const char *msg1, const char *msg2)
{
    (void)fprintf(stderr, COL_ERR "Error:  %s %s" COL_RESET"\n", msg1, msg2);
    exit(EXIT_FAILURE);
}

/* ----------------------------------------------------------------- *\
|  void pdie(const char *msg1, const char *msg2)
|
|  Print a custom error message and a system error message, then die.
\* ----------------------------------------------------------------- */
void pdie(const char *msg1, const char *msg2)
{
    char msg[256];
    (void)sprintf(msg, COL_ERR "%s %s" COL_RESET, msg1, msg2);
    perror(msg);
    exit(EXIT_FAILURE);
}

/* ----------------------------------------------------------------- *\
|  void safefread(void *ptr, size_t size, size_t num, FILE *fp)
|
|  Read from the file, but die if there's an error.
\* ----------------------------------------------------------------- */
void safefread(void *ptr, size_t size, size_t num, FILE *fp)
{
    if (fread(ptr, size, num, fp) < num)
        pdie("Error reading", bixfile);
}

/* ----------------------------------------------------------------- *\
|  char safegetc(FILE *fp)
|
|  Get the next character safely.  Used by routines that assume that
|  they won't run into the end of file.
\* ----------------------------------------------------------------- */
char safegetc(FILE *fp)
{
    if (feof(fp))
        pdie("Error reading", bibfile);
    return getc(fp);
}

/* ----------------------------------------------------------------- *\
|  void *safemalloc(unsigned howmuch, const char *msg1, const char *msg2)
|
|  Allocate memory safely.  Used by routines that assume they won't
|  run out of memory.
\* ----------------------------------------------------------------- */
void *safemalloc(unsigned howmuch, const char *msg1, const char *msg2)
{
    register void *tmp = NULL;

    tmp = (void *)malloc((howmuch > 0) ? howmuch : 1);
    /* some malloc's fail with zero size request */
    if (tmp == NULL)
        pdie(msg1, msg2);

    return tmp;
}

/* ----------------------------------------------------------------- *\
|  void ConvertToHostOrder(int n, int s, void *xx)
|
|  Convert n elements of size s to host-byteorder
\* ----------------------------------------------------------------- */
static void ConvertToHostOrder(size_t n, size_t s, void *xx)
{
    uint16 *x = (uint16 *)xx;
    uint32 *y = (uint32 *)xx;

    if (s == sizeof(uint16)) {
        while (n-- > 0) {
            *x = ntohs(*x);
            x++;
        }
    } else {                            /* assume s == sizeof(uint32) */
        while (n-- > 0) {
            *y = ntohl(*y);
            y++;
        }
    }
}

/* ----------------------------------------------------------------- *\
|  void UncompressRefs(Index_t *list, char *p, Index_t length)
|
|  Uncompress a sequence of Index_t.  See bibindex for algorithm.
\* ----------------------------------------------------------------- */
void UncompressRefs(Index_t *list, char *p, Index_t length)
{
    Index_t prevref = (Index_t)-1;
    Index_t diff;
    char bits, highbit;
    int shift;

    while (length-- > 0) {
        diff = 0;
        shift = 0;
        do {
            bits = *p++;
            highbit = bits & CHAR_HIGHBIT;
            bits &= ~CHAR_HIGHBIT;
            diff |= bits << shift;
            shift += CHAR_BIT - 1;
        } while (highbit);
        *list = prevref + diff;
        prevref = *list++;
    }
}

void CopyrightBanner(void)
{
    (void)printf("\nbiblook version %d.%d  file version %d",
        (int)MAJOR_VERSION, (int)MINOR_VERSION, (int)FILE_VERSION);
#ifdef USE_READLINE
    (void)printf(" (GNU Readline %s support)", rl_library_version);
#endif
    (void)printf("\nCopyright (C) 1992, 1993 Jeff Erickson\n");
    (void)printf("Copyright (C) 1993, 1994, 1995, 1998, 2000 Bill Jones\n");
    (void)printf("Copyright (C) 2000 Rafael Laboissiere\n");
    (void)printf("Copyright (C) 2022 Tobias Schoch\n");
    (void)printf("This is free software with ABSOLUTELY NO WARRANTY;\n");
}

#ifdef USE_READLINE

static void History_init(void)
{
    char *str;
    char file_name[256];

    rl_readline_name = "Biblook";

    using_history();

    str = (char *)getenv("HOME");
    if (str == NULL)
        return;

    strcpy(file_name, str);
    strcat(file_name, "/.biblook.history");

    read_history(file_name);
    printf(COL_OUT "\tHistory read from: %s" COL_RESET "\n", file_name);
}

static void History_term(void)
{
    char *str;
    char file_name[256];

    str = (char *)getenv("HOME");
    if (str == NULL)
        return;

    strcpy(file_name, str);
    strcat(file_name, "/.biblook.history");

    write_history(file_name);
    printf(COL_OUT "\tHistory written in: %s" COL_RESET "\n", file_name);
}

#else

/* ============================= HISTORY =========================== *\
   Implements connected list structure for history. I knew learning
   linked lists will be handy sometime in the future.
\* ================================================================= */
typedef struct _history_list_node
{
    char line[256];
    struct _history_list_node *next;
} HListNode;
typedef HListNode *HList;

static HList hlist = NULL;
static BOOL history_f_remove_duplicate = FALSE;

#define HISTORY_SAVE_SIZE 300

static void History_clean_remark(char *line, char *remark)
{
    char *str;

    str = strchr(line, '#');
    if (str != NULL) {
        if (remark != NULL) {
            if (*remark)
                strcat(remark, ", ");
            strcat(remark, str + 1);
        }
        *str = 0;
    }
}

static void History_clean(char *line)
{
    char *str;
    int len;

    str = line;
    while (*str) {
        if (((unsigned int)*str) < 31)
            *str = ' ';
        str++;
    }

    str = line;
    len = strlen(str);
    while ((len > 0) && (str[len - 1] == ' '))
        len--;

    str[len] = 0;
}

static HList hlist_get_last(HList list)
{
    while ((list != NULL) && (list->next != NULL))
        list = list->next;

    return list;
}

static void hlist_reverse(HList *p_list)
{
    HList rev, tmp, list;

    list = *p_list;
    rev = NULL;
    while (list != NULL) {
        tmp = list->next;
        list->next = rev;
        rev = list;

        list = tmp;
    }

    *p_list = rev;
}

static void hlist_free(HList *p_list)
{
    HList list, tmp;

    if (*p_list == NULL)
        return;

    list = *p_list;
    while (list != NULL) {
        tmp = list->next;
        memset(list, sizeof(HListNode), (int)0);
        free(list);
        list = tmp;
    }

    *p_list = NULL;
}

static void hlist_truncate(HList *p_list, int count)
{
    while ((count > 0) && (*p_list != NULL)) {
        p_list = &((*p_list)->next);
        count--;
    }

    hlist_free(p_list);
}

static void history_add_inner(HList *p_list, char *line_in, BOOL f_strict)
{
    HList *p_l, list;
    char line[256];

    strcpy(line, line_in);
    History_clean(line);
    if (*line == 0)
        return;
    if (!f_strict) {
        if ((*p_list) != NULL && (strcmp(hlist_get_last((*p_list))->line,
                line) == 0))
            return;
    } else {
        list = *p_list;

        if (history_f_remove_duplicate) {
            while (list != NULL) {
                if (strcmp(list->line, line) == 0)
                    return;
                list = list->next;
            }
        }
    }

    p_l = p_list;
    while (*p_l != NULL)
        p_l = &((*p_l)->next);

    *p_l = (HListNode *)safemalloc(sizeof(HListNode),
        "Unable to allocated HListNode", "");

    strcpy((*p_l)->line, line);
    (*p_l)->next = NULL;
}

static void History_add(char *line_in)
{
    history_add_inner(&hlist, line_in, FALSE);
}

static void History_dump(void)
{
    HList list;
    int count;

    list = hlist;
    count = 0;
    while (list != NULL) {
        count++;
        printf("%5d\t%s\n", count, list->line);
        list = list->next;
    }
}

static char *History_getLine(int num)
{
    static char empty_line[2] = "";
    HList list;

    list = hlist;
    while ((list != NULL) && (num > 1)) {
        num--;
        list = list->next;
    }

    if (list != NULL && (num == 1))
        return list->line;

    printf(COL_WARN "Unable to find line %d in history" COL_RESET "\n", num);

    return empty_line;
}

static int read_number(char **p_pos)
{
    int num;
    num = 0;

    ++(*p_pos);

    while (**p_pos && (isdigit(**p_pos))) {
        num = num * 10 + ((int)(**p_pos - '0'));
        ++(*p_pos);
    }

    return num;
}

static char History_expand(char *_in_line)
{
    char line[1024], *in_line, remark[1024];
    char *dst, flag;
    int num;

    remark[0] = 0;
    flag = 0;
    in_line = _in_line;
    dst = line;
    memset(line, 0, 1024);
    while (*in_line) {
        if (*in_line == '\n') {
            in_line++;
            continue;
        }
        while (*in_line && (*in_line != '!'))
            *dst++ = *in_line++;
        if (*in_line != '!')
            break;

        flag = 1;
        num = read_number(&in_line);
        strcpy(dst, History_getLine(num));
        /*  remove remark from expanded text... */
        History_clean_remark(dst, remark);
        /* printf( "dst = [%s], line = [%s]\n", dst, line ); */

        dst += strlen(dst);
        *dst = 0;
    }

    *dst = 0;

    if (*remark) {
        strcat(dst, " # ");
        strcat(dst, remark);
    }

    strcpy(_in_line, line);

    return flag;
}

static HList history_summarize(void)
{
    HList list, pos;

    list = NULL;

    /* we want that later uses of identical lines will appear */
    hlist_reverse(&hlist);

    pos = hlist;
    list = NULL;
    while (pos != NULL) {
        history_add_inner(&list, pos->line, TRUE);
        pos = pos->next;
    }

    hlist_truncate(&list, HISTORY_SAVE_SIZE);
    hlist_reverse(&hlist);
    hlist_reverse(&list);

    return list;
}

static void History_compress(void)
{
    HList hl;
    BOOL f_tmp;

    f_tmp = history_f_remove_duplicate;
    history_f_remove_duplicate = TRUE;

    hl = history_summarize();

    history_f_remove_duplicate = f_tmp;

    hlist_free(&hlist);
    hlist = hl;
}

/* ----------------------------------------------------------------- *\
|  void History_write(char *filename)
|
|  Write the current history into a file.
\* ----------------------------------------------------------------- */
static void History_write(char *filename)
{
    FILE *ofp;
    HList list, hist_list;
#ifndef __SYMBIAN32__
    char *pager;
    char *the_tmpfile = (char *)NULL;
#if unix
    int childpid;
#else
    char *command, *command_fname;
#endif /* unix */
#endif /* __SYMBIAN32__ */

    if (filename) {
        ofp = fopen(filename, "w");
        if (!ofp) {
            (void)printf(COL_WARN "\tCan't open %s: " COL_RESET, filename);
            perror(NULL);
            return;
        }
    } else {

#ifdef __SYMBIAN32__
        ofp = stdout;
#else
        the_tmpfile = (char *)tempnam(NULL, "bibl.");
        ofp = fopen(the_tmpfile, "w");
        if (!ofp) {
            perror("\tCan't open temp file");
            return;
        }
#endif /* __SYMBIAN32__ */
    }

    hist_list = history_summarize();

    list = hist_list;
    while (list != NULL) {
        fprintf(ofp, "%s\n", list->line);
        list = list->next;
    }
    hlist_free(&hist_list);

#ifdef __SYMBIAN32__
    if (filename)
#endif
        fclose(ofp);
    if (filename) {
        (void)printf(COL_OUT "\tHistory written in \'%s\'" COL_RESET"\n",
            filename);
        return;
    }

#ifndef __SYMBIAN32__
    pager = (char *)getenv("PAGER");

#if unix
    if ((childpid = (int)fork()) != 0) {
        waitpid(childpid, (int *)0, 0);
    } else if (pager) {
        execlp(pager, pager, the_tmpfile, (char *)0);
        perror(pager);                  /* should never get here! */
        exit(EXIT_SUCCESS);
    } else {
        /* try absolute path first */
        execl(MOREPATH, MORE, the_tmpfile, (char *)0);
        /* next try to find it in PATH list */
        execlp(MORE, MORE, the_tmpfile, (char *)0);
        /* no pager available, so give up */
        perror(MOREPATH);
        exit(EXIT_SUCCESS);
    }
#elif MSDOS
    if (pager) {
        command = (char *)malloc(strlen(pager) + 2 + strlen(the_tmpfile));
        strcpy(command, pager);
        strcat(command, " ");
    } else {
        command = (char *)malloc(7 + strlen(the_tmpfile));
        strcpy(command, "MORE <");
    }

    command_fname = command + strlen(command);
    strcat(command, the_tmpfile);
    for (; *command_fname != '\0'; command_fname++)
        if (*command_fname == '/')
            *command_fname = '\\';

    if (system(command))
        perror((pager ? pager : "MORE"));
    free(command);
#else                                   /* ? other systems ? */
#endif

    unlink(the_tmpfile);
    free(the_tmpfile);                  /* malloc'ed by tempnam() */
    putchar('\n');
#endif /* __SYMBIAN32__ */
}

/* ----------------------------------------------------------------- *\
|  void History_read(char *filename)
|
|  Read the current history from a file.
\* ----------------------------------------------------------------- */
static void History_read(char *filename, BOOL fQuiet)
{
    FILE *ofp;
    HList list;
    char line[256];

    if (!filename) {
        (void)printf(COL_WARN "\tMust specify history file to read!" COL_RESET
            "\n");
        perror(NULL);
        return;
    }
    ofp = fopen(filename, "r");
    if (!ofp) {
        if (fQuiet)
            return;
        (void)printf(COL_WARN "\tCan't open %s: " COL_RESET, filename);
        perror(NULL);
        return;
    }

    list = NULL;

    while (!feof(ofp)) {
        if (!fgets(line, 254, ofp))
            break;
        history_add_inner(&list, line, FALSE);
    }

    fclose(ofp);

    hlist_free(&hlist);
    hlist = list;

    printf(COL_OUT "\tHistory read from: %s" COL_RESET "\n", filename);
}

static void History_init(void)
{
    char *str;
    char file_name[256];

    if (!fAutoHistory)
        return;
    str = (char *)getenv("HOME");

    if (str == NULL)
        return;

    strcpy(file_name, str);
    strcat(file_name, "/.biblook.history");

    History_read(file_name, TRUE);
}

static void History_term(void)
{
    char *str;
    char file_name[256];

    if (!fAutoHistory)
        return;
    str = (char *)getenv("HOME");
    if (str == NULL)
        return;

    strcpy(file_name, str);
    strcat(file_name, "/.biblook.history");

    History_write(file_name);
}

static void History_swallow_last(void)
{
    HList *p_list;

    p_list = &hlist;
    while (*p_list && ((*p_list)->next != NULL))
        p_list = &((*p_list)->next);

    if (*p_list == NULL)
        return;
    free(*p_list);
    *p_list = NULL;
}

#endif /* ifnedf USE_READLINE */

/* ============================== CACHE ============================ *\

   In the interest of saving memory, starting with version 2.6,
   index lists are now cached.  The cache is a priority queue,
   implemented as a simple binary heap, stored in an array.  The cache
   contains pointers to the real lists, and each list contains its
   rank in the heap.  The heap is prioritized by least recent use.

   Making the rank field an unsigned char almost certainly doesn't
   save anything, since the data will almost certainly have to be
   word- or long-aligned.  [2.6 [NHFB]: change rank type to int, and
   reorder struct to have largest items first, because of
   memory-alignment requirements of many architectures.]

\* ================================================================= */

#define CACHESIZE 8192

typedef struct {
    long offset;                        /* offset into index file  */
    Index_s length;                     /* length of the list	   */
    Index_s bytes;                      /* length when compressed  */
    char *list;                         /* compressed list or NULL */
    int rank;                           /* back pointer into cache */
} CachedList;

typedef struct {
    long stamp;                         /* time stamp */
    CachedList *clist;                  /* the cached list */
} CacheElement;

CacheElement cache[CACHESIZE];
long curstamp;                          /* current "time stamp" */
int cachenum;                           /* number of elements in the cache */

/* ----------------------------------------------------------------- *\
|  void InitCache(VOID)
|
|  Initialize the cache.
\* ----------------------------------------------------------------- */
void InitCache(VOID)
{
    int i;

    for (i = 0; i < CACHESIZE; i++) {
        cache[i].stamp = -1;
        cache[i].clist = NULL;
    }
    curstamp = 0;
}

/* ----------------------------------------------------------------- *\
|  void InitCachedList(CachedList *clist, FILE *ifp)
|
|  Initialize the cached list.  Read the lengths of the list from the
|  file, record the position of the list within the file, and skip
|  over the list itself.
\* ----------------------------------------------------------------- */
void InitCachedList(CachedList *clist, FILE *ifp)
{
    Index_s num;

    clist->list = NULL;

    safefread((void *)&num, sizeof(Index_s), 1, ifp);
    ConvertToHostOrder(1, sizeof(Index_s), &num);
    clist->length = num;
    safefread((void *)&num, sizeof(Index_s), 1, ifp);
    ConvertToHostOrder(1, sizeof(Index_s), &num);
    clist->bytes = num;

    clist->offset = ftell(ifp);
    if (fseek(ifp, (long)num, SEEK_CUR) != 0)
        pdie("Error reading", bixfile);
}

/* ----------------------------------------------------------------- *\
|  void FreeCache(void)
|
|  Free everything in the cache.
\* ----------------------------------------------------------------- */
void FreeCache(VOID)
{
    int i;

    for (i = 0; i < cachenum; i++)
        free(cache[i].clist->list);
}

/* ----------------------------------------------------------------- *\
|  void CheckStamp(void)
|
|  Avoid emacs's timestamp wraparound bug!!
\* ----------------------------------------------------------------- */
void CheckStamp(VOID)
{
    int i;

    if (curstamp < 0) {
        for (i = 0; i < cachenum; i++)
            cache[i].stamp = i;         /* Changes time stamp order! */
        curstamp = cachenum;
        (void)printf(COL_WARN
            "You've been running biblook a long time, haven't you?" COL_RESET
            "\n");
    }
}

/* ----------------------------------------------------------------- *\
|  void HeapBubble(int which)
|
|  Bubble the given element down into its place in the heap.
\* ----------------------------------------------------------------- */
void HeapBubble(int which)
{
    register int i;
    CacheElement tmp;

    i = which;
    tmp = cache[i];

    while (2 * i + 1 < cachenum) {          /* while I still have children */
        if ((2 * i + 2 == cachenum) ||
                (cache[2 * i + 1].stamp < cache[2 * i + 2].stamp)) {
            cache[i] = cache[2 * i + 1];    /* go to left child */
            cache[i].clist->rank = i;
            i = 2 * i + 1;
        } else {
            cache[i] = cache[2 * i + 2];    /* go to right child */
            cache[i].clist->rank = i;
            i = 2 * i + 2;
        }
    }

    cache[i] = tmp;
    cache[i].clist->rank = i;
}

/* ----------------------------------------------------------------- *\
|  void Access(CachedList *clist, FILE *ifp)
|
|  Make sure clist is in memory.  Make sure the given element is in
|  its rightful place in the cache.  If it's already there, just move
|  it.  Otherwise, insert it into the heap, deleting the oldest
|  element if the cache is already full.
\* ----------------------------------------------------------------- */
void Access(CachedList *clist, FILE *ifp)
{
    if (clist->list == NULL) {
        if (fseek(ifp, clist->offset, SEEK_SET) != 0)
            pdie("Error reading", bixfile);

        clist->list = (char *)safemalloc(clist->bytes,
            "Can't allocate index list.", "");
        safefread((void *)clist->list, sizeof(char), clist->bytes, ifp);

        if (cachenum == CACHESIZE) {        /* if cache is full... */
            free(cache[0].clist->list);     /* delete oldest element */
            cache[0].clist->list = NULL;

            cache[0] = cache[CACHESIZE - 1];
            cache[CACHESIZE - 1].clist = NULL;

            cachenum--;
            HeapBubble(0);
        }

        cache[cachenum].stamp = curstamp++;
        cache[cachenum].clist = clist;
        clist->rank = cachenum++;
    } else {
        cache[clist->rank].stamp = curstamp++;
        HeapBubble(clist->rank);
    }

    CheckStamp();
}

/* ========================== INDEX TABLES ========================= */

typedef struct {
    Word theword;
    CachedList refs;
} Index, *IndexPtr;

typedef struct {
    Word thefield;
    Index_t numwords;
    IndexPtr words;
} IndexTable;

Index_s numfields;
IndexTable *fieldtable;

Index_t numabbrevs;
Word *abbrevs;
Index_t *abbrevlocs;

Index_t numoffsets;
Off_t *offsets;

/* ----------------------------------------------------------------- *\
|  void ReadWord(FILE *ifp, Word word)
|
|  Read a "pascal" string into the given buffer
\* ----------------------------------------------------------------- */
void ReadWord(FILE *ifp, Word word)
{
    unsigned char length;

    safefread((void *)&length, sizeof(unsigned char), 1, ifp);
    if (length > MAXWORD)
        die("Index file is corrupt", "(word too long).");

    safefread((void *)word, sizeof(char), length, ifp);
    word[length] = 0;

#if DEBUG
    (void)printf("word:%02d = [%s]\n", (int)length, word);
#endif /* DEBUG */
}

/* ----------------------------------------------------------------- *\
|  void GetOneTable(FILE *ifp, IndexTable *table)
|
|  Get one index table from the file
\* ----------------------------------------------------------------- */
void GetOneTable(FILE *ifp, IndexTable *table)
{
    Index_t i;

    safefread((void *)&table->numwords, sizeof(Index_t), 1, ifp);
    ConvertToHostOrder(1, sizeof(Index_t), &table->numwords);
    table->words = (IndexPtr)safemalloc(table->numwords * sizeof(Index),
        "Can't create index table for", table->thefield);

    for (i = 0; i < table->numwords; i++) {
        ReadWord(ifp, table->words[i].theword);
        InitCachedList(&(table->words[i].refs), ifp);
    }
}

FILE *bixfp;

/* ----------------------------------------------------------------- *\
|  void GetTables(VOID)
|
|  Get the tables from the index file.
\* ----------------------------------------------------------------- */
void GetTables(VOID)
{
    int version, i;
    Index_t k;

    if (fscanf(bixfp, "bibindex %d %*[^\n]%*c", &version) < 1)
        die(bixfile, "is not a bibindex file!");
    if (version < FILE_VERSION)
        die(bixfile, "is the wrong version.\n\tPlease rerun bibindex.");
    if (version > FILE_VERSION)
        die(bixfile, "is the wrong version.\n\tPlease recompile biblook.");

    InitCache();

    safefread((void *)&numoffsets, sizeof(Index_t), 1, bixfp);
    ConvertToHostOrder(1, sizeof(Index_t), &numoffsets);
    offsets = (Off_t *)safemalloc(numoffsets * sizeof(Off_t),
        "Can't create offset table", "");

    safefread((void *)offsets, sizeof(Off_t), numoffsets, bixfp);
    ConvertToHostOrder(numoffsets, sizeof(Off_t), offsets);

    safefread((void *)&numfields, sizeof numfields, 1, bixfp);
    ConvertToHostOrder(1, sizeof numfields, &numfields);
    fieldtable = (IndexTable *)safemalloc(numfields * sizeof(IndexTable),
        "Can't create field table", "");

    for (i = 0; i < (int)numfields; i++)
        ReadWord(bixfp, fieldtable[i].thefield);
    for (i = 0; i < (int)numfields; i++)
        GetOneTable(bixfp, fieldtable + i);

    safefread((void *)&numabbrevs, sizeof(Index_t), 1, bixfp);
    ConvertToHostOrder(1, sizeof(Index_t), &numabbrevs);

    abbrevs = (Word *)safemalloc(numabbrevs * sizeof(Word),
        "Can't create abbreviation table", "");

    for (k = 0; k < numabbrevs; k++)
        ReadWord(bixfp, abbrevs[k]);

    abbrevlocs = (Index_t *)safemalloc(numabbrevs * sizeof(Index_t),
        "Can't create abbrev offset table", "");

    safefread((void *)abbrevlocs, sizeof(Index_t), numabbrevs, bixfp);
    ConvertToHostOrder(numabbrevs, sizeof(Index_t), abbrevlocs);
}

/* ----------------------------------------------------------------- *\
|  void FreeTables(void)
|
|  Free the index tables.
\* ----------------------------------------------------------------- */
void FreeTables(VOID)
{
    register int i;

    FreeCache();                        /* free all index lists in memory */

    for (i = 0; i < (int)numfields; i++)
        free(fieldtable[i].words);

    free(fieldtable);
    free(offsets);
}

/* ----------------------------------------------------------------- *\
|  Index_t FindIndex(IndexTable table, char *word, char prefix)
|
|  Find the index of a word in a table.  Return INDEX_NAN if the word
|  isn't there.  If prefix is true, return the index of the first
|  matching word.
\* ----------------------------------------------------------------- */

static void breakWord(char *word, char *prefix, char *suffix)
{
    char *pos, *p_prefix;

    pos = word;
    p_prefix = prefix;
    while (*pos && (*pos != '*') && (*pos != '?')) {
        *p_prefix = *pos;
        p_prefix++;
        pos++;
    }
    *p_prefix = 0;
    strcpy(suffix, pos);
}

static Index_t linear_scan(IndexTable table, char *prefix, char *suffix,
                           char *word, int lo)
{
    register IndexPtr words = table.words;
    register int times, len;            /* must be signed */

    (void)suffix;

    /* we perform a linear scan from lo. We need to scan at least 2 places */
    times = 0;
    len = strlen(prefix);
    while (lo < (int)table.numwords) {
        if (strncmp(prefix, words[lo].theword, len) && times > 3)
            break;
        if (!strptrcmp(words[lo].theword, word))
            return lo;

        times++;
        lo++;
    }

    return (Index_t)INDEX_NAN;
}

Index_t FindIndex(IndexTable table, char *prefix, char *suffix,
                  char *word, char _prefix)
{
    register IndexPtr words = table.words;
    register int hi, lo, mid;           /* must be signed */
    register int cmp;

    (void)_prefix;                      /* kill warnning */

    hi = table.numwords - 1;
    lo = 0;

    /* binary search for the place that matches the prefix */
    while (hi >= lo) {
        mid = (hi + lo) / 2;
        cmp = strcmp(prefix, words[mid].theword);

        if (cmp <= 0)
            hi = mid - 1;
        else if (cmp > 0)
            lo = mid + 1;
    }

    return linear_scan(table, prefix, suffix, word, lo);
}

/* this was really dumb... */
Index_t FindNextIndex(IndexTable table, char *prefix, char *suffix,
    char *word, char _prefix, int lo)
{
    (void)_prefix;

    lo++;
    return linear_scan(table, prefix, suffix, word, lo);
}

/* ----------------------------------------------------------------- *\
|  Index_t FindAbbrev(char *word)
|
|  Find the index of an abbrev in the abbrev table.  Return INDEX_NAN
|  if the abbrev isn't there.
\* ----------------------------------------------------------------- */
Index_t FindAbbrev(register char *word)
{
    register int hi, lo, mid;           /* must be signed */
    register int cmp;

    hi = numabbrevs - 1;
    lo = 0;

    while (hi >= lo) {
        mid = (hi + lo) / 2;
        cmp = strcmp(word, abbrevs[mid]);

        if (cmp == 0)
            return (Index_t)mid;
        else if (cmp < 0)
            hi = mid - 1;
        else if (cmp > 0)
            lo = mid + 1;
    }

    return (Index_t)INDEX_NAN;
}

/* =================== SET MANIPULATION ROUTINES =================== */

static Index_t setsize;
typedef unsigned long Set_t, *Set;
static Set_t setmask;                   /* used to erase extra bits */

#define SETSCALE (sizeof(Set_t) * 8)

/* ----------------------------------------------------------------- *\
|  Set NewSet(void)
|
|  Get a new variable to hold sets of integers in the range
|  [0, numoffsets].  Set setsize and setmask.
\* ----------------------------------------------------------------- */
Set NewSet(VOID)
{
    setsize = (numoffsets + SETSCALE - 1) / SETSCALE;       /* HACK */
    setmask = ((Set_t)1 << (numoffsets % SETSCALE)) - 1;    /* KLUDGE */

    return (Set)safemalloc(setsize * SETSCALE,
        "Can't create new result list", "");
}

/* ----------------------------------------------------------------- *\
|  void EmptySet(Set theset)
|
|  Empty the set.
\* ----------------------------------------------------------------- */
void EmptySet(Set theset)
{
    register Index_t i;
    for (i = 0; i < setsize; i++)
        theset[i] = 0;
}

/* ----------------------------------------------------------------- *\
|  void SetUnion(Set src1, Set src2, Set result)
|
|  Get the union of two sets
\* ----------------------------------------------------------------- */
void SetUnion(Set src1, Set src2, Set result)
{
    register Index_t i;
    for (i = 0; i < setsize; i++)
        result[i] = src1[i] | src2[i];
}

/* ----------------------------------------------------------------- *\
|  void SetIntersection(Set src1, Set src2, Set result)
|
|  Get the intersection of two sets
\* ----------------------------------------------------------------- */
void SetIntersection(Set src1, Set src2, Set result)
{
    register Index_t i;
    for (i = 0; i < setsize; i++)
        result[i] = src1[i] & src2[i];
}

/* ----------------------------------------------------------------- *\
|  void SetComplement(Set src, Set result)
|
|  Get the complement of a set
\* ----------------------------------------------------------------- */
void SetComplement(Set src, Set result)
{
    register Index_t i;
    for (i = 0; i < setsize; i++)
        result[i] = ~src[i];
    /* Bug fixed at version 2.5: setmask == 0 if numoffsets is  */
    /* a multiple of setsize, so the set contains no partial words, */
    /* The clearing of trailing bits must then be omitted. */
    if (setmask)
        result[setsize - 1] &= setmask;     /* clear those last few bits */
}

/* ----------------------------------------------------------------- *\
|  void CopySet(Set src, Set result)
|
|  Copy one set into another
\* ----------------------------------------------------------------- */
void CopySet(Set src, Set result)
{
    register Index_t i;
    for (i = 0; i < setsize; i++)
        result[i] = src[i];
}

/* ----------------------------------------------------------------- *\
|  int CountSet(Set theset)
|
|  Get the cardinality of the set
\* ----------------------------------------------------------------- */
int CountSet(Set theset)
{
    register Index_t i, count;
    register unsigned int j;

    count = 0;
    for (i = 0; i < setsize; i++)
        for (j = 0; j < SETSCALE; j++)
            if (theset[i] & ((Set_t)1 << j))
                count++;

    return count;
}

/* ----------------------------------------------------------------- *\
|  void BuildSet(Set theset, Index_t *thelist, Index_t length)
|
|  Build a set out of a list of integers
\* ----------------------------------------------------------------- */
void BuildSet(Set theset, Index_t *thelist, Index_t length)
{
    register Index_t i;

    EmptySet(theset);
    for (i = 0; i < length; i++)
        theset[thelist[i] / SETSCALE] |= (Set_t)1 << (thelist[i] % SETSCALE);
}

/* ----------------------------------------------------------------- *\
|  void DoForSet(Set theset, void (*action)(int, void *), void *arg)
|
|  Do something to every element in a set
\* ----------------------------------------------------------------- */
void DoForSet(Set theset, void (*action)(int, void *), void *arg)
{
    register Index_t i;
    register unsigned int j;

    for (i = 0; i < setsize; i++)
        for (j = 0; j < SETSCALE; j++)
            if (theset[i] & ((Set_t)1 << j))
                (*action)((int)(SETSCALE * i + j), arg);
}

/* ======================== SEARCH ROUTINES ======================== */

Set results, oldresults, oneword, onefield;
short firstfield, lastfield;            /* indices into fieldtable */

/* ----------------------------------------------------------------- *\
|  void InitSearch(void)
|
|  Initialize the search lists
\* ----------------------------------------------------------------- */
void InitSearch(VOID)
{
    results = NewSet();
    oldresults = NewSet();
    oneword = NewSet();
    onefield = NewSet();
    firstfield = lastfield = -1;
}

/* ----------------------------------------------------------------- *\
|  void FreeSearch(void)
|
|  Free the search list
\* ----------------------------------------------------------------- */
void FreeSearch(VOID)
{
    free(results);
    free(oldresults);
    free(oneword);
    free(onefield);
}

/* ----------------------------------------------------------------- *\
|  void ClearResults(void)
|
|  Clear the current and old results
\* ----------------------------------------------------------------- */
void ClearResults(VOID)
{
    EmptySet(results);
    SetComplement(results, results);
    CopySet(results, oldresults);
}

/* ----------------------------------------------------------------- *\
|  void SaveResults(void)
|
|  Save and clear the current results
\* ----------------------------------------------------------------- */
void SaveResults(VOID)
{
    CopySet(results, oldresults);
    EmptySet(results);
    SetComplement(results, results);
}

/* ----------------------------------------------------------------- *\
|  void CombineResults(char invert, char intersect)
|
|  Combine current results with old results
\* ----------------------------------------------------------------- */
void CombineResults(char invert, char intersect)
{
    if (invert)
        SetComplement(results, results);
    if (intersect)
        SetIntersection(results, oldresults, results);
    else
        SetUnion(results, oldresults, results);
}

/* ----------------------------------------------------------------- *\
|  char SetUpField(char *field)
|
|  Set up the search fields. Return the number of searchable fields.
\* ----------------------------------------------------------------- */
char SetUpField(char *field)
{
    int i, len;

    firstfield = -1;
    len = strlen(field);

    for (i = 0; i < (int)numfields; i++) {
        if (!strncmp(field, fieldtable[i].thefield, len)) {
            if (firstfield == -1)
                firstfield = i;
            lastfield = i;
        }
    }

    if (firstfield == -1) {
        (void)printf(COL_WARN "\tNo searchable fields matching \"%s\"."
            COL_RESET "\n", field);
        return 0;
    } else {
        return lastfield - firstfield + 1;
    }
}

static const char *const badwords[] = BADWORDS;
/* ----------------------------------------------------------------- *\
|  void FindWord(char *word, char prefix)
|
|  Find a word in the currently active field and update `results'.
|  If the prefix flag is set, find all words having the given prefix.
\* ----------------------------------------------------------------- */
void FindWord(register char *word, char prefix)
{
    register IndexPtr words;
    Index_t win;
    int i;
    char word_suffix[300], word_prefix[300];

    if (!prefix) {
        if (!word[0]) {
            (void)printf(COL_WARN "\t[ignoring empty string]" COL_RESET "\n");
            return;
        }
        if (!word[1]) {
            (void)printf(COL_WARN "\t[ignoring single letter \"%s\"]"
                COL_RESET "\n", word);
            return;
        }
#if !IGNORENONE
        for (i = 0; badwords[i]; i++) {
            if (!strcmp(badwords[i], word)) {
                (void)printf(COL_WARN "\t[ignoring common word \"%s\"]"
                    COL_RESET "\n", word);
                return;
            }
        }
#endif
    }

    EmptySet(oneword);

    for (i = firstfield; i <= lastfield; i++) {
        words = fieldtable[i].words;
        breakWord(word, word_prefix, word_suffix);

        win = FindIndex(fieldtable[i], word_prefix, word_suffix, word, prefix);
        while (win != INDEX_NAN) {
            do {
                CachedList *clist = &(words[win].refs);
                Index_t *p;

                Access(clist, bixfp);
                p = (Index_t *)safemalloc(clist->length * sizeof(Index_t),
                    "Can't allocate entry list.", "");
                UncompressRefs(p, clist->list, clist->length);
                BuildSet(onefield, p, clist->length);
                SetUnion(oneword, onefield, oneword);
                free(p);
            } while (prefix && ++win < fieldtable[i].numwords &&
                !strptrcmp(words[win].theword, word));

            win = FindNextIndex(fieldtable[i], word_prefix, word_suffix,
                word, prefix, win);
        }
    }

    SetIntersection(oneword, results, results);
}

/* ============================= OUTPUT ============================ */
FILE *bibfp;

/* ----------------------------------------------------------------- *\
|  void ReportResults(void)
|
|  Report the results of the previous search.
\* ----------------------------------------------------------------- */
void ReportResults(VOID)
{
    int numresults;

    numresults = CountSet(results);

    if (numresults == 0) {
        (void)printf(COL_WARN "\tNo matches found." COL_RESET "\n");
    } else if (numresults == 1) {
        (void)printf(COL_OUT "\t1 match found." COL_RESET "\n");
    } else {
        (void)printf(COL_OUT "\t%d matches found." COL_RESET "\n", numresults);
    }
}

/* ----------------------------------------------------------------- *\
|  void PrintEntry(int entry, FILE *ofp)
|
|  Print the entry.
\* ----------------------------------------------------------------- */
void PrintEntry(int entry, FILE *ofp)
{
    char ch;
    char braces;
    char quotes;

    if (entry >= (int)numoffsets)       /* extra bits might be set */
        return;

    putc('\n', ofp);
    if (fseek(bibfp, offsets[entry], 0))
        die("Index file is corrupt.", "");

    ch = safegetc(bibfp);

    while (ch != '@') {
        putc(ch, ofp);
        ch = safegetc(bibfp);
    }

    while ((ch != '{') && (ch != '(')) {
        putc(ch, ofp);
        ch = safegetc(bibfp);
    }

    braces = quotes = 0;

    putc(ch, ofp);
    ch = safegetc(bibfp);
    while (braces || quotes || ((ch != '}') && (ch != ')'))) {
        if (ch == '{')
            braces++;
        else if (ch == '}')
            braces--;
        else if ((ch == '"') && !braces)
            quotes = !quotes;
        putc(ch, ofp);
        ch = safegetc(bibfp);
    }

    putc(ch, ofp);
    putc('\n', ofp);
}

/* ----------------------------------------------------------------- *\
|  void PrintResults(char *filename)
|
|  Print the current search results into the given file.  If the
|  filename is NULL, pipe the output through $PAGER.
\* ----------------------------------------------------------------- */
void PrintResults(char *filename)
{
    int numresults;
    FILE *ofp;
#ifndef __SYMBIAN32__
    char *pager;
    char *the_tmpfile = (char *)NULL;
#if unix
    int childpid;
#elif MSDOS
    char *command, *command_fname;
#else /* ? other systems ? */
#endif
#endif

    numresults = CountSet(results);
    if (numresults == 0) {
        (void)printf(COL_WARN "\tNothing to display!" COL_RESET "\n");
#if MAXRESULTS
    } else if (numresults > MAXRESULTS) {
        (void)printf(COL_WARN "\tI can't display that many results!" COL_RESET
            "\n");
#endif
    } else {
        if (filename) {
            ofp = fopen(filename, "a");
            if (!ofp) {
                (void)printf(COL_ERR "\tCan't open %s: " COL_RESET, filename);
                perror(NULL);
                return;
            }
        } else {
#ifdef __SYMBIAN32__
            ofp = stdout;
#else
            the_tmpfile = (char *)tempnam(NULL, "bibl.");
            ofp = fopen(the_tmpfile, "w");
            if (!ofp) {
                perror("\tCan't open temp file");
                return;
            }
#endif
        }

        if (filename) {
            time_t now = time(0);
            (void)fprintf(ofp, "%% Retrieved by biblook %d.%d at %s",
                MAJOR_VERSION, MINOR_VERSION, ctime(&now));
        }

        DoForSet(results, (void (*)(int, void *))PrintEntry, (void *)ofp);

#ifdef __SYMBIAN32__
        if (filename)
#endif
            fclose(ofp);
        if (filename)
            (void)printf(COL_OUT "\tResults saved in \"%s\"" COL_RESET "\n",
                filename);
#ifndef __SYMBIAN32__
        else {
            pager = (char *)getenv("PAGER");

#if unix
            if ((childpid = (int)fork()) != 0) {
                waitpid(childpid, (int *)0, 0);
            } else if (pager) {
                execlp(pager, pager, the_tmpfile, (char *)0);
                perror(pager);          /* should never get here! */
                exit(EXIT_SUCCESS);
            } else {
                /* try absolute path first */
                execl(MOREPATH, MORE, the_tmpfile, (char *)0);
                /* next try to find it in PATH list */
                execlp(MORE, MORE, the_tmpfile, (char *)0);
                /* no pager available, so give up */
                perror(MOREPATH);
                exit(EXIT_SUCCESS);
            }
#elif MSDOS
            if (pager) {
                command = (char *)malloc(strlen(pager) + 2 +
                    strlen(the_tmpfile));
                strcpy(command, pager);
                strcat(command, " ");
            } else {
                command = (char *)malloc(7 + strlen(the_tmpfile));
                strcpy(command, "MORE <");
            }

            command_fname = command + strlen(command);
            strcat(command, the_tmpfile);
            for (; *command_fname != '\0'; command_fname++)
                if (*command_fname == '/')
                    *command_fname = '\\';

            if (system(command))
                perror((pager ? pager : "MORE"));
            free(command);
#else /* ? other systems ? */
#endif

            unlink(the_tmpfile);
            free(the_tmpfile);          /* malloc'ed by tempnam() */
            putchar('\n');
        }
#endif
    }
}

/* ----------------------------------------------------------------- *\
|  void DisplayAbbrev(char *theabbrev)
|
|  Display the definition of the given abbreviation.
\* ----------------------------------------------------------------- */
void DisplayAbbrev(char *theabbrev)
{
    Index_t the_index = FindAbbrev(theabbrev);

    if (the_index == INDEX_NAN) {
        (void)printf(COL_WARN "\tThe abbreviation \"%s\" is not defined."
            COL_RESET "\n", theabbrev);
    } else if (abbrevlocs[the_index] == INDEX_BUILTIN) {
        (void)printf(COL_OUT "\tThe abbreviation \"%s\" is builtin."
            COL_RESET "\n", theabbrev);
    } else {
        PrintEntry((int)abbrevlocs[the_index], stdout);
        putchar('\n');
    }
}

/* ======================== USER INTERFACE ========================= */

typedef enum {
    T_Find,
    T_Whatis,
    T_Display,
    T_Save,
    T_Quit,
    T_Word,
    T_And,
    T_Or,
    T_Not,
    T_Semi,
    T_Return,
    T_Help,
    T_Copyright
#ifndef USE_READLINE
    ,
    T_History,
    T_WriteHistory,
    T_ReadHistory,
    T_CompressHistory
#endif
} Token;

#ifdef USE_READLINE
/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char * stripwhite(string) char *string;
{
    register char *s, *t;

    for (s = string; whitespace(*s); s++)
        ;

    if (*s == 0)
        return (s);

    t = s + strlen(s) - 1;
    while (t > s && whitespace(*t))
        t--;
    *++t = '\0';

    return s;
}

#else /* USE_READLINE */

typedef struct {
    const char *str;
    Token token_id;
    BOOL fStrictMatch;
} TableEntryToken;

static const TableEntryToken tokens_array[] =
    {{"find", T_Find, FALSE},
     {"display", T_Display, FALSE},
     {"help", T_Help, FALSE},
     {"save", T_Save, FALSE},
     {"whatis", T_Whatis, FALSE},
     {"quit", T_Quit, FALSE},
     {"and", T_And, TRUE},
     {"or", T_Or, TRUE},
     {"not", T_Not, TRUE},
     {"history", T_History, FALSE},
     {"writehistory", T_WriteHistory, FALSE},
     {"readhistory", T_ReadHistory, FALSE},
     {"compress", T_CompressHistory, FALSE},
     {NULL, ((Token)0), FALSE}};

static BOOL is_empty_line(char *line)
{
    while (*line) {
        if ((*line != ' ') && (*line != '\t') && (*line != '\n'))
            return FALSE;
        line++;
    }

    return TRUE;
}

#endif /* USE_READLINE */

/* ----------------------------------------------------------------- *\
|  Token GetToken(char *tokenstr)
|
|  Get the next input token.
\* ----------------------------------------------------------------- */
Token GetToken(char *tokenstr)
{
    static char line[256];
#ifdef USE_READLINE
    char *r, *s;
    const char *prompt = "biblook: ";
#endif
    static short pos;
    static char neednew = 1;
    short tlen = 0;
#ifndef USE_READLINE
    const TableEntryToken *p_tokens;
    int token_id;
    BOOL fReadLine;

    fReadLine = FALSE;
#endif
    *tokenstr = 0;

    if (neednew) {
#ifdef USE_READLINE
        r = readline(prompt);
        if (!r)
            return T_Quit;
        s = stripwhite(r);
        if (*s) {
            HIST_ENTRY *curr;
            curr = history_get(history_length);
            /* Only add to history list non-duplicated entries. */
            if (!curr || (curr->line && strcmp(s, curr->line)))
                add_history(s);
        }
        strncpy(line, r, 254);
        strcat(line, "\n");             /* this trick seems to be necessary */
        free(r);
#else
        do {
            (void)printf(COL_IN "biblook: " COL_RESET);
            if (!fgets(line, 254, stdin))
                return T_Quit;
            fReadLine = TRUE;

            if (History_expand(line))
                printf(COL_IN "biblook: %s" COL_RESET "\n", line);

            History_add(line);
            History_clean_remark(line, NULL);
        } while (is_empty_line(line));
        strcat(line, "\n");

#endif /* USE_READLINE */

        pos = 0;
        neednew = 0;
    }

    while ((line[pos] == ' ') || (line[pos] == '\t'))
        pos++;

    switch (line[pos]) {
#ifndef USE_READLINE
    case 0:
#endif
    case '\n':
        pos++;
        neednew = 1;
        return T_Return;

    case '&':
        pos++;
        return T_And;

    case '|':
        pos++;
        return T_Or;

    case '~':
    case '!':
        pos++;
        return T_Not;

    case ';':
        pos++;
        return T_Semi;

    case '?':
        pos++;
        return T_Help;

    case '@':
        pos++;
        return T_Copyright;

    default:
        tokenstr[tlen++] = tolower(line[pos++]);
        while (!isspace(line[pos]) && (line[pos] != ';') &&
                (line[pos] != '&') && (line[pos] != '|')) {
            tokenstr[tlen++] = tolower(line[pos++]);
        }
        tokenstr[tlen] = 0;

#ifdef USE_READLINE
        /* I really ought to use a hash table here. */

        if (!strncmp(tokenstr, "find", tlen))
            return T_Find;
        else if (!strncmp(tokenstr, "display", tlen))
            return T_Display;
        else if (!strncmp(tokenstr, "help", tlen))
            return T_Help;
        else if (!strncmp(tokenstr, "save", tlen))
            return T_Save;
        else if (!strncmp(tokenstr, "whatis", tlen))
            return T_Whatis;
        else if (!strncmp(tokenstr, "quit", tlen))
            return T_Quit;
        else if (!strcmp(tokenstr, "and"))
            return T_And;
        else if (!strcmp(tokenstr, "or"))
            return T_Or;
        else if (!strcmp(tokenstr, "not"))
            return T_Not;
        else
            return T_Word;
#else  /* USE_READLINE */
        token_id = -1;
        p_tokens = tokens_array;
        while (p_tokens->str != NULL) {
            if (p_tokens->fStrictMatch) {
                if (!strcmp(tokenstr, p_tokens->str)) {
                    token_id = p_tokens->token_id;
                    break;
                }
            } else {
                if (!strncmp(tokenstr, p_tokens->str, tlen)) {
                    token_id = p_tokens->token_id;
                    break;
                }
            }

            p_tokens++;
        }
        if (token_id != -1) {
            if (fReadLine && (line[pos] == '\n' || line[pos] == '\r') &&
                    ((token_id == T_Display) || (token_id == T_History) ||
                    (token_id == T_Quit) || (token_id == T_CompressHistory)))
                History_swallow_last();
            return ((Token)token_id);
        }

        return T_Word;
#endif /* USE_READLINE */
    }
}

/* ----------------------------------------------------------------- *\
|  char Strip(char *string)
|
|  Strip all but alphanumeric characters out of the string.  Return
|  true if the original string ended with the prefix character '*'.
\* ----------------------------------------------------------------- */
char Strip(char *string)
{
    char prefix = 0;
    char *src = string;

    while (*src) {
        prefix = (*src == '*');
        if (isalnum(*src))
            *string++ = *src;
        src++;
    }
    *string = 0;
    return prefix;
}

char StripExt(char *string)
{
    char prefix = 0;
    char *src = string;

    while (*src) {
        prefix = (*src == '*');
        if (isalnum(*src) || *src == '*' || *src == '?')
            *string++ = *src;
        src++;
    }
    *string = 0;
    return prefix;
}

/* ----------------------------------------------------------------- *\
|  void CmdError(void)
|
|  Print syntax error message
\* ----------------------------------------------------------------- */
void CmdError(VOID)
{
    (void)printf(COL_WARN "\t?? Syntax error ??" COL_RESET "\n");
}

static const char *const shorthelplines[] = {
        "------------------------------------------------------------",
        "help			Print this message",
        "find <field> <words>	Find entries with <words> in <field>",
        "and  <field> <words>	Narrow search",
        "or   <field> <words>	Widen search",
        "display			Display search results",
        "save <file>		Save search results to <file>",
        "whatis <abbrev>		Find and display an abbreviation",
#ifndef USE_READLINE
        "history                 Display history",
#endif
        "quit			Quit biblook",
        "------------------------------------------------------------",
        "Type `help' or `?' again for more details.",
        (const char *)NULL,
};

static const char *const longhelplines[] = {
        "biblook permits rapid lookup in a BibTeX bibliography data",
        "base, using a compact binary index file prepared by bibindex(1).",
        "",
        "Available commands:",
        "? or h[elp]",
        "     Display this help message.",
        "",
        "f[ind] [not] <field> <words>",
        "     Find the entries containing the given words in any",
        "     field with a prefix matching the <field> argument.  For",
        "     example, `a' matches both `author' and `address', and",
        "     `au' matches `author' only.  If the <field> argument is",
        "     `-' (or any string with no letters or numbers), match",
        "     any field.",
        "",
        "     If `not' appears before the <field>, the sense of the",
        "     search is reversed.  The symbols `~' and `!' can be",
        "     used in place of `not'.",
        "",
        "     Each word is a contiguous sequence of letters and",
        "     digits.  Case is ignored; accents should be omitted;",
        "     apostrophes are not required.  Single characters and a",
        "     few common words are also ignored.  ? matches any single",
        "     character and * matches any string of characters.  Thus,",
        "     `*oint*' matches `point', `points', `pointer', `endpoint',",
        "     `disjoint', etc.  However at present patterns beginning with ?",
        "     cannot be used, as the parser mistakes them for a help request.",
        "",
        "and [not] <field> <words>",
        "or [not] <field> <words>",
        "     Intersect (resp. union) the results of the given search",
        "     with the previous search.  Several of these commands",
        "     may be combined on a single line.  Commands are handled",
        "     in the order in which they appear; there is no pre-",
        "     cedence.  Unlike other commands, and like `not', these",
        "     must be spelled out completely.  `&' can be used in",
        "     place of `and', and `|' can be used in place of `or'.",
        "",
        "d[isplay]",
        "     Display the results of the previous search.",
        "",
        "s[ave] [<filename>]",
        "     Save the results of the previous results into the",
        "     specified file.  If <filename> is omitted, the previous",
        "     save file is used.  If no save file has ever been",
        "     specified, results are saved in the file specified on",
        "     the command line.  If no such file is specified,",
        "     `save.bib' is used.  If the save file exists, results",
        "     are appended to it.",
        "",
#ifndef USE_READLINE
        "history",
        "     Display history.",
        "!<number>",
        "     Insert line <number> from history into current line",
        "writehistory [<filename>]",
        "     Save history file.",
        "readhistory [<filename>]",
        "     Read history file.",
        "# bla bla bla",
        "     Insert comment into history.",
        "compress",
        "     Compress history",
        "",
#endif
        "w[hatis] <abbrev>",
        "     Display the definition of the abbreviation <abbrev>.",
        "",
        "q[uit]/EOF",
        "     Quit.",
        "",
        "Several commands can be combined on a single line by",
        "separating them with semicolons.  For example, the following",
        "command displays all STOC papers cowritten by Erdo\"s",
        "without `Voronoi diagrams' in the title:",
        "",
        "f b stoc* | b symp* theory comp* & au erdos & ~t voronoi diagrams ; d",
        "",
        (const char *)NULL,
};

/* ----------------------------------------------------------------- *\
|  void GiveCopyright ()
|
|  Print the GPL short statement.
\* ----------------------------------------------------------------- */
void GiveCopyright(void)
{
    CopyrightBanner();
    (void)printf("\n\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
You should have received a copy of the GNU General Public License\n\
along with this program. If not, write to the Free Software\n\
Foundation, 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n\n");
}

/* ----------------------------------------------------------------- *\
|  void GiveHelp(int verbose)
|
|  Print a help message.  Lines are stored as separate strings to
|  avoid hitting compiler limits.
\* ----------------------------------------------------------------- */
void GiveHelp(int verbose)
{
    int k;
#if 0
    /* This fails on IBM RS/6000 AIX 3.2 cc with bogus error:
       "Operands must be pointers to compatible types." */
    const char **help = verbose ? &longhelplines[0] : &shorthelplines[0];
#else
    const char *const *help;

    if (verbose)
        help = &longhelplines[0];
    else
        help = &shorthelplines[0];
#endif

    for (k = 0; help[k]; ++k)
        (void)printf("\t%s\n", help[k]);
}

/* ----------------------------------------------------------------- *\
|  States for Lookup()
\* ----------------------------------------------------------------- */
typedef enum {
    Wait,                               /* nothing yet */
    Find,                               /* "find" */
    FindN,                              /* "find not" */
    FindF,                              /* "find [not] <field>" */
    FindW,                              /* "find [not] <field> <words>" */
    Display,                            /* "display" */
    Save,                               /* "save" */
    SaveF,                              /* "save <file>" */
    Whatis,                             /* "whatis" */
    WhatisA,                            /* "whatis <abbrev>" */
    Help,                               /* "help" */
    Copyright,                          /* "Copyright" */
#ifndef USE_READLINE
    History,                            /* "history" */
    WriteHistory,                       /* "writehistory" */
    WriteHistoryF,                      /* "writehistory <file> */
    ReadHistory,                        /* "readhistory" */
    ReadHistoryF,                       /* "readhistory <file> */
    CompressHistory,
#endif
    Error                               /* anything else */
} CmdState;

/* ----------------------------------------------------------------- *\
|  void Lookup(const char *defsave)
|
|  Execute commands until the user quits.  Defsave is the default
|  save file name.  This is one big finite state machine.  It's long
|  and boring, but that's interface code for ya!
|
\* ----------------------------------------------------------------- */
#ifdef USE_READLINE
void Lookup(const char *defsave)
#else
void Lookup(const char *defsave, const char *def_history_save)
#endif
{
    char tokenstr[256];
    char savestr[256];
#ifndef USE_READLINE
    char write_history_str[256];
#endif
    CmdState state = Wait;
    static CmdState last_state = Wait;

    Token thetoken;
    char intersect = 1;                 /* 1 = intersect, 0 = union */
    char invert = 0;                    /* 1 = invert */
    char prefix;                        /* 1 = word is really a prefix */

    ClearResults();
    strcpy(savestr, defsave);
#ifndef USE_READLINE
    strcpy(write_history_str, def_history_save);
#endif

    for (;;) {
        thetoken = GetToken(tokenstr);

        if ((thetoken == T_Quit) && !tokenstr[0])
            return;

        switch (state) {
        case Wait:
            switch (thetoken)
            {
            case T_Quit:
                return;
            case T_Find:
                state = Find;
                invert = 0;
                ClearResults();
                break;
            case T_And:
                state = Find;
                invert = 0;
                SaveResults();
                break;
            case T_Or:
                state = Find;
                invert = 0;
                intersect = 0;
                SaveResults();
                break;
            case T_Display:
                state = Display;
                break;
            case T_Save:
                state = Save;
                break;
#ifndef USE_READLINE
            case T_CompressHistory:
                state = CompressHistory;
                break;
            case T_WriteHistory:
                state = WriteHistory;
                break;
            case T_ReadHistory:
                state = ReadHistory;
                break;
#endif
            case T_Help:
                state = Help;
                break;

            case T_Copyright:
                state = Copyright;
                break;

#ifndef USE_READLINE
            case T_History:
                state = History;
                break;
#endif
            case T_Whatis:
                state = Whatis;
                break;
            case T_Return:
            case T_Semi:
                break;
            default:
                state = Error;
                CmdError();
                break;
            }
            break;

        case Find:
            if (thetoken == T_Not) {
                last_state = state;
                state = FindN;
                invert = 1;
            } else {
                if (tokenstr[0]) {
                    state = FindF;
                    Strip(tokenstr);
                    if (!SetUpField(tokenstr))
                        state = Error;
                    else
                        last_state = Find;
                } else {
                    state = (thetoken == T_Return) ? Wait : Error;
                    CmdError();
                }
            }
            break;

        case FindN:
            if (tokenstr[0]) {
                state = FindF;
                Strip(tokenstr);
                if (!SetUpField(tokenstr))
                    state = Error;
                else
                    last_state = FindN;
            } else {
                state = (thetoken == T_Return) ? Wait : Error;
                CmdError();
            }
            break;

        case FindF:
            if (tokenstr[0]) {
                last_state = state;
                state = FindW;
                prefix = StripExt(tokenstr);
                FindWord(tokenstr, prefix);
            } else {
                state = (thetoken == T_Return) ? Wait : Error;
                CmdError();
            }
            break;

        case FindW:
            switch (thetoken) {
            case T_And:
                last_state = state;
                state = Find;
                CombineResults(invert, intersect);
                SaveResults();
                invert = 0;
                intersect = 1;
                break;
            case T_Or:
                last_state = state;
                state = Find;
                CombineResults(invert, intersect);
                SaveResults();
                invert = 0;
                intersect = 0;
                break;
            case T_Semi:
                last_state = state;
                state = Wait;
                CombineResults(invert, intersect);
                invert = 0;
                intersect = 1;
                break;
            case T_Return:
                last_state = state;
                state = Wait;
                CombineResults(invert, intersect);
                ReportResults();
                invert = 0;
                intersect = 1;
                break;
            default:
                if (tokenstr[0]) {
                    last_state = state;
                    state = FindW;
                    prefix = StripExt(tokenstr);
                    FindWord(tokenstr, prefix);
                } else {
                    state = Error;
                    CmdError();
                }
                break;
            }
            break;

        case Display:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                last_state = state;
                state = Wait;
                PrintResults(NULL);
            } else {
                state = Error;
                CmdError();
            }
            break;

#ifndef USE_READLINE
        case ReadHistory:
            if (tokenstr[0]) {
                last_state = state;
                state = ReadHistoryF;
                strcpy(write_history_str, tokenstr);
            } else if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                state = Wait;
                History_read(write_history_str, FALSE);
            } else {
                state = Error;
                CmdError();
            }
            break;

        case ReadHistoryF:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                last_state = state;
                state = Wait;
                History_read(write_history_str, FALSE);
            } else {
                state = Error;
                CmdError();
            }
            break;

        case CompressHistory:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                state = Wait;
                History_compress();
                History_dump();
                last_state = CompressHistory;
            }
            break;

        case WriteHistory:
            if (tokenstr[0]) {
                last_state = state;
                state = WriteHistoryF;
                strcpy(write_history_str, tokenstr);
            } else if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                state = Wait;
                History_write(write_history_str);
            } else {
                state = Error;
                CmdError();
            }
            break;

        case WriteHistoryF:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                last_state = state;
                state = Wait;
                History_write(write_history_str);
            } else {
                state = Error;
                CmdError();
            }
            break;
#endif
        case Save:
            if (tokenstr[0]) {
                last_state = state;
                state = SaveF;
                strcpy(savestr, tokenstr);
            } else if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                state = Wait;
                PrintResults(savestr);
            } else {
                state = Error;
                CmdError();
            }
            break;

        case SaveF:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                last_state = state;
                state = Wait;
                PrintResults(savestr);
            } else {
                state = Error;
                CmdError();
            }
            break;

        case Whatis:
            if (tokenstr[0]) {
                last_state = state;
                state = WhatisA;
                strcpy(savestr, tokenstr);
            } else {
                state = (thetoken == T_Return) ? Wait : Error;
                CmdError();
            }
            break;

        case WhatisA:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                last_state = state;
                state = Wait;
                DisplayAbbrev(savestr);
            } else {
                state = Error;
                CmdError();
            }
            break;

        case Help:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                state = Wait;
                GiveHelp(last_state == Help);
                last_state = Help;
            }
            break;

        case Copyright:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                state = Wait;
                GiveCopyright();
                last_state = Copyright;
            }
            break;

#ifndef USE_READLINE
        case History:
            if ((thetoken == T_Semi) || (thetoken == T_Return)) {
                state = Wait;
                History_dump();
                last_state = Help;
            }
            break;
#endif
        case Error:
            switch (thetoken) {
            case T_Quit:
                return;
            case T_Return:
                state = Wait;
                break;
            default:
                break;
            }
            break;
        } /* end switch(state) */
    }     /* end for(;;) */
}

/* ================================================================= *\
|  The main program
\* ================================================================= */
int main(int argc, char **argv)
{
    register char *path, *tmp;
    char gotit;
    struct stat bibstat, bixstat;
    char *p;

    CopyrightBanner();
    (void)printf("For details, type @.\n");
    (void)printf("Type ? or h for help.\n\n");

    if ((argc != 2) && (argc != 3)) {
        (void)fprintf(stderr, COL_OUT "Usage: biblook bib [savefile]"
            COL_RESET "\n");
        exit(EXIT_FAILURE);
    }

    if (((p = strrchr(argv[1], '.')) != (char *)NULL) &&
            (strcmp(p, ".bib") == 0)) {
        *p = '\0'; /* remove any .bib extension */
    }

    /* ---- Search BIBLOOKPATH or BIBINPUTS for the files ---- */

    path = (char *)getenv("BIBLOOKPATH");
    if (path == NULL)
        path = (char *)getenv("BIBINPUTS");
    if ((path == NULL) || (argv[1][0] == '/')) {
        (void)sprintf(bibfile, "%s.bib", argv[1]);
        (void)sprintf(bixfile, "%s.bix", argv[1]);
    } else {
        tmp = path;
        gotit = 0;

        while ((tmp != NULL) && !gotit) {
            tmp = strchr(path, ':');
            if (tmp != NULL)
                *tmp = 0;

            if (strcmp(path, ".")) {
                (void)sprintf(bibfile, "%s/%s.bib", path, argv[1]);
                (void)sprintf(bixfile, "%s/%s.bix", path, argv[1]);
            } else {
                (void)sprintf(bibfile, "%s.bib", argv[1]);
                (void)sprintf(bixfile, "%s.bix", argv[1]);
            }

            if (stat(bibfile, &bibstat) != 0) {
                if (errno != ENOENT)
                    pdie("Can't open", bibfile);
            } else {
                gotit = 1;
            }

            if (tmp != NULL)
                path = tmp + 1;
        }

        if (!gotit) {
            (void)sprintf(bibfile, "%s.bib", argv[1]);
            pdie("Can't find", bibfile);
        }
    }

    /* ---- Now that we've found the files, open them and do the job ---- */

    if (stat(bibfile, &bibstat) != 0)
        pdie("Can't open", bibfile);

    if (stat(bixfile, &bixstat) != 0)
        pdie("Can't open", bixfile);

    if (bibstat.st_mtime > bixstat.st_mtime)
        die(bixfile, "is out of date.\n\tPlease rerun bibindex.");

    bibfp = fopen(bibfile, "r");
    if (!bibfp)
        pdie("Can't read", bibfile);
#if MSDOS
    bixfp = fopen(bixfile, "rb");
#else
    bixfp = fopen(bixfile, "r");
#endif
    if (!bixfp)
        pdie("Can't read", bixfile);

    GetTables();
    InitSearch();

    History_init();

    if (argc == 3)

#ifdef USE_READLINE
        Lookup(argv[2]);
#else
        Lookup(argv[2], "biblook.history");
#endif

    else

#ifdef USE_READLINE
        Lookup("save.bib");
#else
        Lookup("save.bib", "biblook.history");
#endif

    History_term();

    FreeSearch();
    FreeTables();

    fclose(bibfp);
    fclose(bixfp);
    return (0);
}

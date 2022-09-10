/*

Copyright (C) 1992, 1993 Jeff Erickson
Copyright (C) 1993, 1994, 1995, 1998, 2000 Bill Jones
Copyright (C) 2000 Rafael Laboissiere

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

   bibindex -- a program to index bibtex files, used in conjunction
           with biblook

   This program was specifically developed for use with the
   computational geometry bibliographic database, and assumes the
   rules thereof.  The database may be obtained by anonymous ftp
   from cs.usask.ca in the file "pub/geometry/geombib.tar.Z".

   Version 1.0 written by Jeff Erickson <jeff@ics.uci.edu>, 27 Mar 92
   Version 2.0 written by Jeff Erickson <jeff@ics.uci.edu>, 17 Jun 92

   This program is in the public domain.  You may use it or modify
   it to your heart's content, at your own risk.  Bouquets, brickbats,
   and bug fixes may be sent to Jeff Erickson, jeffe@cs.berkeley.edu.

   %Make% gcc -O -o bibindex bibindex.c

   Usage: bibindex bibfile [-i field ...]

   -----------------------------------------------------------------
   HOW IT WORKS:

   The bibtex file is read field by field.  The file offset beginning
   each record and each record's citation key are recorded.  A list of
   words is extracted from each field.  These words are placed into
   tables, which remember which records contain them in their
   respective fields.  Once the file has been completely read, the
   hash tables are compacted and sorted.

   The hash tables are extensible, since we have to maintain one for
   each possible field type, and static tables would be way too big.
   Initially, each table holds 1K entries, and the tables are doubled
   whenever they near full capacity.  Each table entry is at least 24
   bytes.  If the resulting hash tables use too much memory, the
   entries should be changed to pointers, allocated on the fly.

   The entry lists associated with each word are implemented as
   extensible arrays.  Initially, each list holds eight entries.  If a
   new entry is inserted into a full list, the list is doubled first.

   The index file has the following format (loosely):

    version info
    # entries
    array of offsets into bib file	-- one per entry
    # field types (incl. "@string")
    array of field names		-- one per field type
    array of			-- one per field type
        # words
        array of			-- one per word
        word			-- in alphabetical order
        # locations
        array of entry #s	-- one per location [compressed]
    # abbreviations
    array of abbreviations		-- in alphabetical order
    array of offsets into bib file	-- one per abbreviation

   There are advantages and disadvantages of having multiple hash
   tables instead of a single table.  I am starting with the premise
   that the lookup program should be very fast.  Consequently, I can't
   make it determine which fields contain a given word.  Doing so
   would require putting ALL of the entry-parsing code into the lookup
   program.  It would also mean potentially parsing a lot of
   extraneous entries to find relatively common words in relatively
   uncommon places (eg, "title edelsbrunner").

   If there were a single word table, each word list would have to
   include bitmasks to indicate the appropriate fields along with the
   entry numbers.  Assuming between 16 and 32 field types (the CG bib
   uses about 24), this would triple the size of each entry.  On the
   average, each word occurs in less than two field types.  The
   bitmask approach would also require knowledge of the field names in
   advance; the multiple table approach does not.

   -----------------------------------------------------------------
   VERSION HISTORY:

   1.0 <jge> 3/26/92	Initial version completed
   1.1 <jge> 3/27/92	Words restricted to letters only; special
            rules added for apostrophes, so that words
            like "didn't" and "O'Rourke" can be handled
            correctly.
   1.2 <jge> 3/30/92	Faster hash function; now compressing hash
            tables before sorting them.  Execution time on
            the CG bib reduced by a factor of thirty-five.
   1.3 <jge> 4/2/92	Toyed with the hash function some more, trying
            to reduce collisions, with limited success.
   1.4 <jge> 4/17/92	Added exit(0) at the end of main()  [I thought
            that was supposed to be automatic!]

   2.0 <jge> 6/12/92	First major revision completed.
    1. Major change in file format -- word tables for every
       field instead of just author, title, and keywords.
    2. Word hash tables made extensible.
    3. Fixed bug in GetNextWord that would cause the function
       to return inside nested braces.
    4. Fixed bug in MungeField that would kill everything in an
       entry after an abbreviation.  Abbreviations now go into
       the appropriate hash table with the other words.
    5. Made GetNextWord consider numbers and math expressions
       to be words.
    6. Added WriteWord, resulting in 40% savings in disk space.
    7. Added -i flag and black holes.  Ignoring "oldlabel"
       gives another 15% savings (6/92 version of CGbib).
   2.1 <jge> 7/9/92	Minor bug fixes.
   2.2 Nelson H. F. Beebe <beebe@math.utah.edu> 03-Oct-1992
    Testing with >100K lines of .bib files led to these changes:
    1. Add support for complete BibTeX keyword name syntax with
       iskeychar() function.
    2. Add support for trailing comma after last key = "value" entry.
    3. Record input line number for display in error messages.
    4. Record initial line number of each BibTeX entry for
       display in error messages to better localize error.
    5. Add test for word buffer overflow in MungeField() to prevent
       run-time core dumps, and increase supported word size from
       15 to 31 (a word-length histogram of a 116921-word dictionary
       found words up to 28 characters long, with 1% longer than 15).
       File version increased to 2 because of word size change.
    6. Add typecasts in calls to qsort() and comparisons of
       unsigned short with short, change main() from void to int,
       remove variable from initializer of msg[2], and add void to
       IndexBibFile() definition to allow compilation with C++ as
       well as C for better compile-time checking.
    7. In MungeEntry(), do an ungetc() after key name collection.
       Otherwise, a key="value" pair without a blank before the =
       will not be recognized because the = read on lookahead has
       not been put back.
    8. Revise table compression code in OutputTables() and
       code in FreeTables() to avoid duplicate frees, which is
       a fatal error on many systems, and specified to result
       in undefined behavior by the 1989 ANSI/ISO C Standard.
    9. Define bcopy() as a macro to invoke standard memcpy()
       instead.
       10. Include time.h, unistd.h, and malloc.h to get proper
       function declarations for library routines needed here.
       11. Add DEBUG_MALLOC support.
       12. Change several char* types to const char*.
       13. Correct some typographical errors in comment.
       14. Write UNIX manual pages for bibindex and biblook.
       15. Allow command-line to specify a filename with .bib extension.
       16. Add help support to biblook.
       17. Correct error in FreeTables() in biblook.c; inner loop
       incremented i instead of j.
   2.3 Bill Jones <jones@cs.usask.ca> 93/01/29
    1. Declarations common to bibindex.c and biblook.c factored out
       to new file biblook.h.
    2. Index type of (signed) short overflows early; created typedef
       Index_t, defined as unsigned short.
    3. Changed hash tables to extend at 75% capacity rather than 50%.
   2.4 Nelson H. F. Beebe <beebe@math.utah.edu> [01-Jun-1993]
    1. Remove some mixed-mode arithmetic.
    2. Increase MAXFIELDS from 64 to 127 to deal with TeX User Group
       bibliography collection
    3. Correct error in GetHashTable(); older versions got into an
       infinite loop if MAXFIELDS field names were already stored, and
       a new one was encountered.
   2.5 Erik Schoenfelder <schoenfr@ibr.cs.tu-bs.de> [14-Aug-1993]
    1. Add support for network byte order I/O, so that index files
       can be shared between big-endian and little-endian systems.
       This option is selected when HAVE_NETINET_IN_H is defined
       at compile time (default on UNIX).
       Nelson H. F. Beebe <beebe@math.utah.edu> [14-Aug-1993]
    2. Add typecast (int) in OutputTables() array reference to eliminate
       compiler warnings.
    3. Correct code in biblook:SetComplement() to check for zero
       setmask; otherwise, a .bib file with a number of entries
       which is an exact multiple of setsize (commonly 32) will
       result in failing searches for the last setsize entries!
   2.6 Jeff Erickson <jeffe@cs.berkeley.edu> 8/19/93
    1. Made MungeWord() use iskeychar() instead of referring to
       NONABBREV directly, and made iskeychar() use NONABBREV (now
       called NONKEYCHARS).  No reason to spell it out twice!
    2. Correctly distinguish between abbreviations and numbers
       not in quotes.
    3. 75% calculation for hash table capacity uses unsigned longs
       instead of Index_t's, which might overflow if Index_t is
       unsigned short.
    4. Added support for abbreviations (@strings).  Abbrevs are
       stored in special "field" table.  HashCell data structure
       modified to keep words in the expansion of any abbrev, along
       with the offset at which the abbrev is defined.  Whenever an
       abbrev is encountered, both the abbrev and its expansion are
       indexed.  Warnings for multiple definitions of the same
       abbrev or use of an undefined abbrev.  Automatic inclusion
       of standard abbrevs.
    5. Abbrevs stored in index file to support biblook "whatis"
       command.  File version changed to 3.
    6. Support for # concatenation added to MungeField.
    7. Brackets ignored in GetNextWord, so optional abbreviations
       like "J[ohn]" are handled correctly.
    8. Elements of compound words, like "lower-bound" and "Fejes
       Toth", are indexed both together and separately.
    9. Added support for BIBINDEXFLAGS environment variable, which
       stores default set of command-line arguments
       10. No longer barfs on @comment and @preamble entries.
       11. Changed format of screen output.
       Nelson H. F. Beebe <beebe@math.utah.edu> [12-Sep-1993]
       12. Add some code fixes so compilation under C++ works again.
       13. Restore extended list of BADWORDS (defined in biblook.h).
       Profiling showed that the sequential search through the
       extended badwords[] table in IsRealWord() is a major
       bottleneck with large input files.  Therefore, replace slow
       sequential search by fast hash lookup, with new functions
       InHashCell() and StandardBadWords(), and new initialization
       code in InitTables(), FreeTables(), and GetHashCell(),
       14. Fix bug in GetNextWord() that produced string overflow in
       word, and corrupt .bix file.  Add corresponding sanity
       check in WriteWord().
       15. [Withdrawn]
       16. [Withdrawn]
       17. Apply some spelling corrections to strings and comments.
       18. Prevent zero divide in statistics output in OutputTables().
       19. Use FILENAME_MAX+1 (defined in biblook.h) for size of filename
       strings.
       20. [Withdrawn]
       21. Change type of numfields from char to unsigned char, and
           use sizeof numfields instead of sizeof(type of numfields).
       22. Change all exit() calls to use Standard C EXIT_xxx symbols.
       23. Put (void) casts on printf() family calls to avoid compiler
           warnings about discarded returned values.
       24. Change type of loop index variables from "short" to "int",
       since shorts carry a significant penalty on modern
       RISC architectures.
       25. Change safemalloc() to request non-zero size when zero size
           is requested; some systems reject a malloc(0) request (e.g.
       IBM RS/6000 AIX 3.2).
       Bill Jones <jones@cs.usask.ca> 93/09/29
       26. MAXFIELDS set to 128, to resolve hash conflicts.
       27. New type String for assembling compound words, MAXSTRING of
       4095 as allowed by bibclean 2.07.  strncpy and kin used in
       hash table.
       28. Hash tables extend at 7/8 capacity.
   2.7 Bill Jones <jones@cs.usask.ca> 94/02/10
    1. size_t used for sizes of tables/lists to escape overflow
       when Index_t is short.
    2. More robust behaviour in the face of word or expansion
       buffer overflow.
    3. Changes for MSDOS use, from Guenter Rote.
   2.8 Jeff Erickson <jeffe@cs.berkeley.edu> 10 Jan 95
        1. Changed some die()s to warn()s and added some error
       recovery code to be more like bibtex.
    2. Changed parsing of @comment to conform with bibtex, even
       though it's totally drain-bamaged.
    3. Changed iskeychar to conform with bibtex implementation
       rather than bibtex documentation.  Unfortunately, this
       means that some things are indexed that can't be looked at
       in biblook, since biblook ignores most non-alphanumeric
       input.
    4. Warn the user again at the end if there were any warnings.
       If there was a warning, there's probably extra garbage in
       the index tables.
       Bill Jones <jones@cs.usask.ca> 95/01/19
    5. Renovations to remove need for short/long Index_t tradeoff.
       Index_t now always 4 bytes internally, but entry reference
       lists written to disk in compressed form.  File version 4.
    6. Changes to hashing code.  New HashWord collects code common to
       GetHashCell and InHashCell.  Calculation now truncates at
       sizeof(Word), fixes bug where words differing beyond storage
       truncation point had distinct hash entries.  Performance tweak,
       % can be done once, outside inner loop.  Overflow bumped to
       15/16 to save memory; profiling shows collisions still
       miniscule on average.
    7. Typedefs and casts necessitated by Alpha, where long is 64
       bits, to keep index file quantities 2 or 4 bytes as intended.
       (Thanks to Markus Stoll.)
    8. Non-USASCII chars ignored, since their char classification
       will differ with ISO locale.
    9. BADWORDS trimmed to a top 20ish English list.
   2.9 Bill Jones <jones@cs.usask.ca> 98/03/30
    1. Pattern matching support, from Sariel Har-Peled, in biblook.

\* ================================================================= */

#include "biblook.h"

/* ======================= UTILITY FUNCTIONS ======================= */

static long line_number = 1L;           /* for debug messages */
static long initial_line_number = 1L;
static int warnings = 0;                /* How many warnings so far? */

/* ----------------------------------------------------------------- *\
|  void die(const char *msg1, const char *msg2)
|
|  Print an error message and die.
\* ----------------------------------------------------------------- */
void die(const char *msg1, const char *msg2)
{
    (void)fprintf(stderr, "\nError:\t in BibTeX entry starting at line %ld, ",
        initial_line_number);
    (void)fprintf(stderr, "error detected at line %ld:\n",
        line_number);
    (void)fprintf(stderr, "\t%s %s\n", msg1, msg2);
    exit(EXIT_FAILURE);
}

/* ----------------------------------------------------------------- *\
|  void diechar(const char *msg1, const char msg2)
|
|  Print an error message and die.  Called when second message is a
|  single character
\* ----------------------------------------------------------------- */
void diechar(const char *msg1, const char msg2)
{
    char tmp[2];
    tmp[0] = msg2;
    tmp[1] = 0;
    die(msg1, tmp);
}

/* ----------------------------------------------------------------- *\
|  void warn(const char *msg1, const char *msg2)
|
|  Print an error message and return.
\* ----------------------------------------------------------------- */
void warn(const char *msg1, const char *msg2)
{
    (void)fprintf(stderr, "\nWarning: %s %s (at line %ld)\n",
        msg1, msg2, line_number);
    warnings++;
}

/* ----------------------------------------------------------------- *\
|  void warnchar(const char *msg1, const char msg2)
|
|  Print an error message and return.  Called when second message is a
|  single character
\* ----------------------------------------------------------------- */
void warnchar(const char *msg1, const char msg2)
{
    char tmp[2];
    tmp[0] = msg2;
    tmp[1] = 0;
    warn(msg1, tmp);
}

/* ----------------------------------------------------------------- *\
|  char safegetc(FILE *fp, const char *what)
|
|  Get the next character safely.  Used by routines that assume that
|  they won't run into the end of file.
\* ----------------------------------------------------------------- */
char safegetc(FILE *fp, const char *what)
{
    register int c;

    if ((c = getc(fp)) == '\n')
        ++line_number;
    else if (c == EOF)
        die("Unexpected end of file", what);
    return ((char)c);
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
        die(msg1, msg2);

    return tmp;
}

/* ====================== HASH TABLE FUNCTIONS ===================== *\

   The hash tables start small and double whenever they reach 15/16
   capacity.  Hashing is performed by going through the string one
   character at a time, multiplying by a constant and adding in the
   new character value each time.  The constant is defined to give
   the same even spread (about size/sqrt(2)) between successive
   multiples, as long as the hash table size is a power of two.

   Collisions are resolved by double hashing.  Since the hash table
   size is always a power of two, the secondary hash value has to be
   odd to avoid loops.

   The field tables are non-extensible hash tables, otherwise handled
   the same way.  It is probably well worth the effort to fine tune
   the field table hash function in order to avoid collisions.

   The field tables associated with ignored fields are black holes.
   Everything is the same, except that InsertEntry doesn't actually
   DO anything.

\* ================================================================= */

#define MAXFIELDS 256       /* small power of 2 */
#define INIT_HASH_SIZE 256  /* power of 2, >= MAXFIELDS */
#define HASH_CONST 1482907  /* prime close to 2^{20.5} */

typedef struct {            /* Hash table entry for index/abbrev tables */
    Word theword;	        /* the hashed word/abbreviation */
    Index_s number;         /* number of refs/words in the list */
    size_t size;	        /* real size of reference/word list */
                            /* need sizeof(size_t) > sizeof(Index_s) */

    /* --- Index tables only --- */
    Index_t *refs;          /* actual list of references */

    /* --- Abbreviation table only --- */
    Index_t entry;          /* entry containing definition */
    Word *words;            /* list of words in expansion */
} HashCell, *HashPtr;

typedef struct {            /* Extensible hash table */
    Word thekey;	        /* field name or "@string" */
    Index_t number;         /* number of words in the table */
    size_t size;	        /* real size of the table */
    HashPtr words;	        /* index hash table */
} ExHashTable;

static ExHashTable fieldtable[MAXFIELDS]; /* the field tables */
static Index_s numfields;				  /* number of fields */
static ExHashTable abbrevtable[1];		  /* the abbrev table */
static ExHashTable badwordtable[1];		  /* the badword table */

/* ----------------------------------------------------------------- *\
|  void InitOneField(ExHashTable *htable)
|
|  Initialize one field's hash table
\* ----------------------------------------------------------------- */
void InitOneField(register ExHashTable *htable)
{
    Index_t i;

    htable->number = 0;
    htable->size = INIT_HASH_SIZE;

    htable->words = (HashPtr)safemalloc(INIT_HASH_SIZE * sizeof(HashCell),
        "Can't create hash table for", htable->thekey);
    for (i = 0; i < INIT_HASH_SIZE; i++) {
        htable->words[i].theword[0] = 0;
        htable->words[i].number = 0;
        htable->words[i].size = 0;
        htable->words[i].refs = NULL;
        htable->words[i].entry = INDEX_NAN;
        htable->words[i].words = NULL;
    }
}

/* ----------------------------------------------------------------- *\
|  void InitTables(void)
|
|  Initialize the field tables
\* ----------------------------------------------------------------- */
void InitTables(VOID)
{
    register unsigned int i;

    numfields = 0;
    for (i = 0; i < MAXFIELDS; i++) {
        fieldtable[i].thekey[0] = 0;
        fieldtable[i].number = 0;
        fieldtable[i].size = 0;
        fieldtable[i].words = NULL;
    }

    strcpy(abbrevtable->thekey, "abbreviations");
    abbrevtable->number = 0;
    abbrevtable->size = 0;
    abbrevtable->words = NULL;

    InitOneField(abbrevtable);

    strcpy(badwordtable->thekey, "bad words");
    badwordtable->number = 0;
    badwordtable->size = 0;
    badwordtable->words = NULL;

    InitOneField(badwordtable);
}

/* ----------------------------------------------------------------- *\
|  void FreeTables(void)
|
|  Free the tables
\* ----------------------------------------------------------------- */
void FreeTables(VOID)
{
    register unsigned int i;
    Index_t j;

    for (i = 0; i < (unsigned int)numfields; i++) {
        if (fieldtable[i].words) {
            for (j = 0; j < fieldtable[i].number; j++)
                if (fieldtable[i].words[j].refs)
                    free(fieldtable[i].words[j].refs);

            free(fieldtable[i].words);
        }
    }

    if (abbrevtable->words) {
        for (j = 0; j < abbrevtable->number; j++)
            if (abbrevtable->words[j].words)
                free(abbrevtable->words[j].words);

        free(abbrevtable->words);
    }

    if (badwordtable->words) {
        for (j = 0; j < badwordtable->number; j++)
            if (badwordtable->words[j].words)
                free(badwordtable->words[j].words);

        free(badwordtable->words);
    }
}

/* ----------------------------------------------------------------- *\
|  ExHashTable *GetHashTable(const char *field)
|
|  Get the hash table associated with the given key -- field name or
|  "@string".  If the table is unclaimed, claim it and initialize it.
\* ----------------------------------------------------------------- */
ExHashTable *GetHashTable(const char *field)
{
    register unsigned long hash = 0;    /* primary hash value	*/
    register unsigned long skip = 1;    /* secondary hash value */
    register int i;
    register ExHashTable *cell;

    for (i = 0; field[i] && (i < ((int)sizeof(Word)) - 1); i++) {
        hash = hash * HASH_CONST + field[i];
        skip += 2 * hash;
    }
    hash &= MAXFIELDS - 1;              /* MAXFIELDS power of 2 */

    /* cell not empty, and not the right word */
    while (cell = fieldtable + hash, cell->thekey[0] && strncmp(cell->thekey,
            field, sizeof(Word) - 1)) {
        hash = (hash + skip) & (MAXFIELDS - 1);
    }

    if (!cell->thekey[0]) {
        strncpy(cell->thekey, field, sizeof(Word));
        if (strlen(field) > sizeof(Word) - 1) {
            cell->thekey[sizeof(Word) - 1] = 0;
            warn("truncated field name:", cell->thekey);
        }
        InitOneField(cell);
        numfields++;
        if (numfields >= MAXFIELDS) /* NB: NOT > because that produces */
            /* an infinite while() loop above on next entry! */
            die("too many field names", field);
    }

    return cell;
}

/* ----------------------------------------------------------------- *\
|  void InitBlackHole(char *field)
|
|  Initialize a black hole for the given field
\* ----------------------------------------------------------------- */
void InitBlackHole(char *field)
{
    ExHashTable *hole;

    hole = GetHashTable(field);
    if (hole->words != NULL) {
        free(hole->words);
        hole->words = NULL;
    }
}

/* ----------------------------------------------------------------- *\
|  int IsBlackHole(ExHashTable *htable)
|
|  Is the given hash table a black hole?
\* ----------------------------------------------------------------- */
#define IsBlackHole(htable) ((htable)->words == NULL)

/* ----------------------------------------------------------------- *\
|  HashPtr HashWord(ExHashTable *htable, const char *word)
|
|  Hashing computation and table search.
\* ----------------------------------------------------------------- */
HashPtr HashWord(ExHashTable *htable, register const char *word)
{
    register unsigned long hash = 0;    /* primary hash value	*/
    register unsigned long skip = 1;    /* secondary hash value */
    register int i;
    register HashPtr cell, table;

    table = htable->words;

    for (i = 0; word[i] && (i < ((int)sizeof(Word)) - 1); i++) {
        hash = (hash * HASH_CONST + word[i]);
        skip += 2 * hash;
    }
    hash &= htable->size - 1;           /* size power of 2 */

    /* cell not empty, and not the right word */
    while (cell = table + hash, cell->theword[0] && strncmp(cell->theword,
            word, sizeof(Word) - 1)) {
        hash = (hash + skip) & (htable->size - 1);
    }

    return cell;
}

/* ----------------------------------------------------------------- *\
|  HashPtr GetHashCell(ExHashTable *htable, const char *word)
|
|  Get the hash table cell associated with the given word/abbrev.
|  If the cell is unclaimed, claim it, initialize it, and update
|  the table's word count.
\* ----------------------------------------------------------------- */
HashPtr GetHashCell(ExHashTable *htable, const char *word)
{
    register HashPtr cell;

    cell = HashWord(htable, word);
    if (!cell->theword[0]) {    /* if cell isn't initialized yet... */
        strncpy(cell->theword, word, sizeof(Word));
        if (strlen(word) > sizeof(Word) - 1) {
            cell->theword[sizeof(Word) - 1] = 0;
            warn("truncated word:", cell->theword);
        }
        cell->size = 4;

        if (htable == abbrevtable) {
            cell->words = (Word *)safemalloc(cell->size * sizeof(Word),
                "Can't store expansion for", word);
        } else if (htable == badwordtable) {
            cell->words = (Word *)safemalloc(cell->size * sizeof(Word),
                "Can't store ignorable word", word);
        } else {
            cell->refs = (Index_t *)safemalloc(cell->size * sizeof(Index_t),
                "Can't create entry list for", word);
        }
        htable->number++;
    }
    return cell;
}
/* ----------------------------------------------------------------- *\
|  int InHashCell(ExHashTable *htable, const char *word)
|
|  Return 1 if word is in table, and 0 otherwise.
\* ----------------------------------------------------------------- */
int InHashCell(ExHashTable *htable, const char *word)
{
    return (HashWord(htable, word)->theword[0] ? 1 : 0);
}

/* ----------------------------------------------------------------- *\
|  void ExtendHashTable(ExHashTable *htable)
|
|  Double the size of the hash table and rehash everything.
\* ----------------------------------------------------------------- */
void ExtendHashTable(ExHashTable *htable)
{
    register HashPtr newcell;
    register HashPtr oldtable;
    size_t i;
    Index_t oldsize;

    oldsize = htable->size;
    oldtable = htable->words;

    htable->number = 0;
    htable->size *= 2;
    if (htable->size <= 0)
        die("hash type overflow:", htable->thekey);
    htable->words = (HashPtr)safemalloc(sizeof(HashCell) * htable->size,
        "Can't extend hash table for", htable->thekey);

    for (i = 0; i < htable->size; i++) {
        htable->words[i].theword[0] = 0;
        htable->words[i].number = 0;
        htable->words[i].size = 0;
        htable->words[i].refs = NULL;
        htable->words[i].entry = INDEX_NAN;
        htable->words[i].words = NULL;
    }

    for (i = 0; i < oldsize; i++) {
        if (oldtable[i].theword[0]) {
            newcell = GetHashCell(htable, oldtable[i].theword);
            *newcell = oldtable[i];
        }
    }

    free(oldtable);
}

/* ----------------------------------------------------------------- *\
|  void InsertEntry(ExHashTable *htable, char *word, Index_t entry)
|
|  Insert the word/entry pair into the hash table, unless it's
|  already there.  Assumes htable is not the abbreviation table.
\* ----------------------------------------------------------------- */
void InsertEntry(ExHashTable *htable, char *word, Index_t entry)
{
    register HashPtr cell;
    Index_t *newlist;

    if (IsBlackHole(htable))
        return;

    if (htable->number * (unsigned long)16 >
        htable->size * (unsigned long)15)   /* unsigned long, NOT Index_t */
        ExtendHashTable(htable);

    cell = GetHashCell(htable, word);

    if (cell->number && (cell->refs[cell->number - 1] == entry))
        return;

    if (cell->number == cell->size) {       /* expand the array */
        cell->size *= 2;
        if (cell->size <= 0)
            die("hash type overflow:", htable->thekey);
        newlist = (Index_t *)safemalloc(cell->size * sizeof(Index_t),
            "Can't extend entry list for", word);

        bcopy(cell->refs, newlist, cell->number * sizeof(Index_t));
        free(cell->refs);
        cell->refs = newlist;
    }
    cell->refs[cell->number++] = entry;
}

/* ----------------------------------------------------------------- *\
|  void InsertExpansion(HashPtr cell, const char *theword);
|
|  Insert a new word into the given abbreviation's expansion.
|  Doesn't bother checking for duplicate words [yet].
\* ----------------------------------------------------------------- */
void InsertExpansion(register HashPtr cell, const char *theword)
{
    Word *newlist;

    /* This should never happen... */
    if (cell->number == cell->size) {       /* expand the array */
        cell->size *= 2;
        if (cell->size <= 0)
            die("hash type overflow:", "abbreviations");

        newlist = (Word *)safemalloc(cell->size * sizeof(Word),
            "Can't extend expansion list for", cell->theword);

        bcopy(cell->words, newlist, cell->number * sizeof(Word));
        free(cell->words);
        cell->words = newlist;
    }
    strncpy(cell->words[cell->number], theword, sizeof(Word));
    if (strlen(theword) > sizeof(Word) - 1) {
        (cell->words[cell->number])[sizeof(Word) - 1] = 0;
        warn("truncated expansion:", cell->words[cell->number]);
    }
    cell->number++;
}

/* ----------------------------------------------------------------- *\
|  Standard bibtex abbreviations.  The journal list is taken from
|  "plain.bst", but the journal abbreviations conform (wherever
|  possible) to the computational geometry bibliography's standard.
|  In particular, these may disagree with the abbreviations used in
|  standard ".bst" files.  See the file "authority" for details
|  on the abbreviation standard.  Journal abbreviations are
|  included automatically iff the macro STD_J_ABBREVS is defined.
\* ----------------------------------------------------------------- */
typedef struct {
    Word abbrev;
    short number;
    Word words[10];
} StdAbbrev;

#ifdef STD_J_ABBREVS
#define NUM_STD_ABBR 32
#else
#define NUM_STD_ABBR 12
#endif

const StdAbbrev std_abbr[] = {
        {"jan", 1, {"january"}},
        {"feb", 1, {"february"}},
        {"mar", 1, {"march"}},
        {"apr", 1, {"april"}},
        {"may", 1, {"may"}},
        {"jun", 1, {"june"}},
        {"jul", 1, {"july"}},
        {"aug", 1, {"august"}},
        {"sep", 2, {"sept", "september"}},
        {"oct", 1, {"october"}},
        {"nov", 1, {"november"}},
        {"dec", 1, {"december"}},

#ifdef LOGABBREVS
        {"acmcs", 3, {"acm", "comput", "surv"}},
        {"acta", 2, {"acta", "inform"}},
        {"cacm", 2, {"commun", "acm"}},
        {"ibmjrd", 3, {"ibm", "res", "develop"}},
        {"ibmsj", 2, {"ibm", "syst"}},
        {"ieeese", 4, {"ieee", "trans", "softw", "eng"}},
        {"ieeetc", 3, {"ieee", "trans", "comput"}},
        {
            "ieeetcad",
            7,
            {"ieee", "trans", "comput", "aided", "design",
             "integrated", "circuits"},
            {"ipl", 3, {"inform", "process", "lett"}},
            {"jacm", 1, {"acm"}},
            {"jcss", 3, {"comput", "syst", "sci"}},
            {"scp", 3, {"sci", "comput", "program"}},
            {"sicomp", 2, {"siam", "comput"}},
            {"tocs", 4, {"acm", "trans", "comput", "syst"}},
            {"tods", 4, {"acm", "trans", "database", "syst"}},
            {"tog", 3, {"acm", "trans", "graph"}},
            {"toms", 4, {"acm", "trans", "math", "softw"}},
            {"toois", 5, {"acm", "trans", "office", "inform", "syst"}},
            {"toplas", 5, {"acm", "trans", "program", "lang", "syst"}},
            {"tcs", 3, {"theoret", "comput", "sci"}},
#endif
        };

/* ----------------------------------------------------------------- *\
|  void StandardAbbrevs(void)
|
|  Put the standard abbreviations into the abbreviation table.
\* ----------------------------------------------------------------- */
void StandardAbbrevs(VOID)
{
    register HashPtr thecell;
    register int i, j;

    for (i = 0; i < NUM_STD_ABBR; i++) {
        thecell = GetHashCell(abbrevtable, std_abbr[i].abbrev);

        thecell->entry = INDEX_BUILTIN;
        for (j = 0; j < std_abbr[i].number; j++)
            InsertExpansion(thecell, std_abbr[i].words[j]);
    }
}

const char *badwords[] = BADWORDS;

/* ----------------------------------------------------------------- *\
|  void StandardBadWords(void)
|
|  Put the standard ignorable words into the badword table.
\* ----------------------------------------------------------------- */
void StandardBadWords(VOID)
{
    register int i;

    for (i = 0; badwords[i] != (const char *)NULL; i++)
        (void)GetHashCell(badwordtable, badwords[i]);
}

/* ============================= INPUT ============================= *\

   I'd like this to work with more than just the CG bib, so I can't
   assume very much about the input.  In particular, all white space
   (blanks, tabs, and newlines) is identical most of the time.  On
   the other hand, it would be nice to include any "comments" that
   obviously go with an entry as part of that entry.  Consequently,
   two newlines in a row (possibly separated by other white space) is
   considered a break between entries.  This will give us bogus
   entries for stray "comments", but we can take care that easily
   enough -- an entry is only real if it contains a @ character.

\* ================================================================= */

/* ----------------------------------------------------------------- *\
|  unsigned long FindNextEntry(FILE *ifp)
|
|  Return the file offset to the next entry in the bib file.  On exit,
|  the file pointer is left just after the "@".  The entry officially
|  begins after the most recent blank line, the end  of the previous
|  entry, or the beginning of the file.  Returns "-1" if there is no
|  next entry.  It is the CALLER's responsibility to determine the type
|  of entry (normal or @string or @comment or @preamble or error).
\* ----------------------------------------------------------------- */
unsigned long FindNextEntry(FILE *ifp)
{
    char ch;
    char blank = 0;                     /* 1 if current line is blank so far */
    unsigned long offset;

    offset = ftell(ifp);
    ch = getc(ifp);
    if (ch == '\n') {
        line_number++;
        offset++;
    }
    initial_line_number = line_number;  /* record for errors */

    for (;;) {
        if (feof(ifp))
            return (unsigned long)-1;

        if (ch == '@') {                /* got an entry */
            return offset;
        } else if (ch == '\n') {
            if (blank) {
                offset = ftell(ifp);
                initial_line_number = line_number;
            }
            blank = 1;
        } else if (!isspace(ch)) {
            blank = 0;
        }

        ch = getc(ifp);
        if (ch == '\n')
            line_number++;
    }
}

/* ----------------------------------------------------------------- *\
|  int GetNextWord(FILE *ifp, char *word)
|
|  Get the next word in the current field.  A word is any contiguous
|  set of letters and numbers, AFTER the following steps:
|	1. Letters are folded to lower case.  Thus, "Voronoi" is
|	   returned as "voronoi"
|	2. All TeX commands, except those in math expressions, are
|	   removed, but their arguments are left behind.  Thus,
|	   "Erd{\H o}s" is returned as "erdos".
|	3. All other non-word characters are removed.  Non-word
|	   characters inside {{possibly} nested} braces or dollar
|          signs do not delimit words, so they may cause unexpected
|	   results.  Thus, "{this example}" is returned as
|	   "thisexample".  (But see below.)
|	4. TeX commands in math expressions are considered normal
|	   text.  Thus, "$O(n\log^2 n)$" is returned as "onlog2n"
|	   instead of "onn".  (But see below.)  This occasionally
|	   gives unexpected or unreadable results.
|	5. Apostrophes, brackets, and hyphens are ignored (but see
|	   below).  Thus, "{\'O}'D{\'u}nlaing" as "odunlaing",
|	   "J[ohn]" as "john", and "half-space" as "halfspace".
|
|  Compound words are words with several components, like "half-space"
|  or "{van Dam}".  Compound words are written into the word buffer
|  with their components separated by null characters.  There are
|  three ways to get compound words:
|	1. Hyphens separate components.  Thus, "half-space" is really
|	   returned as "half\0space".  However, "18--21" is returned
|	   as two separate words.
|	2. Within braces, white space delimits components.  Thus,
|	   "{van Dam}" is really returned as "van\0dam".
|	3. Within math expressions, components are delimited by ANY
|	   string of non-alphanumeric characters.  For example,
|	   "$\Omega(n\log n)$" is really returned as
|	   "omega\0n\0log\0n".
|
|  Returns 0 if no word was found, 1 if a simple word was found, and
|  the number of components if a compound word was found.
|
|  The file pointer is left at the character immediately following the
|  word.  The input is assumed to be syntactically correct: unbalanced
|  braces, math delimiters, or quotation marks will cause errors.
\* ----------------------------------------------------------------- */
int GetNextWord(FILE *ifp, register char *word)
{
    register char ch = ' ';
    char braces = 0;            /* levels of indented braces */
    char math = 0;              /* 1 if in math expression */
    char incmd = 0;             /* 1 if reading TeX command */
    char done = 0;              /* 1 if word is complete */
    char btwn = 1;	            /* 1 if "between" components */
    int nchars = 0;	            /* how many characters in word? */
    int nwords = 0;	            /* how many words have I seen? */

#if DEBUG
    char *start_word = word;    /* DEBUG */
#endif						    /* DEBUG */

    while (!done) {
        ch = safegetc(ifp, "reading next word");

#if DEBUG
        if ((int)(word - start_word) > (MAXSTRING - 3)) {
            (void)printf("WARNING: word = %-.*s\n", MAXWORD, start_word);
        }
#endif /* DEBUG */

        if (!isascii(ch)) {         /* high bit set */
            char buf[32];
            (void)sprintf(buf, "%c (\\%03o)", (unsigned char)ch,
                (unsigned char)ch);
            warn("nonascii char, ignoring: ", buf);
        } else if (isalpha(ch)) {   /* letters */
            if (!incmd) {           /* may be part of TeX command */
                if (btwn) {
                    nwords++;
                    btwn = 0;
                }
                if (++nchars <= MAXSTRING)  /* ignore overflow */
                    *word++ = tolower(ch);
            }
        } else if (isdigit(ch)) {   /* digits */
            incmd = 0;
            if (btwn) {
                nwords++;
                btwn = 0;
            }
            if (++nchars <= MAXSTRING)
                *word++ = ch;
        } else if (math) {          /* other char in math mode */
            if (ch == '$') {        /* end math mode */
                math = 0;
                braces--;
            } else if (!btwn) {
                if (++nchars <= MAXSTRING)
                    *word++ = 0;
                btwn = 1;
            }
        } else if (ch == '\\') {    /* beginning of TeX command */
            ch = safegetc(ifp, "reading next word");
            if (isalpha(ch))
                incmd = 1;
        } else if (ch == '{') {     /* left brace */
            incmd = 0;
            braces++;
        } else if (ch == '}') {     /* right brace */
            incmd = 0;
            if (!braces) {
                ungetc(ch, ifp);
                done = 1;
            } else {
                braces--;
            }
        } else if (ch == '"') {     /* double quote */
            incmd = 0;
            if (!braces) {
                ungetc(ch, ifp);
                done = 1;
            }
        } else if (ch == '$') {    /* begin math mode */
            incmd = 0;
            math = 1;
            braces++;
        } else if (ch == '-' && !btwn) {    /* single hyphens */
            if (++nchars <= MAXSTRING)
                *word++ = 0;
            btwn = 1;
        } else if (isspace(ch) && braces) { /* white space */
            if (incmd) {
                incmd = 0;
            } else if (!btwn) {
                if (++nchars <= MAXSTRING)
                    *word++ = 0;
                btwn = 1;
            }
        } else if (incmd) {                 /* other characters */
            incmd = 0;
        } else if ((ch == '\'') || (ch == '[') || (ch == ']')) {
                                            /* do nothing */;
        } else if ((nwords != 0) && !braces) {
            done = 1;
        }

#if DEBUG
        if ((int)(word - start_word) > (MAXSTRING - 3))
            (void)printf("WARNING: word = %-.*s\n", MAXWORD, start_word);

#endif /* DEBUG */

#if 0
    (void)printf("%c {%d} $%d$ \\%d -%d- #%2d/%d\n",
        ch, braces, math, incmd, btwn, nchars, nwords);
#endif
        /* Two situations can produce an unusually long `word' that
        exceeds MAXSTRING characters:
           (a) a long compound word
           (b) a long braced string (e.g. braces added to protect
               a complete title from lower-casing in certain
               bibliography styles)
        In either case, we bail out early, making sure to properly
        terminate the string. */

        if (nchars > MAXSTRING) {
            done = 1; /* bail out early if buffer full */
            *word = 0;
        }
    }

    if (!btwn)
        *word = 0;

#if DEBUG
    if (strlen(start_word) > (size_t)MAXSTRING) {
        (void)printf("ERROR: word:%d = [%s]\n", strlen(start_word), start_word);
    }
#endif /* DEBUG */
    return nwords;
}

/* ----------------------------------------------------------------- *\
|  int iskeychar(char c, int first_char)
|
|  Return 1 if c is a valid keyword character, and 0 otherwise.
|  The rules are different for the first character, so first_char
|  must be set non-zero to select that case.
|
|  The code in bibtex.web in the section on character set dependencies
|  creates a character classification array, id_class[], which permits
|  the following characters (verified by experiment too) in field
|  names:
|      A-Z a-z 0-9
|      ! $ & * + - . / : ; < > ? @ [ \ ] ^ _ ` | ~ <DELete>
|  In other words, every printing character except the following.
|      " # % ' ( ) , = { }
|  [bibtex.web 33]
|
|  Despite claims in the documentation that say the first character
|  must be a letter, it can really be anything but a digit.
|  [bibtex.web 90]
\* ----------------------------------------------------------------- */
int iskeychar(char c, int first_char)
{
    if (isalpha(c))
        return 1;
    else if (isdigit(c))
        return !first_char;
    else if (iscntrl(c))
        return 0;
    else
        return (strchr(NONKEYCHARS, (int)c) == (char *)NULL);
}

/* ----------------------------------------------------------------- *\
|  int IsRealWord(char *theword)
|
|  Is this a word I really want to index?
\* ----------------------------------------------------------------- */
int IsRealWord(register char *theword)
{
#if 0
    register int i;
#endif

    if (!theword[0] || !theword[1]) /* empty or 1 character */
        return 0;
#if IGNORENONE
    return 1;
#endif

#if 0
    for (i=0; badwords[i]; i++)
    if (!strcmp(theword, badwords[i]))
        return 0;
    return 1;
#else
    return (!InHashCell(badwordtable, theword));
#endif
}

/* ----------------------------------------------------------------- *\
|  char MungeField(FILE *ifp, void (*action)(char *, void *, void *)),
|		   void *arg1, void *arg2)
|
|  Munge the current field.  For every word in the field, call
|  (*action), passing in the word and the args.  If the word is an
|  abbreviation, call (*action) on the words in its expansion.
|
|  On entrance, the file pointer is just after the field name.  On
|  exit, the file pointer is on the comma or closing character for
|  the entry.
|
|  If a parsing error is encountered, warn the user and return 0.  If
|  the field is munged successfully, return 1.
\* ----------------------------------------------------------------- */
char MungeField(FILE *ifp, void (*action)(char *, void *, void *),
    void *arg1, void *arg2)
{
    register char ch;
    register int i, nwords;
    register char *tmp, *tmp2;
    Index_t k;
    String nextword;    /* big, to survive over-embraced titles */
    HashPtr abbrevcell;

    ch = safegetc(ifp, "looking for =");
    while (isspace(ch))
        ch = safegetc(ifp, "looking for =");

    if (ch != '=') {
        warnchar("= expected after field name: ", ch);
        return 0;
    }

    for (;;) {
        ch = safegetc(ifp, "looking for open quote/brace");
        while (isspace(ch))
            ch = safegetc(ifp, "looking for open quote/brace");

        if ((ch == '{') || (ch == '"')) {
            nwords = GetNextWord(ifp, nextword);

            while (nwords != 0) {
                if (nwords != 1) {          /* compound word */
                    tmp = nextword;

                    for (i = 0; i < nwords; i++) {
                        if (IsRealWord(tmp))
                            (*action)(tmp, arg1, arg2);
                        tmp += strlen(tmp) + 1;
                    }

                    tmp = nextword + strlen(nextword);
                    tmp2 = tmp + 1;

                    while (nwords > 1) {    /* recombine the components */
                        if (!*tmp2) {
                            tmp2++;
                            nwords--;
                        } else {
                            *tmp++ = *tmp2++;
                        }
                    }
                    *tmp = 0;
                }

                if (IsRealWord(nextword))
                    (*action)(nextword, arg1, arg2);

                nwords = GetNextWord(ifp, nextword);
            }
            ch = safegetc(ifp, "reading close quote/brace");
            ch = safegetc(ifp, "looking for comma or close brace");
        }
        else if (isdigit(ch)) {             /* number */
            for (i = 0; isdigit(ch); i++) {
                if (i >= MAXWORD) {
                    nextword[MAXWORD] = 0;
                    die("word buffer overflow", nextword);
                }
                nextword[i] = tolower(ch);
                ch = safegetc(ifp, "reading number");
            }
            nextword[i] = 0;
            nextword[MAXWORD] = 0;
            (*action)(nextword, arg1, arg2);
        } else if (iskeychar(ch, 1)) {      /* abbreviation */
            for (i = 0; iskeychar(ch, i == 0); i++) {
                if (i >= MAXWORD) {
                    nextword[MAXWORD] = 0;
                    die("word buffer overflow", nextword);
                }
                nextword[i] = tolower(ch);
                ch = safegetc(ifp, "reading abbreviation");
            }
            nextword[i] = 0;
            nextword[MAXWORD] = 0;

            (*action)(nextword, arg1, arg2);

            /* --- Munge the abbreviation's expansion, too --- */

            abbrevcell = GetHashCell(abbrevtable, nextword);
            if (abbrevcell->entry == INDEX_NAN)
                warn("Undefined abbreviation:", nextword);

            for (k = 0; k < abbrevcell->number; k++)
                (*action)(abbrevcell->words[k], arg1, arg2);
        } else {
            warnchar("Illegal character after =:", ch);
            return 0;
        }

        while (isspace(ch))
            ch = safegetc(ifp, "looking for comma, close brace, or #");

        if ((ch == ',') || (ch == '}') || (ch == ')')) {
            ungetc(ch, ifp);
            return 1;
        } else if (ch == '#') {
            continue;
        } else {
            warnchar("Expected } or , or # after field string:", ch);
            return 0;
        }
    }
}

/* ----------------------------------------------------------------- *\
|  void MF_InsertExpansion(char *word, ExHashTable *htable, HashPtr cell)
|
|  Version of InsertEntry for passing to MungeField
\* ----------------------------------------------------------------- */
void MF_InsertExpansion(char *word, ExHashTable *htable, HashPtr cell)
{
    InsertEntry(htable, word, cell->entry);
    InsertExpansion(cell, word);
}

/* ----------------------------------------------------------------- *\
|  void MF_InsertEntry(char *word, ExHashTable *htable, Index_t *entry)
|
|  Version of InsertEntry for passing to MungeField
\* ----------------------------------------------------------------- */
void MF_InsertEntry(char *word, ExHashTable *htable, Index_t *entry)
{
    InsertEntry(htable, word, *entry);
}

/* ----------------------------------------------------------------- *\
|  void MungeAbbrev(FILE *ifp, Index_t entry)
|
|  Wander though the abbreviation, putting the words into the
|  abbreviation table.  Looks a lot like MungeField, doesn't it?
|
|  If a parsing error is encountered, warn the user and return
|  immediately.  This makes us ignore everything up to the next @,
|  which is more or less what bibtex does in the same situation.
\* ----------------------------------------------------------------- */
void MungeAbbrev(FILE *ifp, Index_t entry)
{
    register char ch;
    register int i;
    Word theabbrev;
    HashPtr thecell;
    ExHashTable *htable;

    ch = safegetc(ifp, "looking for abbreviation");
    while (isspace(ch))
        ch = safegetc(ifp, "looking for abbreviation");

    if (!iskeychar(ch, 1)) {
        warnchar("Illegal character starting abbreviation:", ch);
        (void)fprintf(stderr, "\t I'm skipping the rest of this entry.\n");
        return;
    }

    for (i = 0; iskeychar(ch, i == 0); i++) {
        if (i >= MAXWORD) {
            theabbrev[i] = 0;
            die("abbreviation buffer overflow", theabbrev);
        }
        theabbrev[i] = tolower(ch);
        ch = safegetc(ifp, "reading abbreviation");
    }
    ungetc(ch, ifp); /* put back lookahead char */
    theabbrev[i] = 0;

    if (abbrevtable->number * (unsigned long)8 >
        abbrevtable->size * (unsigned long)7)
        ExtendHashTable(abbrevtable);

    htable = GetHashTable("@string");

    thecell = GetHashCell(abbrevtable, theabbrev);
    if (thecell->entry != INDEX_NAN)
        warn("Multiply-defined abbreviation:", theabbrev);

    thecell->entry = entry;

    MungeField(ifp, (void (*)(char *, void *, void *))MF_InsertExpansion,
               (void *)htable, (void *)thecell);

    ch = safegetc(ifp, "trying to read close brace");
}

/* ----------------------------------------------------------------- *\
|  void MungeRealEntry(FILE *ifp, Index_t entry)
|
|  Wander though the entry, mungeing each field.  On entry, the file
|  pointer is just after the opening brace/paren.
|
|  If a parsing error is encountered, warn the user and return
|  immediately.  This makes us ignore everything up to the next @,
|  which is exactly what bibtex does in the same situation.
\* ----------------------------------------------------------------- */
void MungeRealEntry(FILE *ifp, Index_t entry)
{
    register char ch;
    Word thefield;
    int i;
    ExHashTable *htable;

    ch = safegetc(ifp, "looking for citekey");
    while (isspace(ch))
        ch = safegetc(ifp, "looking for citekey");

    /* Pretty much anything can go in a bibtex key, including braces, */
    /* parens, quotes, and even chars that are illegal ANYWHERE else! */
    while (ch != ',')
        ch = safegetc(ifp, "reading citekey");

    while (ch == ',') {
        ch = safegetc(ifp, "looking for field descriptor");
        while (isspace(ch))
            ch = safegetc(ifp, "looking for field descriptor");

        if ((ch == '}') || (ch == ')')) /* allow trailing comma after */
            return;						/* last key = "value" entry */

        if (!iskeychar(ch, 1)) {
            warnchar("Illegal character starting field descriptor:", ch);
            (void)fprintf(stderr, "\t I'm skipping the rest of this entry.\n");
            return;
        }

        for (i = 0; iskeychar(ch, i == 0); i++) {
            if (i >= MAXWORD) {
                thefield[i] = 0;
                die("field name buffer overflow", thefield);
            }
            thefield[i] = tolower(ch);
            ch = safegetc(ifp, "reading field descriptor");
        }
        ungetc(ch, ifp);                /* put back lookahead char */
        thefield[i] = 0;

        htable = GetHashTable(thefield);
        if (!MungeField(ifp, (void (*)(char *, void *, void *))MF_InsertEntry,
                (void *)htable, (void *)&entry)) {
            (void)fprintf(stderr, "\t I'm skipping the rest of this entry.\n");
            return;
        }

        ch = safegetc(ifp, "trying to read comma or close brace");
    }
}

/* ----------------------------------------------------------------- *\
|  void SkipEntry(FILE *ifp)
|
|  Skip the current entry, assuming that we're starting at the
|  beginning.  Currently only used to skip @preamble's.
\* ----------------------------------------------------------------- */
void SkipEntry(FILE *ifp)
{
    char ch;
    int braces;
    int quotes;

    quotes = 0;
    braces = 0;
    ch = safegetc(ifp, "skipping false entry");

    while (quotes || braces || ((ch != '}') && (ch != ')'))) {
        if (ch == '{')
            braces++;
        else if (ch == '}')
            braces--;
        else if ((ch == '"') && !braces)
            quotes = !quotes;

        ch = safegetc(ifp, "skipping false entry");
    }
}

/* ----------------------------------------------------------------- *\
|  int MungeEntry(FILE *ifp, Index_t entry)
|
|  Determine whether the current entry is real or @comment or
|  @preamble, and munge the entry appropriately.  Return 1 if the
|  entry was real, or 0 if it was a "comment" or if there was a
|  parsing error AT THIS LEVEL.
|
|  If a parsing error is encountered, warn the user and return 0
|  immediately.  This makes us ignore everything up to the next @,
|  which is exactly what bibtex does in the same situation.
\* ----------------------------------------------------------------- */
int MungeEntry(FILE *ifp, Index_t entry)
{
    register char ch;
    Word therecord;
    int i;

    ch = safegetc(ifp, "looking for entry type");
    while (isspace(ch))
        ch = safegetc(ifp, "looking for entry type");

    if (!isalpha(ch)) {
        warnchar("Letter expected after @:", ch);
        (void)fprintf(stderr, "\t I'm skipping the rest of this entry.\n");
        return 0;
    }

    for (i = 0; iskeychar(ch, i == 0); i++) {
        if (i >= MAXWORD) {
            therecord[i] = 0;
            die("record name buffer overflow", therecord);
        }
        therecord[i] = tolower(ch);
        ch = safegetc(ifp, "recording entry type");
    }
    therecord[i] = 0;

    while (isspace(ch))
        ch = safegetc(ifp, "looking for open brace");

    if ((ch != '(') && (ch != '{')) {
        warnchar("{ or ( expected after entry type:", ch);
        (void)fprintf(stderr, "\t I'm skipping the rest of this entry.\n");
        return 0;
    }

    if (!strcmp(therecord, "string")) {
        MungeAbbrev(ifp, entry);
        return 1;
    } else if (!strcmp(therecord, "comment")) {
        /* Do nothing! [bibtex.web 241] */
        return 0;
    } else if (!strcmp(therecord, "preamble")) {
        SkipEntry(ifp);
        return 0;
    } else {
        MungeRealEntry(ifp, entry);
        return 1;
    }
}

/* ============================= OUTPUT ============================ */

/* ----------------------------------------------------------------- *\
|  void WriteWord(FILE *ofp, Word word)
|
|  Output the word in "Pascal" string format -- length byte followed
|  by characters.  This saves some disk space over writing MAXWORD+1
|  bytes in all cases.
\* ----------------------------------------------------------------- */
void WriteWord(FILE *ofp, Word word)
{
    unsigned char length = (unsigned char)strlen(word);

    /* Apply a sanity check: had this been here in the first place,
       a nasty bug introduced with compound word support would have
       been caught many hours sooner. */
    if (length > MAXWORD) {
        warn("punting on overflowed word buffer: ", word);
        length = MAXWORD;
        word[MAXWORD] = 0;
    }
    fwrite((void *)&length, sizeof(unsigned char), 1, ofp);
    fwrite((void *)word, sizeof(char), (size_t)length, ofp);
}

/* ----------------------------------------------------------------- *\
|  void NetOrderFwrite(void *vbuf, int s, int n, FILE *fp)
|
|  Write n elements of size s in network byteorder
\* ----------------------------------------------------------------- */
static void NetOrderFwrite(void *vbuf, size_t s, size_t n, FILE *fp)
{
    char *buf = (char *)vbuf;

    while (n-- > 0) {
        size_t rc;

        if (s == sizeof(uint16)) {
            uint16 x = htons(*(uint16 *)buf);
            rc = fwrite(&x, s, 1, fp);
        } else {                    /* assume s == sizeof(uint32) */
            uint32 x = htonl(*(uint32 *)buf);
            rc = fwrite(&x, s, 1, fp);
        }

        if (rc != 1) {
            perror("bibindex: cannot write; reason");
            exit(EXIT_FAILURE);
        }
                                    /* advance and continue: */
        buf += s;
    }
}

/* ----------------------------------------------------------------- *\
|  void SortTable(ExHashTable* htable)
|
|  Compress and sort a hash table.  KLUDGE -- Passing strcmp to qsort
|  assumes intimate knowledge of HashCell.
\* ----------------------------------------------------------------- */
void SortTable(register ExHashTable *htable)
{
    register HashPtr words;
    size_t m, n;

    words = htable->words;

    for (m = 0, n = 0; m < htable->size; m++) {
        if (words[m].theword[0]) {
            if (m > n) {
                words[n] = words[m];    /* copy mth table to nth */
                words[m].number = 0;    /* then clear mth table */
                words[m].size = 0;	    /* to avoid duplicate free() later */
                words[m].refs = (Index_t *)0;
            }
            n++;
        }
    }
    qsort(words, (size_t)htable->number, sizeof(HashCell),
          (int (*)(const void *, const void *))strcmp);
}

/* ----------------------------------------------------------------- *\
|  Index_s CompressRefs(char *p, Index_t *list, Index_t length)
|
|  Compress a sequence of Index_t.  Assumes and exploits redundancy
|  where sequence is monotonic increasing, with interterm difference
|  typically small.  Tests on a 68 MB concatenation of bibliographies
|  gave sample probabilities for difference bitlengths as
|
|  +	1	2	3	4	5	6	7	8
|  0	.22279	.06392	.06831	.07519	.07839	.07693	.07075	.06177
|  8	.05325	.04293	.03429	.02824	.02399	.02756	.02232	.02607
|  16	.01580	.00749
|
|  with expected value 6.177 bits.  To compress, we encode difference
|  values in a low-to-high nibble scheme where the low n bits are
|  difference data and the high (n+1)st bit indicates the value continues
|  to the next nibble.  From the data above, we can calculate the
|  relative storage used by n between 1 and 7 bits as
|
|	1.0910	0.8964	0.8693	0.8824	0.9149	0.9506	1.0000
|
|  so n of 3 looks optimal, but we use 7 bits here for convenience.
|  Difference range of this scheme is thus
|
|  with 1 bytes	127 entries
|	2	16383+127
|	3	2M+16383+127
|	4	256M+2M+16383+127
|
|  and testing on biblios from 1.7 to 68 MB showed average bytes per
|  difference as low as 1.25, but generally close to 1.40.
\* ----------------------------------------------------------------- */
Index_s CompressRefs(char *p, Index_t *list, Index_t length)
{
    Index_t prevref = (Index_t)-1;
    Index_t diff;
    char bits;
    char *p0 = p;

    while (length-- > 0) {
        diff = *list - prevref;

        while (diff > 0) {
            bits = diff & ~CHAR_HIGHBIT;
            if ((diff >>= (CHAR_BIT - 1)))
                bits |= CHAR_HIGHBIT;
            *p++ = bits;
        }
        prevref = *list++;
    }
    return p - p0;
}

/* ----------------------------------------------------------------- *\
|  Index_s WriteIndices(Index_t *list, Index_t length, FILE *ofp)
|
|  Compress and write an array of Index_t
\* ----------------------------------------------------------------- */
Index_s WriteIndices(Index_t *list, Index_t length, FILE *ofp)
{
    char *p;
    Index_s n;

    p = (char *)safemalloc(length * sizeof(Index_t),
        "can't allocate index list", "");
    n = CompressRefs(p, list, length);
    NetOrderFwrite((void *)&n, sizeof(Index_s), 1, ofp);
    if (fwrite(p, sizeof(char), n, ofp) != n) {
        perror("bibindex: cannot write indices; reason");
        exit(EXIT_FAILURE);
    }
    free(p);
    return n;
}

/* ----------------------------------------------------------------- *\
|  void OutputTables(FILE *ofp)
|
|  Compress and output the tables, with lots of user feedback.
|  KLUDGE -- Passing strcmp to qsort assumes intimate knowledge of
|  ExHashTable.
\* ----------------------------------------------------------------- */
void OutputTables(FILE *ofp)
{
    register HashPtr words;
    register int i, k;
    long count, numwords, numrefs;
    Index_t m;

    numwords = numrefs = 0;

    (void)printf("Writing index tables...");
    fflush(stdout);

    numfields = 0; /* recount, ignoring black holes */
    for (i = 0; i < MAXFIELDS; i++) {
        if (fieldtable[i].words) {
            if (i > (int)numfields) {
                fieldtable[(int)numfields] = fieldtable[i]; /* copy ith table */
                fieldtable[i].number = 0; /* then clear it  */
                fieldtable[i].size = 0; /* to avoid duplicate free() later */
                fieldtable[i].words = NULL;
            }
            numfields++;
        }
    }
    qsort(fieldtable, (size_t)numfields, sizeof(ExHashTable),
          (int (*)(const void *, const void *))strcmp);

    NetOrderFwrite((void *)&numfields, sizeof numfields, 1, ofp);
    for (i = 0; i < (int)numfields; i++)
        WriteWord(ofp, fieldtable[i].thekey);

    (void)printf("%d fields\n", (int)numfields);

    for (k = 0; k < (int)numfields; k++) {
        (void)printf("%3d. %-12s ", k + 1, fieldtable[k].thekey);
        fflush(stdout);

        SortTable(&(fieldtable[k]));
        NetOrderFwrite((void *)&(fieldtable[k].number), sizeof(Index_t), 1,
            ofp);
        count = 0;
        words = fieldtable[k].words;
        for (m = 0; m < fieldtable[k].number; m++) {
            WriteWord(ofp, words[m].theword);
            NetOrderFwrite((void *)&(words[m].number), sizeof(Index_s), 1, ofp);
            WriteIndices(words[m].refs, words[m].number, ofp);
            count += words[m].number;
        }

        (void)printf("%6ld words,%8ld refs,%7.2f refs/word\n",
            (long)fieldtable[k].number, count, (double)count /
            ((fieldtable[k].number == 0) ? 1.0 : (double)fieldtable[k].number));
        numwords += fieldtable[k].number;
        numrefs += count;
    }

    (void)printf("--- TOTAL ---    %7ld words,%8ld refs,%7.2f refs/word\n",
        numwords, numrefs, (double)numrefs / (double)((numwords == 0) ? 1 :
        numwords));
    putchar('\n');
    (void)printf("Writing abbrev table...");
    fflush(stdout);

    SortTable(abbrevtable);
    NetOrderFwrite((void *)&(abbrevtable->number), sizeof(Index_t), 1, ofp);

    words = abbrevtable->words;
    for (m = 0; m < abbrevtable->number; m++)
        WriteWord(ofp, words[m].theword);

    for (m = 0; m < abbrevtable->number; m++)
        NetOrderFwrite((void *)&(words[m].entry), sizeof(Index_t), 1, ofp);

    (void)printf("%d+%d abbreviations\n", abbrevtable->number - NUM_STD_ABBR,
        NUM_STD_ABBR);
}

/* ========================== MAIN PROGRAM ========================= */

/* ----------------------------------------------------------------- *\
|  IndexBibFile(FILE *ifp, FILE *ofp, char *filename)
|
|  Index a bibtex file.  Input comes from ifp; output goes to ofp.
|  Filename is the name of the bibliography, with no prefix.
\* ----------------------------------------------------------------- */
void IndexBibFile(FILE *ifp, FILE *ofp, char *filename)
{
    Index_t count = 0;
    long curoffset;
    Off_t *offsets;
    Off_t *oldoff;
    size_t offsize;
    time_t now = time(0);

    (void)printf("Indexing %s.bib.", filename);
    fflush(stdout);

    offsize = 128;                      /* MINIMUM OFFSET LIST SIZE */
    offsets = (Off_t *)malloc(offsize * sizeof(Off_t));

    while (!feof(ifp)) {
        curoffset = FindNextEntry(ifp);
        if (curoffset == (Off_t)-1)
            break;

        if (MungeEntry(ifp, count)) {
            offsets[count++] = (Off_t)curoffset;

            if (count == offsize) {     /* expand full offset array */
                oldoff = offsets;
                offsize *= 2;
                offsets = (Off_t *)malloc(offsize * sizeof(Off_t));
                bcopy(oldoff, offsets, count * sizeof(Off_t));
                free(oldoff);
            }

            if (!(count % 200)) {       /* feedback */
                if (count % 1000)
                    putchar('.');
                else
                    (void)printf("%d.", count);
                fflush(stdout);
            }
        }
    }

    (void)printf("done.\n");

    (void)fprintf(ofp, "bibindex %d %d %d %s", FILE_VERSION, MAJOR_VERSION,
        MINOR_VERSION, ctime(&now));

    (void)printf("Writing offset table...");
    fflush(stdout);
    NetOrderFwrite((void *)&count, sizeof(Index_t), 1, ofp);
    NetOrderFwrite((void *)offsets, sizeof(Off_t), count, ofp);
    free(offsets);
    (void)printf("%d entries\n", count);

    OutputTables(ofp);

    if (warnings) {
        (void)printf("\nWarning: %d problems were encountered.\n", warnings);
        (void)printf("\t Biblook may give unexpected results.\n\n");
    }

    (void)printf("All done!\n");
}

/* ----------------------------------------------------------------- *\
|  The main program
\* ----------------------------------------------------------------- */

int main(int argc, char **argv)
{
    FILE *ifp;
    FILE *ofp;
    char infile[FILENAME_MAX + 1];
    char outfile[FILENAME_MAX + 1];
    char *p, *opts;
    int i, inopt;

#if DEBUG_MALLOC
    malloc_debug(2);
#endif /* DEBUG_MALLOC */

    if (argc < 2)
        die("Usage: bibindex bib [-i field...]", "");

    if (((p = strrchr(argv[1], '.')) != (char *)NULL) &&
        (strcmp(p, ".bib") == 0)) {
        *p = '\0';                      /* remove any .bib extension */
    }

    (void)sprintf(infile, "%s.bib", argv[1]);
    (void)sprintf(outfile, "%s.bix", argv[1]);

    ifp = fopen(infile, "r");
    if (!ifp)
        die("Can't read", infile);

#if MSDOS
    ofp = fopen(outfile, "wb");
#else
    ofp = fopen(outfile, "w");
#endif
    if (!ofp)
        die("Can't write", outfile);

    InitTables();
    StandardAbbrevs();
    StandardBadWords();

    if ((argc > 2) && (!strcmp(argv[2], "-i"))) {
        for (i = 3; i < argc; i++)
            InitBlackHole(argv[i]);
    } else {
        opts = (char *)getenv("BIBINDEXFLAGS");
        if (opts != NULL) {
            p = opts;
            inopt = 0;

            while (*p != 0) {
                if (isspace(*p)) {
                    if (inopt) {
                        inopt = 0;
                        *p = 0;
                        if (strcmp(opts, "-i"))
                            InitBlackHole(opts);
                        opts = p + 1;
                    }
                } else {
                    inopt = 1;
                }
                p++;
            }

            if (inopt && strcmp(opts, "-i"))
                InitBlackHole(opts);
        }
    }

    IndexBibFile(ifp, ofp, argv[1]);

    FreeTables();
    fclose(ifp);
    fclose(ofp);

    exit(EXIT_SUCCESS);                 /* Argh! */
    return (0);			                /* keep compilers happy */
}

/* ================================================================= *\

   ref2bib -- A better r2bib; translates refer to bibtex

   This program was specifically developed for use with the
   computational geometry bibliographic database, and assumes the
   rules thereof.  The database may be obtained by anonymous ftp
   from cs.usask.ca in the file pub/geometry/geombib.tar.Z .

   Written by Jeff Erickson <jeff@ics.uci.edu>, 03 Jan 92
   Modified by Bill Jones <jones@cs.usask.ca>, 07 Feb 92

   This program is in the public domain.  You may use it or modify
   it to your heart's content, at your own risk.

   Usage: ref2bib [input]

   %Make% gcc -o ref2bib -g ref2bib.c

   -----------------------------------------------------------------

   VERSION HISTORY:

   0.3	03 Jan 92	<jge>	Initial version (# is meaningless)
   0.4	07 Feb 92	<wdj>	Folded in some (but not quite all) ideas
    from Robert Freimer and Peter Yamamoto.  Cleaned up the code a bit.
    Used to convert geom bib.
   0.4	22 Apr 92	<jge>	Cleaned up the code a little more.
    Changed back to 4-column indentation.
   0.5	22 May 92	<wdj>	Adopted another idea from Freimer, to
    distinguish incollection from inproceedings on conversion.
    Resulting changes folded into geom.bib.

\* ================================================================= */

#define MAJOR_VERSION 0
#define MINOR_VERSION 5

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAXLINE 4096
typedef char Line[MAXLINE], Word[16];

/* ----------------------------------------------------------------- *\
|  Math tokens, sorted by eqn format in ASCII order.  Some TeX tokens
|  (eg, "_" or "\sqrt") require an argument; these are flagged in the
|  third field of the MToken record.
|
|  This list is taken almost directly from the 10/15/91 version of
|  the computational geometry bibliography project database, with
|  some obvious additions like \pi and \Delta tacked on.  It would
|  be fairly easy to enter the whole list from eqnchar(7), but I
|  don't feel like typing that much.
\* ----------------------------------------------------------------- */

typedef struct
{
    Word eqn;	   /* eqn syntax */
    Word TeX;	   /* TeX syntax */
    char argument; /* takes an argument? */
} MathToken;

MathToken theTokens[] =
    {
        {"->", "\\rightarrow", 0},
        {"DELTA", "\\Delta", 0},
        {"OMEGA", "\\Omega", 0},
        {"PI", "\\Pi", 0},
        {"SIGMA", "\\Sigma", 0},
        {"THETA", "\\Theta", 0},
        {"delta", "\\delta", 0},
        {"from", "_", 1},
        {"inf", "\\infty", 0},
        {"lg", "\\lg", 0}, /* unrecognized by eqn */
        {"lim", "\\lim", 0},
        {"ln", "\\ln", 0},		  /* unrecognized by eqn */
        {"log", "\\log", 0},	  /* unrecognized by eqn */
        {"log*", "\\log^{*}", 0}, /* unrecognized by eqn */
        {"max", "\\max", 0},
        {"min", "\\min", 0},
        {"omega", "\\omega", 0},
        {"pi", "\\pi", 0},
        {"sigma", "\\sigma", 0},
        {"sqrt", "\\sqrt", 1},
        {"sub", "_", 1},
        {"sup", "^", 1},
        {"times", "\\times", 0},
        {"to", "^", 1},
        {"union", "\\bigcup", 0},
};

#define numTokens (sizeof(theTokens) / sizeof(MathToken))

/* --- Special token types --- */

#define NO_MATCH (-1)
#define SUB_EXPR (-2)
#define WHITE_SP (-3)

/* ----------------------------------------------------------------- *\
|  The bibliographic entry type (and related stuff)
\* ----------------------------------------------------------------- */

typedef enum /* Publication types */
{
    Unknown = 0,
    Article,
    Book,
    InProc,
    InColl,
    Report,
    Unpub
} PubType;

Word PTName[] = /* Output strings for the above types */
    {
        "misc",
        "article",
        "book",
        "inproceedings",
        "incollection",
        "techreport",
        "unpublished"};

typedef struct
{
    Line marginalia;
    PubType pubtype;
    Word citekey;
    Line author;
    Line edition;
    Line editor;
    Line title;
    Line booktitle;
    Line series;
    Line journal;
    Line publisher; /* same as "institution" for TRs    */
    Line type;
    Line address;
    Line volume;
    Line number;
    Line year;
    Line month;
    Line pages;
    Line keywords; /* These two fields are normally    */
    Line comments; /*    ignored by bibtex.            */
    Line label;	   /* trial mnemonic citekey           */
} BibEntry;

BibEntry be = {"", Unknown, "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""};

#define ZeroEntry(x) bzero((char *)(x), sizeof(BibEntry));

static int nline = 0;

/* ======================= UTILITY FUNCTIONS ======================= */

/* ----------------------------------------------------------------- *\
|  char *append(char *src, char *dst)
|
|  Append src to dst and return ptr to last character ('\0') copied.
\* ----------------------------------------------------------------- */
char *append(char *src, char *dst)
{
    while (*dst++ = *src++)
        ;
    return dst - 1;
}

/* ----------------------------------------------------------------- *\
|  void die(char *msg1, char *msg2) -- print an error message and die
\* ----------------------------------------------------------------- */
void die(char *msg1, char *msg2)
{
    fprintf(stderr, "Error (%d): %s %s", nline, msg1, msg2);
    exit(1);
}

/* ---------------------------------------------------------------- *\
|  void warn(char *msg1, char *msg2) -- print a warning message
\* ----------------------------------------------------------------- */
void warn(char *msg1, char *msg2)
{
    fprintf(stderr, "Warning (line %d): %s %s\n", nline, msg1, msg2);
}

/* =========================== MATH STUFF ========================== */

/* ----------------------------------------------------------------- *\
|  int LookUpToken(char *tok)
|
|  Look up the token in theTokens.  Return either the index into the
|  token list, or NO_MATCH if none found.  This really ought to use
|  bsearch(3), since that's what it does.
\* ----------------------------------------------------------------- */
int LookUpToken(char *tok)
{
    int hi, lo, cmp;

    lo = 0;
    hi = numTokens - 1;

    if ((strcmp(tok, theTokens[lo].eqn) >= 0) &&
        (strcmp(tok, theTokens[hi].eqn) <= 0))
    {
        while (lo <= hi)
        {
            cmp = strcmp(tok, theTokens[(hi + lo) / 2].eqn);

            if (cmp == 0)
                return (hi + lo) / 2;
            else if (cmp < 0)
                hi = (hi + lo) / 2 - 1;
            else
                lo = (hi + lo) / 2 + 1;
        }
    }
    return NO_MATCH;
}

/* ----------------------------------------------------------------- *\
|  char *GetEqnToken(char *src, char *dst, int *tindex)
|
|  Get the first eqn token out of src, copy it into dst, put its
|  index in the token table into tindex, and return a pointer to the
|  first character after the token.  Tokens are defined as either
|   (1) anything enclosed in (possibly nested) braces or
|   (2) any string of letters (* is considered a letter (log*))
|   (3) any string of digits (. is considered a digit (3.1416))
|   (4) any string of spaces (~ and ^ are considered spaces)
|   (5) any string of nonspaces
\* ----------------------------------------------------------------- */
char *GetEqnToken(char *src, char *dst, int *tindex)
{
    register char *tmp = dst;
    int braces = 0;

    if (*src == '{') /* sub-expression */
    {
        src++;
        braces++;

        while (braces)
        {
            if (!*src)
                die("Unbalanced braces in math expression!", "");
            else if (*src == '}')
                braces--;
            else if (*src == '{')
                braces++;

            *tmp++ = *src++;
        }
        tmp[-1] = 0; /* trash the close brace! */
        *tindex = SUB_EXPR;
    }
    else if (isalpha(*src))
    {
        while (isalpha(*src))
            *tmp++ = *src++;
        if (*src == '*') /* "log*" */
            *tmp++ = *src++;
        *tmp = 0;
        *tindex = LookUpToken(dst);
    }
    else if (isdigit(*src) || (*src == '.'))
    {
        while (isdigit(*src) || (*src == '.'))
            *tmp++ = *src++;
        *tindex = NO_MATCH;
    }
    else if (isspace(*src) || (*src == '~') || (*src == '^'))
    {
        while (isspace(*src) || (*src == '~') || (*src == '^'))
            src++;
        *tmp++ = ' ';
        *tindex = WHITE_SP;
    }
    else
    {
        /* swallows some eqnchar(7) constructions, or any single char */
        while ((*tmp++ = *src++) && *src && strchr("<-=>", *src))
            ;
        *tmp = 0;
        *tindex = LookUpToken(dst);
    }

    *tmp = 0;
    return src;
}

/* ----------------------------------------------------------------- *\
|  char *TeXifyMath(char *src, char *dst) -- change eqn into TeX
\* ----------------------------------------------------------------- */
char *TeXifyMath(char *src, char *dst)
{
    char token[MAXLINE];
    int tindex;
    char pending = 0; /* waiting for argument? */
    char braces = 0;  /* number of nested braces */

    while (*src)
    {
        src = GetEqnToken(src, token, &tindex);

        if (tindex == NO_MATCH) /* no match -- copy verbatim */
        {
            if (isalpha(dst[-1]) && isalpha(token[0]))
                *dst++ = ' ';
            dst = append(token, dst);
        }
        else if (tindex == SUB_EXPR) /* subexpression in {}'s */
        {
            if (dst[-1] != '{')
            {
                *dst++ = '{';
                dst = TeXifyMath(token, dst);
                *dst++ = '}';
            }
            else
                dst = TeXifyMath(token, dst);
        }
        else if (tindex == WHITE_SP) /* white space */
        {
            if (!pending)
            {
                while (braces)
                {
                    *dst++ = '}';
                    --braces;
                }
            }
            else
                pending = 0;
        }
        else /* translatable token */
        {
            dst = append(theTokens[tindex].TeX, dst);

            if (theTokens[tindex].argument)
            {
                *dst++ = '{';
                pending = 1;
                braces++;
            }
        }
    }

    while (braces--)
        *dst++ = '}';

    return dst;
}

/* ==================== GENERAL TeX TRANSLATION ==================== */

/* ----------------------------------------------------------------- *\
|  void TeXify(char *text) -- change troff into TeX
|
|  This routine has more than its share of kludges to get around
|  BibTeX's stupidity:
|   (1) BibTeX doesn't know that umlauts (\") aren't real quotes,
|       so umlauts are put inside braces.
|   (2) BibTeX doesn't know not to case-fold control sequences (eg,
|       "Erd\H os") so I have to add some more braces.
\* ----------------------------------------------------------------- */
void TeXify(char *text)
{
    char buffer[MAXLINE];
    char *bufptr;
    char *mathptr;

    strcpy(buffer, text);
    bufptr = buffer;

    /* Translate normal text characters */

    while (*bufptr)
    {
        if (*bufptr == '$') /* Is it math? */
        {
            ++bufptr;
            mathptr = bufptr;
            bufptr = strchr(bufptr, '$');

            if (bufptr == NULL)
                die("Stranded in math mode!\n\t", mathptr);

            *bufptr++ = 0; /* terminate mathptr string */

            *text++ = '$';
            text = TeXifyMath(mathptr, text);
            *text++ = '$';
        }
        else if (*bufptr == '\\') /* Is it a command? */
        {
            ++bufptr;

            if (*bufptr == '0' || *bufptr == ' ') /* tie (hard space) */
            {
                ++bufptr;
                *text++ = '~';
            }
            else if (*bufptr == '*') /* accent or em dash */
            {
                ++bufptr;
                switch (*bufptr)
                {
                case '-': /* em dash */
                    ++bufptr;
                    text = append("---", text);
                    break;

                case 'C': /* ha\v{c}ek */
                    ++bufptr;
                    text = append("{\\v ", text);
                    *text++ = *bufptr++;
                    *text++ = '}';
                    break;

                case ',': /* cedilla */
                    ++bufptr;
                    text = append("{\\c ", text);
                    *text++ = *bufptr++;
                    *text++ = '}';
                    break;

                case ':': /* umlaut */
                    ++bufptr;
                    text = append("{\\\"", text);
                    *text++ = *bufptr++;
                    *text++ = '}';
                    break;

                default: /* other accent */
                    *text++ = '{';
                    *text++ = '\\';
                    *text++ = *bufptr++;
                    if (isalpha(bufptr[-1]))
                        *text++ = ' ';
                    *text++ = *bufptr++;
                    *text++ = '}';
                }
            }
            else
                die("Unrecognized troff command!\n\t", bufptr - 1);
        }
        else if ((*bufptr == '#') || /* weird TeX characters */
                 (*bufptr == '$') ||
                 (*bufptr == '%') ||
                 (*bufptr == '&') ||
                 (*bufptr == '_'))
        {
            *text++ = '\\';
            *text++ = *bufptr++;
        }
        else /* Must be normal text */
        {
            *text++ = *bufptr++;
        }
    }

    *text = 0;
}

/* ============================ INPUT ============================== */

/* ----------------------------------------------------------------- *\
|  GetField(FILE *ifp, char *input) -- Get one field from the bib.
|
|  Assumes long lines have been wrapped to MAXLINE characters or less,
|    (such that fgets buffer is sufficient) and unwraps them.
|  Blank lines are considered fields.
|  Assumes no line (before wrapping) is longer than MAXLINE characters
\* ----------------------------------------------------------------- */
void GetField(FILE *ifp, char *input)
{
    char tmp;

    if (feof(ifp))
        return;

    fgets(input, MAXLINE, ifp);
    ++nline;

    tmp = getc(ifp); /* look at the next character */
    ungetc(tmp, ifp);
    while (!feof(ifp) && (tmp != '%') && (tmp != '\n')) /* folded line? */
    {
        input += strlen(input) - 1;
        *input++ = ' '; /* change newline to space */
        fgets(input, MAXLINE, ifp);
        tmp = getc(ifp); /* look at the next character */
        ungetc(tmp, ifp);
    }

    input += strlen(input) - 1;
    *input = 0; /* remove final newline */
}

/* ----------------------------------------------------------------- *\
|  void ForceCapitals(char *title) -- Surround all capital letters
|    with braces, with the following exceptions:
|    (1) Don't touch the first letter in the title
|    (2) Don't touch anything already in braces
|    (3) Math expressions are considered single capital letters
|
|  Capital letters in non-math control sequences ARE surrounded by
|  braces, unless they're already hidden, and this WILL cause an
|  error when TeX is run.  The routine that puts the control sequence
|  into the text in the first place is responsible for making sure
|  this never happens.
\* ----------------------------------------------------------------- */
void ForceCapitals(char *title)
{
    Line buffer;
    char *tmp = buffer;
    char math = 0, braces = 0;

    strcpy(buffer, title);

    while (*tmp)
    {
        if (isupper(*tmp) && (tmp > buffer) && (!math) && (!braces))
        {
            if (title[-1] == '}') /* prefer {VLSI} to {V}{LSI} */
                --title;
            else /* prefer {VLSI} to V{LSI} */
            {
                char *title0 = title;
                *title++ = '{';
                while (--title0 >= buffer && isupper(*title0))
                {
                    title0[1] = title0[0];
                    title0[0] = '{';
                }
            }
            while (*tmp && isalpha(*tmp)) /* prefer {Voronoi} to {V}oronoi */
                *title++ = *tmp++;
            *title++ = '}';
        }
        else if (*tmp == '$')
        {
            if (!math)
            {
                math = 1;
                *title++ = '{';
                *title++ = *tmp++;
            }
            else
            {
                *title++ = *tmp++;
                *title++ = '}';
                math = 0;
            }
        }
        else
        {
            if (*tmp == '{')
                braces++;
            if (*tmp == '}')
                braces--;
            *title++ = *tmp++;
        }
    }
    *title = 0;
}

/* ----------------------------------------------------------------- *\
|  void BraceCommas(char *name)
|
|  Put braces around any word in name that contains a comma.
\* ----------------------------------------------------------------- */
void BraceCommas(char *name)
{
    Line buffer;
    char *tmp = buffer;
    char *word;

    strcpy(buffer, name);

    while (tmp && *tmp)
    {
        word = tmp;
        tmp = strchr(tmp, ' '); /* get a word */
        if (tmp)
            *tmp++ = 0;

        if (strchr(word, ',') != NULL) /* if it has a comma */
        {
            *name++ = '{'; /* add braces */
            name = append(word, name);
            *name++ = '}';
        }
        else
            name = append(word, name);

        *name++ = ' ';
    }

    *(--name) = 0; /* trash the last space */
}

/* ----------------------------------------------------------------- *\
|  void GetEntry(FILE *ifp, FILE *ofp) -- get a bib entry.
|
|  Output comments immediately.
|  Run everything else thru TeXify()
|  Force capitals in the title and type fields
|  Put braces around words with commas in the author and editor
|    fields, so names like "Joe Blow,~Jr." work correctly.
|  Coalesce multiple author, editor, comment, and keyword fields.
|  Split dates into month and year.
|  Determine the publication type, as given.
|  Generate a citation key based on the %Z field.
\* ----------------------------------------------------------------- */
void GetEntry(FILE *ifp, FILE *ofp)
{
    static char edition[] = " edition, "; /* precedes publisher in */
                                          /* some %I fields */
    char tmpstr[MAXLINE];
    Line input;
    char *tmp;
    char inEntry = 0; /* 1 if real field has been read */

    ZeroEntry(&be);

    while (!feof(ifp))
    {
        GetField(ifp, input);

        if (input[0] == 0) /* blank line */
        {
            if (inEntry == 1)
            {
                if (!be.title[0] && !be.author[0])
                {
                    if (!be.marginalia[0])
                        warn(be.citekey, "has no author or title --- skipping.");
                    return;
                }
                else if (be.pubtype == Unknown)
                { /* try to intuit real type */
                    if (be.journal[0])
                        be.pubtype = Article;
                    else if (be.type[0])
                        be.pubtype = Report;
                    else if (be.booktitle[0])
                        be.pubtype = InColl;
                    else if (be.publisher[0])
                        be.pubtype = Book;
                }
                if (be.pubtype == InColl) /* maybe it's a proceedings */
                    if ((tmp = strstr(be.booktitle, "Proc.")) ||
                        (tmp = strstr(be.booktitle, "Conf.")) ||
                        (tmp = strstr(be.booktitle, "Sympos.")) ||
                        (tmp = strstr(be.booktitle, "Colloq.")) ||
                        (tmp = strstr(be.booktitle, "Symp.")) ||
                        (tmp = strstr(be.booktitle, "Coll.")) ||
                        (tmp = strstr(be.booktitle, "Proceedings")) ||
                        (tmp = strstr(be.booktitle, "Conference")) ||
                        (tmp = strstr(be.booktitle, "Symposium")) ||
                        (tmp = strstr(be.booktitle, "Colloqium")) ||
                        (tmp = strstr(be.booktitle, "Symposia")) ||
                        (tmp = strstr(be.booktitle, "Colloquia")))
                        be.pubtype = InProc;
                return;
            }
            else
            {
                fprintf(ofp, "\n");
                continue;
            }
        }

        if (input[0] != '%')
            die("%% expected at beginning of line!\n\t", input);
        inEntry = 1;
        if (input[1] != '#') /* Don't TeXify comments!! */
            TeXify(input + 3);

        switch (input[1])
        {
        case 'A':
            BraceCommas(input + 3);
            if (be.author[0]) /* allow for multiple authors */
                strcat(be.author, " and ");
            strcat(be.author, input + 3);
            break;

        case 'B':
            strcpy(be.booktitle, input + 3);
            break;

        case 'C':
            strcpy(be.address, input + 3);
            break;

        case 'D':
            if ((tmp = strrchr(input + 3, ' ')))
            {
                *tmp++ = '\0';
                strcpy(be.year, tmp);
#if 1
                strcpy(be.month, input + 3);
#else
                strncpy(be.month, input + 3, 3);
                be.month[3] = '\0';
                if (isupper(be.month[0]))
                    be.month[0] = tolower(be.month[0]);
#endif
            }
            else
                strcpy(be.year, input + 3);
            break;

        case 'E':
            BraceCommas(input + 3);
            if (be.editor[0]) /* allow for multiple editors */
                strcat(be.editor, " and ");
            strcat(be.editor, input + 3);
            break;

        case 'I':
            if ((tmp = strstr(input, edition)))
            {
                *tmp = '\0';
                strcpy(be.edition, input + 3);
                strcpy(be.publisher, tmp + strlen(edition));
            }
            else
                strcpy(be.publisher, input + 3);
            break;

        case 'J':
            strcpy(be.journal, input + 3);
            break;

        case 'K': /* allow for multiple keywords */
            if (be.keywords[0])
                strcat(be.keywords, ", ");
            strcat(be.keywords, input + 3);
            break;

        case 'L':
            strcpy(be.label, input + 3);
            break;

        case 'N':
            strcpy(be.number, input + 3);
            break;

        case 'O':
            if (be.comments[0]) /* allow for multiple comments */
                strcat(be.comments, "; ");
            strcat(be.comments, input + 3);
            break;

        case 'P':
            if (tmp = strchr(input + 3, '-'))
            {
                tmp[0] = 0;
                sprintf(be.pages, "%s--%s", input + 3, tmp + 1);
            }
            else
                strcpy(be.pages, input + 3);
            break;

        case 'R':
            if ((tmp = strstr(input, "Report ")))
            {
                tmp += 6;
                *tmp++ = '\0';
                strcpy(be.number, tmp);
            }
            ForceCapitals(input + 3);
            strcpy(be.type, input + 3);
            break;

        case 'S':
            strcpy(be.series, input + 3);
            break;

        case 'T':
            if (be.pubtype != Book)
                ForceCapitals(input + 3);
            strcpy(be.title, input + 3);
            break;

        case 'V':
            strcpy(be.volume, input + 3);
            break;

        case 'X':
            switch (input[3])
            {
            case 'A':
                be.pubtype = Article;
                break;
            case 'B':
                be.pubtype = Book;
                break;
            case 'I':
                be.pubtype = InColl;
                break;
            case 'R':
                be.pubtype = Report;
                break;
            default:
                be.pubtype = Unknown;
                break;
            }
            break;

        case 'Z':
            sprintf(be.citekey, "geom-%s", input + 3);
            break;

        default:
            warn("unknown entry, printing as comment:\n\t", input);
            input[1] = '#';
        /* fall through */
        case '#':
            sprintf(tmpstr, "%s%% %s\n", be.marginalia, input + 3);
            strcpy(be.marginalia, tmpstr);
            break;
        }
    }
}

/* ============================= OUTPUT ============================ */

/* ----------------------------------------------------------------- *\
|  void PutField(FILE *ofp, char *key, char *value)
|
|  Output a single bib field, if there's anything to output
\* ----------------------------------------------------------------- */
void PutField(FILE *ofp, char *key, char *value)
{
    if (*value)
        fprintf(ofp, ", %s =\t\"%s\"\n", key, value);
}

/* ----------------------------------------------------------------- *\
|  void PutEntry(FILE *ofp) -- output the bib entry.
|
|  Warn the user and don't output anything if both the author and
|    title are missing.
|  Warn the user if the title is missing.
|  Warn the user if both the author and the editor are missing.
|  Warn the user if the year is missing, but output "19??".  This
|    may not be the best solution, but it reduces the number of
|    BibTeX warnings.
|  Warn the user about other mistakes, like books without publishers,
|    articles without journals, conference papers without
|    proceedings.  Output "??" to avoid BibTeX warnings.
|  If the reference is a paper in a proceedings which appears as a
|    journal issue, it's formatted as an article, with the conference
|    title placed in the booktitle field (where current style files
|    ignore it).  This seems to only happen with SIGGRAPH papers.
|    An alternate way to handle this (#if'ed out below) is to pack both
|    conference and journal names into the journal field.
|  If the reference is a report with no institution, it is formatted
|    as an unpublished manuscript.  Most of the time, this is even
|    right!  The few cases that aren't were labelled ## fixme in
|    the database.
\* ----------------------------------------------------------------- */
void PutEntry(FILE *ofp)
{
    char tmpstr[MAXLINE];

    fprintf(ofp, "\n%s", be.marginalia);
    if (!be.title[0] && !be.author[0])
        return;

    /* ---- Handle SIGGRAPH articles ---- */

    /* actually I think that when bibtex and bibliographic reality
       conflict it is bibtex which should give way -- [wdj] */

    if ((be.pubtype == InProc) && be.journal[0])
    {
        be.pubtype = Article;
#if stopped_worrying_and_learned_to_love_bibtex
        sprintf(tmpstr, "%s, %s", be.booktitle, be.journal);
        strcpy(be.journal, tmpstr);
        be.booktitle[0] = 0;
#endif
    }

    /* ---- Handle manuscripts ---- */

    if ((be.pubtype == Report) && !be.publisher[0])
    {
        be.pubtype = Unpub;
        if (!be.type[0])
            strcpy(be.type, "Manuscript");
    }

    /* ---- Output the reference ---- */

    fprintf(ofp, "@%s{%s\n", PTName[be.pubtype], be.citekey);

    if (!be.author[0] && !be.editor[0])
        warn(be.citekey, "has no author or editor.");

    PutField(ofp, "author", be.author);

    if (be.title[0])
        PutField(ofp, "title", be.title);
    else
        warn(be.citekey, "has no title.");

    PutField(ofp, "editor", be.editor);

    if (be.booktitle[0])
        PutField(ofp, "booktitle", be.booktitle);
    else if (be.pubtype == InProc)
    {
        warn(be.citekey, "has no booktitle.  Using \"??\".");
        PutField(ofp, "booktitle", "??");
    }

    PutField(ofp, "series", be.series);

    if (be.journal[0])
        PutField(ofp, "journal", be.journal);
    else if (be.pubtype == Article)
    {
        warn(be.citekey, "has no journal.  Using \"??\".");
        PutField(ofp, "journal", "??");
    }

    if (be.pubtype == Unpub)
        PutField(ofp, "note", be.type);
    else
        PutField(ofp, "type", be.type);

    PutField(ofp, "volume", be.volume);

    if (be.number[0])
    {
        if (be.pubtype != Article)
            warn(be.citekey, "has a spurious number.");
        PutField(ofp, "number", be.number);
    }

    PutField(ofp, "edition", be.edition);

    if (be.publisher[0])
    {
        if (be.pubtype == Report)
            PutField(ofp, "institution", be.publisher);
        else
            PutField(ofp, "publisher", be.publisher);
    }
    else if (be.pubtype == Book)
    {
        warn(be.citekey, "has no publisher.  Using \"??\".");
        PutField(ofp, "publisher", "??");
    }

    PutField(ofp, "address", be.address);
    PutField(ofp, "month", be.month);

    if (be.year[0])
        PutField(ofp, "year", be.year);
    else
    {
        warn(be.citekey, "has no year.  Using \"19??\".");
        PutField(ofp, "year", "19??");
    }

    PutField(ofp, "pages", be.pages);
    PutField(ofp, "keywords", be.keywords);
    PutField(ofp, "comments", be.comments);
    PutField(ofp, "label", be.label);

    fprintf(ofp, "}\n");
}

/* ----------------------------------------------------------------- *\
|  The main program -- do everything!!
\* ----------------------------------------------------------------- */
void main(int argc, char **argv)
{
    time_t now;
    FILE *ifp = stdin;
    FILE *ofp = stdout;
    char nowstr[64];

    if (argc > 1)
    {
        ifp = fopen(argv[1], "r");
        if (!ifp)
            die("Can't read", argv[1]);

        if (argc > 2) /* should accept multiple inputs ... */
            die("Usage: ref2bib [input]", "");
    }

    now = time(0);

    strftime(nowstr, 64, "%Y %b %d %R\n", localtime(&now));
    /* strcpy(nowstr, ctime(&now)); */

    fprintf(ofp, "%% Created by %s %d.%d from %s, %s",
            argv[0], MAJOR_VERSION, MINOR_VERSION,
            argc > 1 ? argv[1] : "standard input", nowstr);

    while (!feof(ifp))
    {
        GetEntry(ifp, ofp);
        PutEntry(ofp);
    }

    fclose(ifp);
    fclose(ofp);
    exit(0);
}

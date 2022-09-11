#=======================================================================
# Makefile for bibindex and biblook.
#
# These programs are written in ISO/ANSI Standard C.  They must be
# compiled with a Standard C compiler, or with a C++ compiler.
#
# Current target list:
#	all 			make bibindex and biblook
#	bibindex 		make indexing program
#	bibindex.tar		UNIX tar archive file for bibindex
#				distribution
#	bibindex.tar-lst	listing of UNIX tar archive
#	bibindex.tar.z		compressed UNIX tar archive file for bibindex
#	bibindex.txt 		ascii text file from UNIX man pages
#	bibindex.uue		uuencoded bibindex.tar.z archive
#	bibindex.zip		InfoZip archive file for bibindex
#				distribution
#	bibindex.zip-lst	listing of InfoZip archive file
#	bibindex.zoo		zoo archive file for bibindex distribution
#	bibindex.zoo-lst	listing of zoo archive file
#	biblook.txt 		ascii text file from UNIX man pages
#	biblook 		make lookup program
#	clean 			remove all recreatable files, except
#				executables
#	clobber 		remove all recreatable files
#	install 		install executables and manual pages
#	install-ftp		install distribution in anonymous ftp tree
#	uninstall 		uninstall executables and manual pages
#
# Preprocessor symbol definitions that you might want to change:
#
# CC		see the large selection below of recommended settings.
#
# BINDIR	where the installed programs go
#
# CATDIR	where the formatted manual pages go
#
#
# DEF_H_FILES	On non-UNIX systems, if your system has the include file
#		netinet/in.h, define HAVE_NETINET_IN_H (this symbol is
#		automatically defined on UNIX), since all (we hope)
#		such systems have that include file.  When
#		HAVE_NETINET_IN_H is defined and non-zero, the .bix
#		file is read and written in network byte order,
#		allowing it to be shared between big-endian and
#		little-endian architectures on the same network.  If
#		you don't require this feature, then you can define
#		HAVE_NETINET_IN_H to 0 at compile time.  Since
#		heterogeneous networks are common at many sites, the
#		default is to use network byte order.
#
#		If your site has malloc.h (most UNIX systems do), add
#		-DHAVE_MALLOC_H.
#
#		If your site has stdlib.h, but not yet a Standard C compiler,
#		add -DHAVE_STDLIB_H.
#
# DEST		root directory of local binaries and man pages
#
# MANDIR	where the unformatted manual pages go
#
# MORE		use pg, more, or less for output paging (NB: see security
# 		remarks there)
#
# Numerous other preprocessor symbols are defined below, but are
# unlikely to need modifications.
#
# [13-Sep-1993]
#=======================================================================

DEST		= /usr/local

BINDIR		= $(DEST)/bin

CATDIR		= $(DEST)/man/cat1

# Compilation with a C++ compiler is preferable.  SunOS 4.1 CC cannot be
# used, however, because of its erroneous function prototypes in stdlib.h
# which use char* instead of void* in many places.  There is no such
# problem with Sun Solaris 2.1 and 2.2 CC, which works fine.
CC		= CC			## UNIX C++ compilers (HP, SGI, Sun Solaris 2.x)
CC		= CC -I/usr/CC/incl	## C++ on SunOS 4.1.1
CC		= acc -Dsun		## SunOS Standard C compiler
CC		= c89 -D_POSIX_SOURCE	## HP 9000/850 HP-UX A.08.00 D
CC		= c89 -D_POSIX_SOURCE	## IBM RS/6000 Standard C
CC		= xlC			## IBM RS/6000 C++
CC		= cc			## many UNIX systems
CC		= cc $(GCCFLAGS)	## NeXT
CC		= lcc -A -A -n		## Princeton/AT&T Standard C compiler
CC		= clcc			## CenterLine C
CC		= gcc $(GCCFLAGS)	## GNU C
CC		= gcc $(GCCFLAGS)	## UNIX systems with GNU C++
CC		= gcc $(GCCFLAGS) -Dtemplate=Template ## NeXT systems with GNU C++
CC		= cc -ObjC		## NeXT systems with Objective C
# Pick your compiler by copying its line here.
CC		= gcc -Wall $(GCCFLAGS)	## GNU C

CFLAGS		= $(DEFINES) $(OPT)	## most cases

GCCFLAGS	= -Wall -Wshadow -Wcast-qual -Wpointer-arith \
		  -Wwrite-strings

COL		= col -b

COMPRESS	= compress

CP		= /bin/cp

# Added DEF_READLINE to DEFINES
# (Modified by Rafael Laboissiere <rafael@laboissiere.net>)
DEFINES		= $(DEF_H_FILES) $(DEF_MAXRESULTS) $(DEF_MORE) $(DEF_READLINE)

# Pick one of these; see the comments above.

DEF_H_FILES	= -DHAVE_MALLOC_H -DHAVE_STDLIB_H -DHAVE_NETINET_IN_H=0
DEF_H_FILES	= -DHAVE_MALLOC_H -DHAVE_NETINET_IN_H=0
DEF_H_FILES	= -DHAVE_MALLOC_H

# Maximum number of bibliographic entries that can be displayed
# A remote lookup service may wish to reduce this limit to prevent
# fetching of large chunks of a bibliography data base.
DEF_MAXRESULTS	= -DMAXRESULTS=500
DEF_MAXRESULTS	=		## use built-in defaults

# Use less if possible, and otherwise more, or some other pager.
# Set MOREPATH to the correct absolute path, and MORE to the
# path-less file name.
# NB: Security note: if you set up a service like the "telnet biblio"
# facility on siggraph.org, make sure that you take precautions to
# ensure that the shell escape facility of these pagers cannot give
# a remote user a separate login session!  You can for example use
# the restricted shell (/usr/lib/rsh), and launch biblook from the
# .profile file.
DEF_MORE	=		## use built-in defaults
DEF_MORE	= -DMOREPATH=\"/usr/bin/pg\" -DMORE=\"pg\"
DEF_MORE	= -DMOREPATH=\"/usr/ucb/more\" -DMORE=\"more\"
DEF_MORE	= -DMOREPATH=\"/usr/local/bin/less\" -DMORE=\"less\"
DEF_MORE	= -DMOREPATH=\"/usr/sww/bin/less\" -DMORE=\"less\"
DEF_MORE        = -DMOREPATH=\"/usr/bin/pager\" -DMORE=\"pager\"

# Use GNU Readline library for command history & editing.  If
# USE_READLINE is unset, fallback to Sariel Har-Peled's implementaton.
# (Modified by Rafael Laboissiere <rafael@laboissiere.net>)
ifdef USE_READLINE
DEF_READLINE 	= -DUSE_READLINE
endif

# This setting is suitable for ftp.math.utah.edu:
FTPDIR		= /usr/spool/ftp/pub/tex/bib
FTPFILES	= bibindex$(VERSION).tar.z bibindex$(VERSION).zip \
		bibindex$(VERSION).zoo \
		bibindex$(VERSION).tar-lst bibindex$(VERSION).zip-lst \
		bibindex$(VERSION).zoo-lst

LDFLAGS		=

# Use /usr/lib/debug/malloc.o on Sun systems for malloc debugging
# with acc, gcc, or CC
LIBS		= /usr/lib/debug/malloc.o
# (Modified by Rafael Laboissiere <rafael@laboissiere.net>)
ifdef USE_READLINE
LIBS		= -lreadline -lncurses
else
LIBS 		=
endif

MANDIR		= $(DEST)/man/man1

MANEXT		= 1

NROFF		= nroff -man

# Define DEBUG_MALLOC on Sun systems for debugging memory allocation
OPT		= -g -DDEBUG_MALLOC
OPT		= -g

RM		= /bin/rm -f

SHELL		= /bin/sh

# If your system lacks strip, use echo instead
STRIP		= echo
STRIP		= strip

TAR		= tar

TARFILES	= README COPYING README.Readline PROBLEMS \
                Makefile mingw.mk bibindex.c bibindex.man \
		bibindex.txt biblook.c biblook.h biblook.man biblook.txt

UNZIP		= unzip

UUENCODE	= uuencode

VERSION		= -2-10

ZIP		= zip

ZOO		= zoo

#=======================================================================

all:	bibindex biblook bibindex.txt biblook.txt

bibindex:	bibindex.o
	$(CC) $(CFLAGS) -o bibindex bibindex.o $(LDFLAGS) $(LIBS)

bibindex$(VERSION).tar:	$(TARFILES)
	$(TAR) cf $@ $(TARFILES)

bibindex$(VERSION).tar-lst:	bibindex$(VERSION).tar
	$(TAR) tvf $? >$@

bibindex$(VERSION).tar.z:	bibindex$(VERSION).tar
	$(COMPRESS) <$? >$@

bibindex.txt:	bibindex.man
	$(NROFF) $? | $(COL) >$@

bibindex$(VERSION).uue:	bibindex$(VERSION).tar.z
	$(UUENCODE) $? $? >$@

bibindex$(VERSION).zip:	$(TARFILES)
	-$(RM) $@
	$(ZIP) $@ $(TARFILES)

bibindex$(VERSION).zip-lst:	bibindex$(VERSION).zip
	$(UNZIP) -v $? >$@

bibindex$(VERSION).zoo:	$(TARFILES)
	-$(RM) $@
	$(ZOO) a $@ $(TARFILES)

bibindex$(VERSION).zoo-lst:	bibindex$(VERSION).zoo
	$(ZOO) v $? >$@

biblook:	biblook.o
	$(CC) $(CFLAGS) -o biblook biblook.o $(LDFLAGS) $(LIBS)

biblook.txt:	biblook.man
	$(NROFF) $? | $(COL) >$@

bibindex.o biblook.o: biblook.h

clean mostlyclean:
	-$(RM) \#*
	-$(RM) *~
	-$(RM) core
	-$(RM) *.i
	-$(RM) *.o
	-$(RM) bibindex$(VERSION).tar bibindex$(VERSION).tar.z
	-$(RM) bibindex$(VERSION).tar-lst
	-$(RM) bibindex$(VERSION).uue
	-$(RM) bibindex$(VERSION).zip bibindex$(VERSION).zip-lst
	-$(RM) bibindex$(VERSION).zoo bibindex$(VERSION).zoo-lst

clobber distclean realclean reallyclean:	clean
	-$(RM) biblook bibindex
	-$(RM) biblook.txt bibindex.txt
	-$(RM) TAGS

install:	bibindex biblook
	-$(CP) bibindex $(BINDIR)
	-$(STRIP) $(BINDIR)/bibindex
	-chmod 775 $(BINDIR)/bibindex
	-$(CP) biblook $(BINDIR)
	-$(STRIP) $(BINDIR)/biblook
	-chmod 775 $(BINDIR)/biblook
	-$(CP) bibindex.man $(MANDIR)/bibindex.$(MANEXT)
	-chmod 774 $(MANDIR)/bibindex.$(MANEXT)
	-$(CP) biblook.man $(MANDIR)/biblook.$(MANEXT)
	-chmod 774 $(MANDIR)/biblook.$(MANEXT)

install-ftp:	$(FTPFILES)
	-for f in $? ; \
	do \
		$(CP) $$f $(FTPDIR) ; \
		chmod 664 $(FTPDIR)/$$f ; \
	done

uninstall:
	-$(RM) $(BINDIR)/bibindex
	-$(RM) $(BINDIR)/biblook
	-$(RM) $(MANDIR)/bibindex.$(MANEXT)
	-$(RM) $(MANDIR)/biblook.$(MANEXT)
	-$(RM) $(CATDIR)/bibindex.$(MANEXT)
	-$(RM) $(CATDIR)/biblook.$(MANEXT)

uninstall-ftp:
	-for f in $(FTPFILES) ; \
	do \
		$(RM) $(FTPDIR)/$$f ; \
	done

#=======================================================================

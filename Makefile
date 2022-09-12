#===============================================================================
# Makefile for bibindex and biblook with gcc and POSIX environment. The
# original Makefile has been renamed to Makefile_original
# 2022-09-11, Tobias Schoch <tobias.schoch@gmail.com>
#
# Target list:
#	all 				make bibindex and biblook
#	bibindex 			make indexing program
#	bibindex.txt 		ascii text file from UNIX man pages
#	biblook.txt 		ascii text file from UNIX man pages
#	biblook 			make lookup program
#	clean 				remove all recreatable files, except executables
#	clobber 			remove all recreatable files
#	install 			install executables and manual pages
#	uninstall 			uninstall executables and manual pages

#===============================================================================
# Set application-specific flags

# Maximum number of bibliographic entries that can be displayed; we
# do not use it (otherwise use e.g., F_MAX_RES = -DMAXRESULTS=500)
F_MAX_RES 	=

# We use less and thus overrule MOREPATH and MORE (fallback is more; see
# biblook.h
F_MORE		= -DMOREPATH=\"/usr/bin/less\" -DMORE=\"less\"

# We use GNU readline (otherwise leave F_READLINE = empty, i.e., F_READLINE =
# and do the same with the linker flags LIBS and LDFLAGS)
F_READLINE  = -DUSE_READLINE
LIBS		= -lreadline -lncurses
LDFLAGS		= -L/usr/lib/ -I/usr/include/readline

# We use color output
F_COLOR		= -DWITH_COLOR

# We have malloc (see biblook.h)
F_HEADER	= -DHAVE_MALLOC_H

# All flags
TOOLFLAGS	= $(F_MAX_RES) $(F_MORE) $(F_READLINE) $(F_COLOR) $(F_HEADER)

#===============================================================================

# Root directory of local binaries and man pages
DEST		= /usr/local

# Where the installed programs go
BINDIR		= $(DEST)/bin

# Where the formatted manual pages go
CATDIR		= $(DEST)/man/cat1

# Where the unformatted manual pages go
MANDIR		= $(DEST)/man/man1
MANEXT		= 1

# Utilities
NROFF		= nroff -man
COL			= col -b
RM			= /bin/rm -f
STRIP		= strip
CP			= /usr/bin/cp

# Compilier setting
CC			= gcc
CFLAGS		= -Wall -Wshadow -Wcast-qual -Wpointer-arith -Wwrite-strings

#===============================================================================

all: bibindex biblook bibindex.txt biblook.txt

bibindex: bibindex.o
	$(CC) bibindex.o -o bibindex

bibindex.txt: bibindex.man
	$(NROFF) $? | $(COL) >$@

biblook: biblook.o
	$(CC) biblook.o $(LDFLAGS) $(LIBS) -o biblook

%.o : %.c
	$(CC) $(CFLAGS) $(TOOLFLAGS) -c $< -o $@

biblook.txt: biblook.man
	$(NROFF) $? | $(COL) >$@

clean mostlyclean:
	-$(RM) \#*
	-$(RM) *~
	-$(RM) core
	-$(RM) *.i
	-$(RM) *.o

clobber distclean realclean reallyclean: clean
	-$(RM) biblook bibindex
	-$(RM) biblook.txt bibindex.txt

install: bibindex biblook
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

uninstall:
	-$(RM) $(BINDIR)/bibindex
	-$(RM) $(BINDIR)/biblook
	-$(RM) $(MANDIR)/bibindex.$(MANEXT)
	-$(RM) $(MANDIR)/biblook.$(MANEXT)
	-$(RM) $(CATDIR)/bibindex.$(MANEXT)
	-$(RM) $(CATDIR)/biblook.$(MANEXT)

#===============================================================================

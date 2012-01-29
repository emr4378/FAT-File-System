#
# Created by gmakemake (Sparc Jul 27 2005) on Fri Jul  1 22:59:56 2011
#

#
# Definitions
#

.SUFFIXES:
.SUFFIXES:	.a .o .c .C .cpp
.c.o:
		$(COMPILE.c) $<
.C.o:
		$(COMPILE.cc) $<
.cpp.o:
		$(COMPILE.cc) $<
.c.a:
		$(COMPILE.c) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.C.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%
.cpp.a:
		$(COMPILE.cc) -o $% $<
		$(AR) $(ARFLAGS) $@ $%
		$(RM) $%

CC =		gcc
CXX =		g++

RM = rm -f
AR = ar
LINK.c = $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)
LINK.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS)
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -c
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c

########## Default flags (redefine these with a header.mak file if desired)
CXXFLAGS =	-ggdb
CFLAGS =	-ggdb
CLIBFLAGS =	-lm
CCLIBFLAGS =	
########## End of default flags


CPP_FILES =	 FileSys.cpp Shell.cpp main.cpp
C_FILES =	
H_FILES =	 FileSys.h Shell.h
SOURCEFILES =	$(H_FILES) $(CPP_FILES) $(C_FILES)
.PRECIOUS:	$(SOURCEFILES)
OBJFILES =	 FileSys.o Shell.o

#
# Main targets
#

all:	 main 

main:	main.o $(OBJFILES)
	$(CXX) $(CXXFLAGS) -o os1shell main.o $(OBJFILES) $(CCLIBFLAGS)

#
# Dependencies
#

FileSys.o:	 FileSys.h
Shell.o:	 FileSys.h Shell.h
main.o:	 FileSys.h Shell.h

#
# Housekeeping
#

Archive:	archive.tgz

archive.tgz:	$(SOURCEFILES) Makefile
	tar cf - $(SOURCEFILES) Makefile | gzip > archive.tgz

clean:
	-/bin/rm -r $(OBJFILES) main.o core 2> /dev/null

realclean:        clean
	/bin/rm -rf  os1shell 

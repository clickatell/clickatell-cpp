#
# This Makefile creates a static library file:  lib/libclickatell_sms.a
# The static library can me linked into your application.
#
SHELL = /bin/sh
RANLIB = ranlib

includedir = ./

CPP=g++
LIBS=-lrt -lresolv -lnsl -lm -L/usr/lib64
CFLAGS=-D_REENTRANT=1 -D_XOPEN_SOURCE=600 -D_BSD_SOURCE -D_FILE_OFFSET_BITS=64 -Wall -static -ggdb -O2 -I. -I$(includedir)
LDFLAGS= -rdynamic

MKDEPEND=$(CPP) $(CFLAGS) -MM

progsrcs = clickatell_sms.cpp clickatell_debug.cpp clickatell_string.cpp
progobjs = $(progsrcs:.cpp=.o)
progs = $(progsrcs:.cpp=)

staticlib = lib/libclickatell_sms.a

libsrcs = $(wildcard *.cpp)
libobjs = $(libsrcs:.cpp=.o)
libsobj = $(libsrcs:.cpp=.so)

cleanfiles = $(progobjs) $(progs) $(staticlib)

srcdirs = .

.SUFFIXES: .cpp .o .so

.cpp.o:
	$(CPP) $(CFLAGS) -o $@ -c $<

.cpp.so:
	$(CPP) $(CFLAGS) -fPIC -o $@ -c $<

all: $(staticlib) $(libsobj)

clean:
	rm -f $(cleanfiles)

# this archives the object files into our library
$(staticlib): $(libobjs)
	$(AR) rc $(staticlib) $(libobjs)
	$(RANLIB) $(staticlib)

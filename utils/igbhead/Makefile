####################################################
#
# Makefile for igbutils
#
# All rights , etc., blah, blah, blah  Edward Vigmond
#
####################################################

OBJS = igbhead.o \
	IGBheader.o \
	cmdline.o \
	getopt.o \
	getopt1.o

all:  igbhead
	
igbhead : $(OBJS)
	g++ $(CFLAGS) -I.. -g -static $(LDFLAGS) -o igbhead $(OBJS) -lz

install: igbhead
	$(INSTALL) igbhead $(bindir)

%.o : %.cc
	$(CXX) -c -g $(CPPFLAGS) -I../src $(CFLAGS) $(INCLUDES) -o $@ $<

%.o : %.c
	$(CC) -c -g $(CPPFLAGS)  $(CFLAGS) $(INCLUDES) -o $@  $<

cmdline.c: igbhead.ggo
	gengetopt -u < igbhead.ggo

clean:
	rm *.o 

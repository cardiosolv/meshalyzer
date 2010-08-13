HOSTMACHINE := $(shell uname)

FLTK_INC      := $(shell fltk-config --use-gl --cxxflags)
FLTK_LD_FLAGS := $(shell fltk-config --use-images --use-gl --ldflags)
COMMON_LIBS    = -lpng -lpthread -lm -lz 
COMMON_INC     = -I. -O0 -g -DOBJ_CLASS -D_REENTRANT -MMD -DNOMINMAX 

ifeq ($(HOSTMACHINE),Darwin)
LIBS =  $(FLTK_LD_FLAGS) $(COMMON_LIBS)
CFLAGS = -I/usr/X11R6/include $(FLTK_INC) $(COMMON_INC)
else
LIBS   = $(FLTK_LD_FLAGS) $(COMMON_LIBS)
CFLAGS = $(FLTK_INC) $(COMMON_INC)
endif

CPPFLAGS = $(CFLAGS)

FLTK_SOURCES = $(wildcard *.fl)
OBJS = $(FLTK_SOURCES:.fl=.o)\
	$(patsubst %.cc,%.o,$(wildcard *.cc))\
	$(patsubst %.c,%.o,$(wildcard *.c))\
	$(patsubst %.C,%.o,$(wildcard *.C))

meshalyzer: $(FLTK_SOURCES:.fl=.cc) $(OBJS)
	$(CXX) $(CFLAGS) -o meshalyzer $(sort $(OBJS)) $(LIBS)
ifeq ($(HOSTMACHINE),Darwin)
	fltk-config --post meshalyzer
endif

clean:
	rm -rf $(FLTK_SOURCES:.fl=.h) $(FLTK_SOURCES:.fl=.cc) *.o *.d meshalyzer

utils:
	(cd utils; make )

%.h %.cc: %.fl
	fluid -c $<

-include $(OBJS:.o=.d)

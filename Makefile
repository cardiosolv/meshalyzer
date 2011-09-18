HOSTMACHINE := $(shell uname)

HDF5API_ROOT  := ./hdf5api

COMMON_INC    := -I. -O0 -g -DOBJ_CLASS -D_REENTRANT -MMD -DNOMINMAX 

ifdef HDF5
LIB_CH5       := (HDF5_ROOT)/lib/libch5.a
LIB_HDF5      := -lch5 -lhdf5 -lhdf5_hl 
COMMON_INC    += -DUSE_HDF5
else
LIB_CH5       := 
LIB_HDF5      := 
endif

FLTK_INC      := $(shell fltk-config --use-gl --cxxflags)
FLTK_LD_FLAGS := $(shell fltk-config --use-images --use-gl --ldflags)
COMMON_LIBS    = -lpng -lpthread -lm -lz $(LIB_HDF5) 

LIBS     = -L$(HDF5API_ROOT)/lib $(FLTK_LD_FLAGS) $(COMMON_LIBS)
LDFLAGS  = -L$(HDF5API_ROOT)/lib

ifeq ($(HOSTMACHINE),Darwin)
CXXFLAGS = -I/usr/X11R6/include -I$(HDF5API_ROOT)/src $(FLTK_INC) $(COMMON_INC)
else
CXXFLAGS = -I$(HDF5API_ROOT)/src $(FLTK_INC) $(COMMON_INC)
endif

CPPFLAGS = $(CFLAGS) -g
ifdef ENABLE_LOGGING
CPPFLAGS += -DLOGGING_ENABLED
endif

FLTK_SOURCES = $(wildcard *.fl)
OBJS = $(FLTK_SOURCES:.fl=.o)\
	$(patsubst %.cc,%.o,$(wildcard *.cc))\
	$(patsubst %.c,%.o,$(wildcard *.c))\
	$(patsubst %.C,%.o,$(wildcard *.C))

all: meshalyzer $(LIB_CH5) 

meshalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OBJS) $(LIB_CH5)
	$(CXX) $(CFLAGS) -o meshalyzer $(sort $(OBJS)) $(LIBS)
	fltk-config --post meshalyzer

ifdef HDF5
$(LIB_CH5): 
	cd hdf5api && make all
endif

clean:
	rm -rf $(FLTK_SOURCES:.fl=.h) $(FLTK_SOURCES:.fl=.cc) *.o *.d meshalyzer meshalyzer.app

utils: 
	cd utils && make all

docs: 
	doxygen Doxyfile
ifdef HDF5
	cd hdf5api && doxygen Doxyfile
endif

%.h %.cc: %.fl
	fluid -c $<

-include $(OBJS:.o=.d)

.PHONY: utils


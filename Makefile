HOSTMACHINE := $(shell uname)

HDF5API_ROOT  := ./hdf5api

FLTK_INC      := $(shell fltk-config --use-glut --use-gl --cxxflags)
FLTK_LD_FLAGS := $(shell fltk-config --use-images --use-glut --use-gl --ldflags)
COMMON_INC    := -I. -O0 -g -DOBJ_CLASS -D_REENTRANT -MMD -DNOMINMAX  -fopenmp

#HDF5=1
ifdef HDF5
LIB_CH5       := (HDF5_ROOT)/lib/libch5.a
LIB_HDF5      := -lch5 -lhdf5 -lhdf5_hl 
COMMON_INC    += -DUSE_HDF5
else
LIB_CH5       := 
LIB_HDF5      := 
endif

COMMON_LIBS    = -lpng -lpthread -lm -lz $(LIB_HDF5) 

LIBS     = -L$(HDF5API_ROOT)/lib  -lglut $(FLTK_LD_FLAGS) $(COMMON_LIBS)
LDFLAGS  =  -fopenmp
CXXFLAGS =  -I$(HDF5API_ROOT)/src $(FLTK_INC) $(COMMON_INC)

CPPFLAGS = $(CFLAGS) -g
ifdef ENABLE_LOGGING
CPPFLAGS += -DLOGGING_ENABLED
endif

FLTK_SOURCES = $(wildcard *.fl)
OBJS = $(FLTK_SOURCES:.fl=.o)\
	$(patsubst %.cc,%.o,$(wildcard *.cc))\
	$(patsubst %.c,%.o,$(wildcard *.c))\
	$(patsubst %.C,%.o,$(wildcard *.C))

#the files in OS_files contain OSMESA dependent clauses
OS_files= main.o TBmeshWin.o
OS_OBJS=$(filter-out $(OS_files),$(OBJS)) $(OS_files:.o=_os.o)

all: meshalyzer

meshalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OBJS) $(LIB_CH5)
	$(CXX) $(LDFLAGS) -o meshalyzer $(sort $(OBJS)) $(LIBS)
	fltk-config --post meshalyzer

mesalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OS_OBJS) $(LIB_CH5)
	$(CXX) $(LDFLAGS) -o mesalyzer $(sort $(OS_OBJS)) -lOSMesa $(LIBS)
	fltk-config --post mesalyzer

ifdef HDF5
$(LIB_CH5): 
	cd hdf5api && make all $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OBJS) $(LIB_CH5)

endif

$(OS_files:.o=_os.o): %_os.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPP_FLAGS) -D OSMESA -c -o $@  $<

clean:
	rm -rf $(FLTK_SOURCES:.fl=.h) $(FLTK_SOURCES:.fl=.cc) *.o *.d meshalyzer mesalyzer meshalyzer.app

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


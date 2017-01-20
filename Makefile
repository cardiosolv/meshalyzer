HOSTMACHINE := $(shell uname)
include make.conf

HDF5API_ROOT  := ./hdf5api

FLTK_INC      := $(shell fltk-config --use-glut --use-gl --cxxflags)
FLTK_LD_FLAGS := $(shell fltk-config --use-images --use-glut --use-gl --ldflags)
COMMON_INC    := -I. -DOBJ_CLASS -D_REENTRANT

FLTK_SOURCES  := $(wildcard *.fl)
OBJS = $(FLTK_SOURCES:.fl=.o)\
	$(patsubst %.cc,%.o,$(wildcard *.cc))\
	$(patsubst %.c,%.o,$(wildcard *.c))\
	$(patsubst %.C,%.o,$(wildcard *.C))

ifdef HDF5
LIB_CH5       := $(HDF5API_ROOT)/lib/libch5.a
LIB_HDF5      := -L$(HDF5_ROOT)/lib -lch5 -lhdf5_hl -lhdf5
COMMON_INC    += -DUSE_HDF5
else
LIB_CH5       := 
LIB_HDF5      :=
OBJS          := $(OBJS:HDF5DataBrowser.o=)
endif

# MAC vs Linux differences
ifeq ($(HOSTMACHINE), Darwin)
  GLUT_LIB = -framework GLUT 
else
  GLUT_LIB = -lglut
  OMPFLAG := -fopenmp
ifeq (,$(findstring -lGL,$(FLTK_LD_FLAGS)))
  FLTK_LD_FLAGS += -lGL -lGLU
endif
endif

COMMON_LIBS  = -lpng -lpthread -lm -lz $(LIB_HDF5) 
LIBS         = -L$(HDF5API_ROOT)/lib $(FLTK_LD_FLAGS) $(COMMON_LIBS) 
CPPFLAGS     = -I$(HDF5_ROOT)/include -I$(HDF5API_ROOT)/include $(FLTK_INC) $(COMMON_INC)
CXXFLAGS     = -std=c++11 -g -O$(DEBUG_LEVEL) $(OMP_FLAG) -MMD -DNOMINMAX  

HDF5_CXXFLAGS= $(CXXFLAGS)

ifdef ENABLE_LOGGING
CPPFLAGS += -DLOGGING_ENABLED
endif

#the files in OS_files contain OSMESA dependent clauses
OS_files= main.o Frame.o
OS_OBJS=$(filter-out $(OS_files),$(OBJS)) $(OS_files:.o=_os.o)

all: meshalyzer

meshalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OBJS) $(LIB_CH5)
	$(CXX) $(LDFLAGS) -o meshalyzer $(sort $(OBJS)) $(GLUT_LIB) $(LIBS)
	fltk-config --post meshalyzer

mesalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OS_OBJS) $(LIB_CH5)
	$(CXX) $(LDFLAGS) -o mesalyzer $(sort $(OS_OBJS)) -lOSMesa $(LIBS) -lrt
	fltk-config --post mesalyzer

ifdef HDF5
$(LIB_CH5): 
	cd hdf5api && make all 
	cd hdf5api && make install PREFIX=. 
endif

$(OS_files:.o=_os.o): %_os.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -D OSMESA -c -o $@  $<

clean:
	rm -rf $(FLTK_SOURCES:.fl=.h) $(FLTK_SOURCES:.fl=.cc) *.o *.d meshalyzer mesalyzer meshalyzer.app
ifdef HDF5
	cd hdf5api && make clean
endif

utils: 
	cd utils && make all

docs: 
	doxygen Doxyfile
ifdef HDF5
	cd hdf5api && doxygen Doxyfile
endif

%.h %.cc: %.fl
	fluid -c -o .cc $<

-include $(OBJS:.o=.d)

.PHONY: utils


HOSTMACHINE := $(shell uname)
include make.conf

VTK_LIBDIR     =/usr/local/lib
VTK_INCDIR     =/usr/local/include/vtk-8.1

FLTK_INC      := $(shell fltk-config --use-glut --use-gl --cxxflags)
FLTK_LD_FLAGS := $(shell fltk-config --use-images --use-glut --use-gl --ldflags)
COMMON_INC    := -I. -DOBJ_CLASS -D_REENTRANT

FLTK_SOURCES  := $(wildcard *.fl)
OBJS = $(FLTK_SOURCES:.fl=.o)\
	$(patsubst %.cc,%.o,$(wildcard *.cc))\
	$(patsubst %.c,%.o,$(wildcard *.c))\
	$(patsubst %.C,%.o,$(wildcard *.C))

HDF5API_ROOT  := ./hdf5api
ifdef HDF5
LIB_CH5       := $(HDF5API_ROOT)/lib/libch5.a
LIB_HDF5      := -L$(HDF5API_ROOT)/lib -L$(HDF5_ROOT)/lib -lch5 -lhdf5_hl -lhdf5
COMMON_INC    += -DUSE_HDF5 -I$(HDF5_ROOT)/include -I$(HDF5API_ROOT)/include 
else
LIB_CH5       := 
LIB_HDF5      :=
OBJS          := $(OBJS:HDF5DataBrowser.o=)
endif

# MAC vs Linux differences
ifeq ($(HOSTMACHINE), Darwin)
  GLUT_LIB  = -framework GLUT 
  FLTK_INC += /usr/X11/include
  LIB_EXT   = .dylib
else
  GLUT_LIB  = -lglut
  OMP_FLAG := -fopenmp
  LDFLAGS  += $(OMP_FLAG)
ifeq (,$(findstring -lGL,$(FLTK_LD_FLAGS)))
  FLTK_LD_FLAGS += -lGL -lGLU
endif
  LIB_EXT   = .so
endif

COMMON_LIBS  = -lpng -lpthread -lm -lz $(LIB_HDF5) 
LIBS         =  $(FLTK_LD_FLAGS) $(COMMON_LIBS) 
CPPFLAGS     =  $(FLTK_INC) $(COMMON_INC)
CXXFLAGS     = -std=c++11 -g -O$(DEBUG_LEVEL) $(OMP_FLAG) -MMD -DNOMINMAX  

# VTK
ifdef VTK_LIBDIR
COMMON_INC    += -DUSE_VTK -I$(VTK_INCDIR)
VTK_LSLIBS     = $(shell ls $(VTK_LIBDIR)/libvtk*$(LIB_EXT)|grep -v Python|grep -v Java |grep -v TCL)
a              = $(subst $(VTK_LIBDIR)/lib,-l,$(VTK_LSLIBS) )
VTK_LIBS       = $(subst $(LIB_EXT),,$(a) )
LIBS          += $(VTK_LIBS)
endif

ifdef ENABLE_LOGGING
CPPFLAGS += -DLOGGING_ENABLED
endif

#the files in OS_files contain OSMESA dependent clauses
OS_files= main.o Frame.o
OS_OBJS=$(filter-out $(OS_files),$(OBJS)) $(OS_files:.o=_os.o)

all: meshalyzer

meshalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OBJS) 
	$(CXX) $(LDFLAGS) -o meshalyzer $(sort $(OBJS)) $(GLUT_LIB) $(LIBS)
	fltk-config --post meshalyzer

mesalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OS_OBJS)
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


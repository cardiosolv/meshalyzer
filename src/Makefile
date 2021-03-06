HOSTMACHINE := $(shell uname)
include ../make.conf

Cxx           := $(shell fltk-config --cxx)
FLTK_INC      := $(shell fltk-config --use-glut --use-gl --cxxflags)
FLTK_LD_FLAGS := $(shell fltk-config --use-images --use-glut --use-gl --ldflags)
COMMON_INC    := -I. -DOBJ_CLASS -D_REENTRANT

BUILD_DIR     := ../obj
FLTK_SOURCES  := $(wildcard *.fl)
OBJS          := $(sort $(addprefix $(BUILD_DIR)/, $(FLTK_SOURCES:.fl=.o) \
	             $(patsubst %.cc,%.o,$(wildcard *.cc)) $(patsubst %.C,%.o,$(wildcard *.C)) ))
DEP           := $(OBJS:%.o=%.d)

HDF5API_ROOT  := ../hdf5api
ifdef HDF5
LIB_CH5       := $(HDF5API_ROOT)/lib/libch5.a
LIB_HDF5      := -L$(HDF5API_ROOT)/lib -L$(HDF5_ROOT)/lib -lch5 -lhdf5_hl -lhdf5
COMMON_INC    += -DUSE_HDF5 -I$(HDF5_ROOT)/include -I$(HDF5API_ROOT)/include 
else
LIB_CH5       := 
LIB_HDF5      :=
OBJS          := $(filter-out $(BUILD_DIR)/HDF5DataBrowser.o, $(OBJS))
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

COMMON_LIBS  = -lGLEW -lpng -lpthread -lm -lz $(LIB_HDF5) 
LIBS         =  $(FLTK_LD_FLAGS) $(COMMON_LIBS) 
CPPFLAGS     =  $(FLTK_INC) $(COMMON_INC)
CXXFLAGS     = -std=c++11 -g -O$(DEBUG_LEVEL) $(OMP_FLAG) -MMD -DNOMINMAX  

# VTK
ifdef VTK_LIBDIR
COMMON_INC    += -DUSE_VTK -I$(VTK_INCDIR)
VTK_LSLIBS     = $(shell ls $(VTK_LIBDIR)/libvtk*$(LIB_EXT) | grep -Ev '(Python)|(Java)|(TCL)|(Parallel)|(MPI)|(Filters)|(Imaging)|(Interaction)|(Rendering)' )
a              = $(subst $(VTK_LIBDIR)/lib,-l,$(VTK_LSLIBS) )
VTK_LIBS       = $(subst $(LIB_EXT),,$(a) )
LIBS          += $(VTK_LIBS)
endif

ifdef ENABLE_LOGGING
CPPFLAGS += -DLOGGING_ENABLED
endif

#the files in OS_files contain OSMESA dependent clauses
OS_files:=$(addprefix $(BUILD_DIR)/,$(patsubst %.cc,%.o,$(shell grep -l OSMESA *.cc)))
OS_OBJS:=$(filter-out $(OS_files),$(OBJS)) $(OS_files:.o=_os.o)

all: meshalyzer

meshalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OBJS) 
	$(CXX) $(LDFLAGS) -o meshalyzer $(OBJS) $(GLUT_LIB) $(LIBS)
	fltk-config --post meshalyzer

mesalyzer: $(LIB_CH5) $(FLTK_SOURCES:.fl=.cc) $(OS_OBJS)
	$(CXX) $(LDFLAGS) -o mesalyzer $(OS_OBJS) -lOSMesa $(LIBS) -lrt
	fltk-config --post mesalyzer

ifdef HDF5
$(LIB_CH5): 
	cd hdf5api && make all 
	cd hdf5api && make install PREFIX=. 
endif

$(BUILD_DIR)/%.o: %.cc 
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.C
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OS_files:.o=_os.o): $(BUILD_DIR)/%_os.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -D OSMESA -c -o $@  $<

clean:
	rm -rf $(FLTK_SOURCES:.fl=.h) $(FLTK_SOURCES:.fl=.cc) meshalyzer mesalyzer meshalyzer.app
	rm -f $(BUILD_DIR)/*
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

-include $(DEP)

.PHONY: utils


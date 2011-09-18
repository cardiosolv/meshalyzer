HDF5API_ROOT := ../../hdf5api
CFLAGS       := -std=c99  -g -O0 -Wall -I$(HDF5API_ROOT)/src
CXXFLAGS     := -g -O0 -Wall -I$(HDF5API_ROOT)/src

OBJ1         := $(patsubst %.c, %.o, $(wildcard *.c))
OBJ1         += $(patsubst %.cc, %.o, $(wildcard *.cc))
OBJS         := $(filter-out cmdline.o, $(OBJ1) ) #remove cmdline.o if present
OBJS         += cmdline.o                         #now make sure it is there only once

BINNAME      := package_data
BINPATH      := $(BINNAME)
CC           := h5cc  # use the hdf5 compiler helper
CXX          := h5c++ # use the hdf5 compiler helper
LDFLAGS      := -L$(HDF5API_ROOT)/lib -lch5

prefix := /usr/local

all: $(BINPATH)

$(BINPATH): $(OBJS) 
	$(CXX) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(BINPATH)

cmdline.c: pd.ggo
	gengetopt -u < pd.ggo

clean:
	rm -f *.o $(BINPATH)

.PHONY: install uninstall clean

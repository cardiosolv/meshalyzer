HDF5 = 1
include ../../make.conf

HDF5API_ROOT := ../../hdf5api

CFLAGS       := -std=c99  -g -O0 -Wall -I$(HDF5API_ROOT)/include
CXXFLAGS     := -g -std=c++11 -O0 -Wall -I$(HDF5API_ROOT)/include

OBJ1         := $(patsubst %.c, %.o, $(wildcard *.c))
OBJ1         += $(patsubst %.cc, %.o, $(wildcard *.cc))
OBJS         := cmdline.o $(filter-out cmdline.o, $(OBJ1) )

BINNAME      := package_data
BINPATH      := $(BINNAME)
LDFLAGS      := -L$(HDF5_ROOT)/lib -L$(HDF5API_ROOT)/lib -lch5 -lz
LD           := $(CXX)

prefix := /usr/local

all: $(BINPATH)

$(BINPATH): $(OBJS) 
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $(BINPATH)

cmdline.h cmdline.c: pd.ggo
	gengetopt -u < pd.ggo

clean:
	rm -f *.o $(BINPATH)

.PHONY: install uninstall clean

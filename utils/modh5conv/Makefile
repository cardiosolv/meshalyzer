HDF5API_ROOT := ../../hdf5api
CFLAGS       := -std=c99  -g -O0 -Wall -I$(HDF5API_ROOT)/src
OBJS         := $(patsubst %.c, %.o, $(wildcard src/*.c))
BINNAME      := modh5conv
BINPATH      := bin/$(BINNAME)
CC           := h5cc # use the hdf5 compiler helper
LDFLAGS      := -L$(HDF5API_ROOT)/lib -lch5

prefix := /usr/local

all: $(BINPATH)

$(BINPATH): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(BINPATH)

install:
	install $(BINPATH) $(prefix)/bin/$(BINNAME)

uninstall:
	rm -f $(prefix)/bin/$(BINNAME)

clean:
	rm -f src/*.o $(BINPATH)

.PHONY: install uninstall clean

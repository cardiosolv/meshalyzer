SRCDIR     := src
SRCSUBDIRS := model simulation
TESTDIR    := test
TESTBIN    := $(TESTDIR)/suite
TESTINVALID:= ./test/data/test_invalid.h5
GENBIN     := $(TESTDIR)/data/generator
LIBFILE    := libch5.a
LIBPATH    := lib/$(LIBFILE)
CC         := h5cc # use the hdf5 compiler helper
CFLAGS     := -std=c99 -Wall -Isrc -g -O0
AR         := ar
ARFLAGS    := -rcs
HDF5_ROOT  := $(PETSC_DIR)/$(PETSC_ARCH)

CFLAGS     += -I$(HDF5_ROOT)/include -L$(HDF5_ROOT)/lib
OBJS       := $(patsubst %.c, %.o, $(wildcard $(SRCDIR)/*.c))
OBJS       += $(foreach dir, ${SRCSUBDIRS}, $(patsubst %.c, %.o, $(wildcard $(SRCDIR)/$(dir)/*.c)))
TESTOBJS   := $(patsubst %.c, %.o, $(wildcard $(TESTDIR)/*.c))
GENOBJS    := $(patsubst %.c, %.o, $(wildcard $(TESTDIR)/data/*.c))

ifdef PREFIX
prefix     := $(PREFIX)
else
prefix     := /usr/local
endif

all: $(LIBPATH)

$(LIBPATH): $(OBJS)
	$(AR) $(ARFLAGS) $(LIBPATH) $(OBJS)

test: $(TESTBIN)
	./$(TESTBIN)

test-general: $(TESTBIN)
	./$(TESTBIN) --no-model --no-sim

test-sim: $(TESTBIN)
	./$(TESTBIN) --no-model --no-general

test-model: $(TESTBIN)
	./$(TESTBIN) --no-sim --no-general

$(TESTINVALID): $(GENBIN)
	./$(GENBIN) ./test/data/test_invalid.h5

$(TESTBIN):	$(LIBPATH) $(TESTOBJS) $(TESTINVALID)
	$(CC) $(CFLAGS) $(LIBPATH) $(TESTOBJS) -o $(TESTBIN)

$(GENBIN): $(GENOBJS)
	$(CC) $(CFLAGS) $(GENOBJS) -o $(GENBIN)

install: uninstall
	install -d $(prefix)/include/ch5
	$(foreach dir, ${SRCSUBDIRS}, install -d $(prefix)/include/ch5/$(dir);)
ifneq ($(shell readlink -f $(prefix) ), $(shell readlink -f . ))
	install $(LIBPATH) $(prefix)/lib
endif
	install $(SRCDIR)/*.h $(prefix)/include/ch5
	$(foreach dir, ${SRCSUBDIRS}, install $(SRCDIR)/$(dir)/*.h $(prefix)/include/ch5/$(dir);)

uninstall:
ifneq ($(shell readlink -f $(prefix) ), $(shell readlink -f . ))
	rm -f $(PREFIX)/lib/$(LIBFILE)
else
	$(info Not deleting $(LIBPATH) since src=dst)
endif
	rm -rf  $(PREFIX)/include/ch5

clean:
	rm -f $(LIBPATH) $(TESTBIN) $(GENBIN) $(SRCDIR)/*.o $(SRCDIR)/*/*.o $(TESTDIR)/*.o $(TESTDIR)/data/*.o $(TESTINVALID)

.PHONY: install uninstall clean test test-general test-sim test-model

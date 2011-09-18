DIRS := $(subst /,,$(shell ls -d */ ))

all: $(DIRS)

$(DIRS) :  FORCE
	cd $@ && make all

FORCE: 


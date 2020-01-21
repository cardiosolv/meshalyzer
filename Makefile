define linkexe
 	ln -sf src/$(if $(filter $(shell uname),Darwin),meshalyzer.app/Contents/MacOS/,)$(1) .
endef

all: obj
	cd src && $(MAKE)
	$(call linkexe,meshalyzer) 

mesalyzer:
	cd src && $(MAKE) mesalyzer
	$(call linkexe,$@) 

clean :
	cd src && $(MAKE) clean

obj: 
	mkdir obj


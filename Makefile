all: obj
	cd src && $(MAKE)

mesalyzer:
	cd src && $(MAKE) mesalyzer

clean :
	cd src && $(MAKE) clean

obj: 
	mkdir obj


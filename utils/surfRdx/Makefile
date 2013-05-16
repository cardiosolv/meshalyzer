CXXFLAGS = -g

OBJS = main.o mesh.o pmesh.o triangle.o vec3.o vertex.o cmdline.o

all : surfRdx

surfRdx : cmdline.c $(OBJS) 
	$(CXX) -g -o surfRdx $(OBJS)

%.o : %.cpp
	 $(CXX) -g -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.o : %.cc %.h
	 $(CXX) -g -c $(CFLAGS) $(CPPFLAGS) $< -o $@

cmdline.c: opts.ggo
	gengetopt -u < opts.ggo

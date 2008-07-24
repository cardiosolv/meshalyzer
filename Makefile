
HOSTMACHINE := $(shell uname)

FLTK_INC      := $(shell fltk-config --use-gl --cxxflags)
FLTK_LD_FLAGS := $(shell fltk-config --use-gl --ldflags)
COMMON_LIBS    = -lpng -lpthread -lm -lz
COMMON_INC     = -I. -O0 -g -DOBJ_CLASS  -D_REENTRANT

ifeq ($(HOSTMACHINE),Linux)
LIBS   = $(FLTK_LD_FLAGS) $(COMMON_LIBS)
CFLAGS = $(FLTK_INC) $(COMMON_INC) #-fopenmp
endif

ifeq ($(HOSTMACHINE),Darwin)
LIBS =  -L/sw/lib $(FLTK_LD_FLAGS) $(COMMON_LIBS)
CFLAGS = -I/sw/include -I/usr/X11R6/include $(FLTK_INC) $(COMMON_INC)
endif

ifneq (,$(findstring CYGWIN,$(HOSTMACHINE)))
LIBS     = $(FLTK_LD_FLAGS) $(COMMON_LIBS)
INCLUDES = $(FLTK_INC) $(COMMON_INC) -DNOMINMAX
endif

OBJS = Fl_Gl_Tb_Window.o\
	ContCable.o\
	ClipPlane.o\
	Colourscale.o\
	colourchoice.o\
	Connection.o\
	CutSurfaces.o\
	DataOpacity.o\
	DrawingObj.o\
	FileType.o\
	gl2ps.o\
	Graph.o PlotWin.o plottingwin.o\
	Hexahedron.o\
	HiLiteWinInfo.o\
	IGBheader.o\
	main.o\
	Matrix4x4.o\
	Model.o\
	Mouse.o\
	Myslider.o\
	MyValueInput.o\
	Point.o\
	PolyGon.o\
	PNGwrite.o\
	Prism.o\
	Pyramid.o\
	Quadrilateral.o\
	Region.o\
	Sequence.o\
	Surfaces.o\
	TBmeshWin.o\
	Tetrahedral.o\
	Trackball.o\
	Triangle.o\
	trimesh.o\
	VecData.o\
	Vector3D.o\
	Vector4D.o

READER_OBJS = DataReader.h asciireader.h FileSeqCGreader.h \
			  IGBreader.h ThreadedData.h DataAllInMem.h
	
all: $(OBJS)
	c++ $(CFLAGS) -o meshalyzer $(OBJS) $(LIBS)
	fltk-config --post meshalyzer

clean:
	rm $(OBJS) meshalyzer

format:
	astyle --options=astylerc *.cc *.h *.c

TBmeshWin.o: TBmeshWin.cc $(READER_OBJS)
	c++ -c $(CFLAGS) $(INCLUDES) $<
	
%.o : %.cc
	c++ -c $(CFLAGS) $(INCLUDES) $<

%.o : %.C
	c++ -c $(CFLAGS) $(INCLUDES) $<

utils:
	(cd utils; make )


cmake_minimum_required(VERSION 2.6)

PROJECT(Meshalyzer)

set(FLTK_SKIP_FLUID, ON)
set(FLTK_SKIP_FORMS, ON)
set(FLTK_SKIP_IMAGES, ON)
include(FindFLTK)
if (FLTK_FOUND)
  include_directories(${FLTK_INCLUDE_DIR})
  link_libraries(${FLTK_LIBRARIES})
else (FLTK_FOUND)
  message(FATAL_ERROR "Couldn't find FLTK 1")
endif (FLTK_FOUND)

include(FindZLIB)
if (ZLIB_FOUND)
  include_directories(${ZLIB_INCLUDE_DIR})
  link_libraries(${ZLIB_LIBRARIES})
else (ZLIB_FOUND)
  message(FATAL_ERROR "Couldn't find development files for zlib")
endif (ZLIB_FOUND)

include(FindPNG)
if (PNG_FOUND)
  include_directories(${PNG_INCLUDE_DIR})
  link_libraries(${PNG_LIBRARIES})
  add_definitions(${PNG_DEFINITIONS})
else (PNG_FOUND)
  message(FATAL_ERROR "Couldn't find development files for PNG")
endif (PNG_FOUND)

if (CYGWIN)
  add_definitions(-DNOMINMAX)
endif (CYGWIN)

# No common library for pthreads yet.
find_library(PTHREAD_LIB pthread)
if (PTHREAD_LIB)
  link_libraries(${PTHREAD_LIB})
else (PTHREAD_LIB)
  message(FATAL_ERROR "Couldn't find pthread library")
endif (PTHREAD_LIB)

#Check for GLU
find_library(GLU_LIB GLU)
if (GLU_LIB)
  link_libraries(${GLU_LIB})
else (GLU_LIB)
  message(FATAL_ERROR "Couldn't find GLU")
endif (GLU_LIB)


# ok, now adding everything else
add_definitions(
-DOBJ_CLASS
-D_REENTRANT
)
include_directories(.)

fltk_wrap_ui(meshalyzer 
ClipPlane.fl
colourchoice.fl
DataOpacity.fl
HiLiteWinInfo.fl
isosurf.fl
plottingwin.fl
Sequence.fl
trimesh.fl
TimeLink.fl
)

add_executable(meshalyzer
${meshalyzer_FLTK_UI_SRCS}
AuxGrid.cc
Colourscale.cc
Connection.cc
ContCable.cc
CutSurfaces.cc
DrawingObjects.cc
FileType.cc
Fl_Gl_Tb_Window.C
gl2ps.c
Graph.cc
Hexahedron.cc
IsoLines.cc
IsoSurface.cc
IGBheader.cc
main.cc
Matrix4x4.C
Model.cc
ModInt.C
Mouse.C
Myslider.cc
MyValueInput.cc
PlotWin.cc
PNGwrite.cc
Point.cc
PolyGon.cc
Prism.cc
Pyramid.cc
Quadrilateral.cc
Surfaces.cc
Region.cc
TBmeshWin.cc
Tetrahedral.cc
Trackball.C
Triangle.cc
VecData.cc
Vector3D.C
Vector4D.C
GLee.c
)

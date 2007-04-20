#ifndef DRAWGL_H
#define DRAWGL_H


#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Single_Window.H>
#include "Fl_Gl_Tb_Window.h"
#include "Myslider.h"

typedef enum disp_t { asSurface, asTetMesh, asIsosurface } Display_t;
typedef enum data_state_t { NoData, Static, Dynamic } Data_Type_t;
enum DataColouring { off, on, opacval };

typedef float DATA_TYPE;

static const float opaque_min = 0.95;

#endif

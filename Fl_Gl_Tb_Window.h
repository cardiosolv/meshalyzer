/* A GL window with a trackball interface.  This is a subclass of Fl_Gl_Window
that only implements the handle() method to keep track of mouse motions.
You subclass Tb_Window and implement draw(), and call the transform() method
to orient the scene. */

#ifndef Fl_Gl_Tb_Window_h
#define Fl_Gl_Tb_Window_h

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>
#include "Quaternion.h"
#include "Vector3D.h"
#include "Mouse.h"
#include "Trackball.h"

#define B2_MIMIC_MASK (FL_BUTTON1 | FL_SHIFT)
#define B3_MIMIC_MASK (FL_BUTTON1 | FL_CTRL)

class Fl_Gl_Tb_Window : public Fl_Gl_Window
{
  public:
    Fl_Gl_Tb_Window(int x, int y, int w, int h, const char *l = 0)
        : Fl_Gl_Window(x, y, w, h, l), trackball(mouse) { init(); }
    Fl_Gl_Tb_Window(int w, int h, const char *l = 0)
        : Fl_Gl_Window(w, h, l), trackball(mouse) { init(); }
    ~Fl_Gl_Tb_Window();

    void DoTransform(); // Call this from draw() to transform the view
  protected:
    int handle(int);

  public:
    Mouse mouse;
    Trackball trackball;

  private:
    void init();
    bool isIdleRedrawing;    // call redraw() in any case
    int  map_button( int eventstate );
    bool active;
};

#endif

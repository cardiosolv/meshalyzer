#include "MyValueInput.h"
#include <FL/Fl.H>

MyValueInput :: MyValueInput( int X, int Y, int W, int H, const char *L) :
    Fl_Value_Input(X,Y,W,H,L)
{
  value();
}


/** A value input widget that also responds to mouse wheel events.
 *  Holding CTRL moves by 10, SHIFT by 100, and both by 1000
 *
 * \param event event type
 *
 */
int MyValueInput :: handle( int event )
{
  int jump=1;

  if (event == FL_MOUSEWHEEL) {

    if ( Fl::event_state(FL_CTRL) )
      jump *= 10;
    if ( Fl::event_state(FL_SHIFT) )
      jump *= 100;

    int dy = Fl::event_dy();
    double v1 = value();
    v1 = increment(v1, dy*jump); 
    v1 = clamp(v1); 
    value(v1); 
    return 1; 

  }  else if ( event == FL_KEYBOARD ) {

    if ( Fl::event_state(FL_CTRL) )
      jump = 10;
    if ( Fl::event_state(FL_SHIFT) )
      jump = 100;

    switch (Fl::event_key()) {
        case FL_Up:
            if ( value()+jump*step() <= maximum() ) {
          value(increment(value(), jump));
          do_callback();
          return 1;
        }
        break;
      case FL_Down:
        if ( value()-jump*step() >= minimum() ) {
          value(increment(value(), -jump ));
          do_callback();
          return 1;
        }
        break;
      default:
        return Fl_Value_Input::handle(event);
    }
  }
  return Fl_Value_Input::handle(event);
}


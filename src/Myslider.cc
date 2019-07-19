/*
	flounder - copyright 2002 Edward J. Vigmond

	This file is part of flounder.

	flounder is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

	flounder is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
    along with flounder; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "Myslider.h"
#include <FL/Fl.H>

const int w_valout = 40;

Myslider :: Myslider( int X, int Y, int W, int H, const char *L):
    Fl_Group(X,Y,W,H,L),
    valout(X,Y,w_valout,H),
    slider( X+w_valout, Y, W-w_valout, H )
{
  end();
  slider.type(FL_HOR_NICE_SLIDER);
  slider.minimum(0);
  slider.maximum(0);
  slider.step(1);
  valout.textsize(12);
}

void Myslider :: value( int t )
{
  slider.value( t );
  valout.value( t );
}

float Myslider :: value( void )
{
  return slider.value();
}

void Myslider:: maximum( int t )
{
  slider.maximum(t);
}

float Myslider:: maximum( void )
{
  return slider.maximum();
}

void Myslider:: minimum( int t )
{
  slider.minimum(t);
}

float Myslider:: minimum( void )
{
  return slider.minimum();
}

int Myslider :: handle( int event )
{
  switch ( event ) {
    case FL_KEYBOARD:
      switch (Fl::event_key()) {
        case FL_Left:
          if ( Fl::event_state(FL_SHIFT) ) {
            if ( value()>=minimum()+10 ) value(int(value())-10);
          } else
            if ( value()>minimum() ) value(int(value())-1);
          do_callback();
          break;
        case FL_Right:
          if ( Fl::event_state(FL_SHIFT) ) {
            if ( value()<=maximum()-10 ) value(int(value())+10);
          } else
            if ( value()< maximum() ) value(int(value())+1);
          do_callback();
          break;
          return 1;
      }
    case FL_FOCUS:
    case FL_UNFOCUS:
      return 1;
      break;
    case FL_PUSH:
    case FL_DRAG:
      if ( Fl::event_key()==FL_Button+2 || (Fl::event_key()==FL_Button+1 && Fl::event_state(FL_SHIFT)) ) {
        float newval = float(Fl::event_x()-x()-w_valout)/
                       float(w()-w_valout)*(maximum()-minimum())+minimum();
        if ( newval< minimum() ) newval = minimum();
        if ( newval>maximum() ) newval = maximum();
        value(int(newval));
        do_callback();
        return 1;
      } else if ( Fl::event_key() ==  FL_Button+1 ||
                  Fl::event_key() ==  FL_Button+3 ) {
        double inc = (Fl::event_key()==FL_Button+3) ? 10. : 1.;
        float currx = (value()-minimum())/(maximum()-minimum())*
                      float(w()-w_valout)+x()+w_valout;
        if ( Fl::event_x()>currx ) {
          if ( value()<=maximum()-inc ) value(int(value()+inc));
          do_callback();
        } else {
          if ( value()>=minimum()+inc ) value(int(value()-inc));
          do_callback();
        }
        return 1;
      }
      break;
  }
  return 0;
}

void Myslider :: step( double a )
{
  slider.step(a);
}

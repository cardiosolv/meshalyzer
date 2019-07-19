/* A GL window with a trackball interface. */

#include <stdio.h>
#include <stdlib.h>
#include "Fl_Gl_Tb_Window.h"

// Initialize the trackball window
void Fl_Gl_Tb_Window::init() {

	isIdleRedrawing = false;
	trackball.Reset();
  	mouse.SetWindowSize(w(), h());
}


// Modification by Edward Vigmond to handle 1 button mice
// button         = FL_BUTTON1
// SHIFT + button = FL_BUTTON2
// CTRL  + button = FL_BUTTON3
int Fl_Gl_Tb_Window::map_button( int e ) {

  if( e&FL_BUTTON2 || ((e&B2_MIMIC_MASK)==B2_MIMIC_MASK )) return FL_BUTTON2;
  if( e&FL_BUTTON3 || ((e&B3_MIMIC_MASK)==B3_MIMIC_MASK )) return FL_BUTTON3;
  if( e&FL_BUTTON1 ) return FL_BUTTON1;

  return 0;
}


Fl_Gl_Tb_Window::~Fl_Gl_Tb_Window() {
}

// The handle() method now does the actual work.
int Fl_Gl_Tb_Window::handle(int e) {
	switch (e) {
		case FL_PUSH:
			mouse.UpdatePos(Fl::event_x(),Fl::event_y());
			mouse.UpdatePos(Fl::event_x(),Fl::event_y());
			mouse.UpdateButtons(map_button(Fl::event_state()));
			break;
		case FL_RELEASE:
			mouse.UpdateButtons(map_button(Fl::event_state()));
			break;
		case FL_DRAG:
			mouse.UpdatePos(Fl::event_x(),Fl::event_y());
			mouse.UpdateButtons(map_button(Fl::event_state()));
			break;
		default:
			// some other event, which might be resize
			mouse.SetWindowSize(w(), h());
			return 0;
	}
	trackball.Update();
	if (isIdleRedrawing || trackball.isSpinning || trackball.isChanged) {
		redraw();
		trackball.isChanged = false;
	}
	return 1;
}


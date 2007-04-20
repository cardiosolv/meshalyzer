
#ifndef MY_VALUE_INPUT_H
#define MY_VALUE_INPUT_H

#include <FL/Fl_Value_Input.H>

class MyValueInput : public Fl_Value_Input {
	public:
		MyValueInput( int, int, int, int, const char *L=0 );
		int handle( int );
};

#endif

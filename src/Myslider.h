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

#ifndef MYSLIDER_H

#define MYSLIDER_H
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Value_Output.H>

class Myslider : public Fl_Group
{
    Fl_Slider       slider;
    Fl_Value_Output valout;
  public:
    Myslider( int, int, int, int, const char *L=0 );
    int handle( int );
    float value( void );
    void  value( int );
    void maximum( int );
    float maximum( void );
    void minimum( int );
    float minimum( void );
    void step( double );
};

class Strideslide : public Fl_Group
{
    Fl_Value_Slider slider;
    Fl_Value_Output valout;
    Fl_Box     		samplab;
    int quantity;

  public:
    Strideslide( int, int, int, int, char *L=0 );
    int value( void );
    void  value( int );
    void update( void );
    void setnum( int );
    void color( int );
};

#endif

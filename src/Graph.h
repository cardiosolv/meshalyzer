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
//
// "$Id: Graph.h,v 1.5 2007/01/16 09:06:14 vigmond Exp $"
//
// Arc drawing test program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2000 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems to "fltk-bugs@fltk.org".
//

//! \class A simple class to hold sets for 2D plotting and
//   generate labels
//


#ifndef GRAPH_H
#define GRAPH_H
#include <cstdio>
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <ostream>
#include "plottingwin.h"

const int max_num_sets=10;
const int num_labels=7;
const double nlm1= double(num_labels-1);

class Graph : public Fl_Widget
{
    void draw();
    int handle(int);
  public:
    //! The contructor
    /*!
     * \param X  x-position
     * \param Y  y-position
     * \param W  width
     * \param H  height
     * \param dc number of dynamic curves
     */
    Graph(int X,int Y,int W,int H) : Fl_Widget(X,Y,W,H),
            v_autoscale(true), zero_yaxis(false),numset(0){}
    int  set_2d_data(const double *, const double *, int n, int c, int id=-1 );
    void reset_view( void );
    void range( double &, double &, double &, double& );
    void set_range( double, double, double, double, bool=false );
    void autoscale( bool a ){ v_autoscale=a; }
    void copy_curve(int);
    void clear_curves();
    void scale();
    void write(ostream&, int);
    int  n(){ return numset; }
    void to_world( int x, int y, double &wx, double &wy );
    void rotate();
    int  id( int a ){ if(a<numset)return _id[a]; }
    void num_dynamic( int a ){ num_dyn=a; }
    void toggle_zero_yaxis(){ zero_yaxis=!zero_yaxis;}
    bool crvi_vis(){return crvi && crvi->win->visible();}
    friend void static_curve_info_cb( Fl_Widget *w );
  private:
    const double *xv[max_num_sets], *yv[max_num_sets];
    double  x0, x1, y0, y1;					// data range being plotted
    double  xmin, xmax, ymin, ymax;			// data extrema
    int     np[max_num_sets];				// #points/set
    int     _id[max_num_sets];              // identifier of set 
    int     numset;
    char    xlabel[num_labels][25], ylabel[num_labels][25];
    void    change_view( int, int, int, int );
    void    make_labels(void);
    bool    v_autoscale;				    // autoscale with set change
    int     num_dyn;                        //!< \# non-static curves 
    bool    zero_yaxis;
    double  nicenum( double, int, int );
    void    round_axis_limits(double &amin, double &amax);
  public:
    CurveInfo *crvi=NULL;
};

#endif

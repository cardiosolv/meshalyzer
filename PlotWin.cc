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
#include "plottingwin.h"
#include <FL/Fl_Widget.H>
#include <FL/Fl_File_Chooser.H>

// set the data to plot
void PlotWin :: set_data( int n, double *d, int t, double *xd )
{
	if( n != datasize ) {
		delete[] xv;
		delete[] data;
		xv = new double[n];
		data = new double[n];
		datasize = n;
	}
	datamin = d[0];
	for( int i=0; i<n; i++ ) { 
		if( d[i]<datamin ) datamin = d[i]; 
	}
	memcpy( data, d, n*sizeof(double) );
	if( xd != NULL )
		memcpy( xv, xd, n*sizeof(double) );
	else
		for( int i=0; i<n; i++ ) xv[i] = i;
	highlight( t );
}


// set the data to plot
void PlotWin :: set_data( int n, float *d, int t, float *xd )
{
	if( n != datasize ) {
		delete[] xv;
		delete[] data;
		xv = new double[n];
		data = new double[n];
		datasize = n;
	}
	datamin = d[0];
	for( int i=0; i<n; i++ ) { 
	    data[i] = d[i];
		if( d[i]<datamin ) datamin = d[i]; 
		if( xd != NULL ) xv[i] = xd[i];
	}
	if( xd == NULL )
		for( int i=0; i<n; i++ ) xv[i] = i;

	highlight( t );
}


PlotWin :: ~PlotWin ()
{
  delete[] xv;
  delete[] data;

#ifdef USE_GNUPLOT
	graph->~Fl_Gnuplot();
#endif

}

void PlotWin :: highlight( int tindx=-1 )
{
	if( !(window->visible()) ) return;
	if( !rotated )
		graph->set_2d_data(xv,data,datasize,0);
	else
		graph->set_2d_data(data,xv,datasize,0);
	if( !rotated ) {
		tmx[0] = tmx[1] = xv[tindx];
		tmy[0] = datamin;
		tmy[1] = data[tindx];
		if( !rotated )
			graph->set_2d_data(tmx,tmy,2,1);
		else
			graph->set_2d_data(tmy,tmx,2,1);
	}
#ifdef USE_GNUPLOT
	window->redraw();
#else
	graph->redraw();
#endif
}

void PlotWin :: rotate( bool a )
{
	if( a!=rotated ) {
		rotated = a; 
		rotbut->value(a);
		window->size( window->h(), window->w() );
		highlight();
	}
}
#include <fstream>
void PlotWin :: writedata()
{
	char *ofname = fl_file_chooser( "Select output file", "*.dat", "" );
	if( ofname == NULL ) return;
	ofstream of( ofname );
	for( int i=0; i<graph->n(); i++ ) {
	  if( i==1 ) continue;
      graph->write( of, i );
	  of << endl;
	}
}	





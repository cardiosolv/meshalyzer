#include "plottingwin.h"
#include <FL/Fl_Widget.H>
#include <FL/Fl_File_Chooser.H>

/** set the data to plot
 *
 *  \param n    number of points
 *  \param d    ordinates
 *  \param t    current time
 *  \param xd   abscissa
 *  \param torg initial time
 */
void PlotWin :: set_data( int n, double *d, int t, float dt, float torg, 
                                                             double *xd )
{
  if ( n != datasize ) {
    delete[] xv;
    delete[] data;
    xv = new double[n];
    data = new double[n];
    datasize = n;
  }
  datamin = d[0];
  for ( int i=0; i<n; i++ ) {
    if ( d[i]<datamin ) datamin = d[i];
  }
  memcpy( data, d, n*sizeof(double) );
  if ( xd != NULL )
    memcpy( xv, xd, n*sizeof(double) );
  else
    for ( int i=0; i<n; i++ ) xv[i] = torg+i*dt;
  highlight( t );
}


/** set the data to plot
 *
 *  \param n    number of points
 *  \param d    ordinates
 *  \param t    current time
 *  \param xd   abscissa
 *  \param torg initial time
 */
void PlotWin :: set_data( int n, float *d, int t, float dt, float torg,
                                                            float *xd )
{
  if ( n != datasize ) {
    delete[] xv;
    delete[] data;
    xv = new double[n];
    data = new double[n];
    datasize = n;
  }
  datamin = d[0];
  for ( int i=0; i<n; i++ ) {
    data[i] = d[i];
    if ( d[i]<datamin ) datamin = d[i];
    if ( xd != NULL ) xv[i] = xd[i];
  }
  if ( xd == NULL )
    for ( int i=0; i<n; i++ ) xv[i] = torg + i*dt;

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
  if ( !(window->visible()) ) return;
  if ( !rotated )
    graph->set_2d_data(xv,data,datasize,0);
  else
    graph->set_2d_data(data,xv,datasize,0);
  tmx[0] = tmx[1] = xv[tindx];
  tmy[0] = datamin;
  tmy[1] = data[tindx];
  if ( !rotated )
    graph->set_2d_data(tmx,tmy,2,1);
  else
    graph->set_2d_data(tmy,tmx,2,1);
#ifdef USE_GNUPLOT
  window->redraw();
#else
  graph->redraw();
#endif
}

void PlotWin :: rotate( bool a )
{
  if ( a!=rotated ) {
    rotated = a;
    graph->rotate();
    rotbut->value(a);
    window->size( window->h(), window->w() );
    highlight();
  }
}
#include <fstream>
void PlotWin :: writedata()
{
  char *ofname = fl_file_chooser( "Select output file", "*.dat", "" );
  if ( ofname == NULL ) return;
  ofstream of( ofname );
  for ( int i=0; i<graph->n(); i++ ) {
    if ( i==1 ) continue;
    graph->write( of, i );
    of << endl;
  }
}





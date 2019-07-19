/* \file Graph.cc
 *
 * A class for drawing simple 2D plots on linear x and y scales (for now)
 * The graph widget can draw multiple sets.
 * Sets greater than 0 are the static sets.
 */
#include "plottingwin.h"
#include "Graph.h"
#include <FL/Fl_Menu_Item.H>
#include <FL/fl_ask.H>
#include <sstream>
#include <math.h>

const int setcolour[]=
  {
    FL_RED, FL_GREEN, FL_BLUE, FL_DARK_MAGENTA,
    FL_YELLOW , FL_MAGENTA, FL_CYAN, FL_DARK_CYAN, FL_DARK_RED, FL_GRAY
  };
const double l_marg=0.1;
const double r_marg=0.05;
const double top_marg=.05;
const double bot_marg=0.1;
const double l_axis_sep=0.;
const double tic_length=0.025;
const double plotw=1.-r_marg-l_marg;
const double ploth=1.-top_marg-bot_marg;

// input an x-range to display
void xrange_cb( Fl_Widget *w, void *p )
{
  Graph* g = (Graph *)w;
  double a, b, c, d;
  g->range( a, b, c, d );
  char range[256];
  sprintf( range, "%f:%f", a, b );
  const char *s=fl_input("Enter X range (x1:x2)", range );
  if ( s != NULL && sscanf( s, "%lf : %lf", &a, &b ) == 2 ) {
    g->set_range( a, b, c, d );
    g->redraw();
  }
}

// input an y-range to display
void yrange_cb( Fl_Widget *w, void *p )
{
  Graph* g = (Graph *)w;
  double a, b, c, d;
  g->range( a, b, c, d );
  char range[256];
  sprintf( range, "%f:%f", c, d );
  const char *s=fl_input("Enter Y range (y1:y2)", range );
  if ( s != NULL && sscanf( s, "%lf : %lf", &c, &d ) == 2 ) {
    g->set_range( a, b, c, d );
    g->redraw();
  }
}

void reset_cb( Fl_Widget *w, void *p )
{
  ((Graph *)w)->reset_view();
}


void
zero_yaxis_cb( Fl_Widget *w, void *p )
{
  Graph* g = (Graph *)w;
  g->toggle_zero_yaxis();
  g->redraw();
}

const int button_height=25;

void
but_disp_toggle_cb( Fl_Widget *w, void *p )
{
  PlotWin *pwin=(PlotWin *)(((Graph *)w)->window()->user_data());

  if ( pwin->button_grp->visible()) {
    pwin->button_grp->hide();
    pwin->graph->size(pwin->graph->w(),pwin->graph->h()+button_height );
    pwin->window->redraw();
  } else {
    pwin->graph->size(pwin->graph->w(),pwin->graph->h()-button_height );
    pwin->button_grp->show();
    pwin->window->redraw();
  }
}


void
close_graph_cb( Fl_Widget *w, void *p )
{
  PlotWin *pwin=(PlotWin *)(((Graph *)w)->window()->user_data());

  pwin->window->hide();
  pwin->but->value(0);
}


void
static_curve_info_cb( Fl_Widget *w ) 
{
  Graph* g = (Graph *)w;
  if( !g->crvi ) g->crvi = new CurveInfo;
  g->crvi->win->show();
  g->crvi->info->clear();
  for(int i=0; i<g->n(); i++) {
    if( g->id(i) < 0 ) continue;
    stringstream legend;
    int txtclr = (setcolour[i]==FL_YELLOW || setcolour[i]==FL_GRAY)? FL_BLACK : FL_WHITE;
    legend << "@B" << setcolour[i] << "@C" << txtclr << "@b@l@c@."<<g->id(i);
    g->crvi->add_line( legend.str() );
  }
}

void
clear_curves_cb( Fl_Widget *w )
{
  Graph* g = (Graph *)w;
  g->clear_curves();
  if( g->crvi && g->crvi_vis() ) static_curve_info_cb(g);
}


Fl_Menu_Item graph_pop_menu[] = {
  { "reset view",  'r', (Fl_Callback *)reset_cb, 0, 0, 0, 0, 14, 56 },
  { "zero Y-axis", 'z', (Fl_Callback *)zero_yaxis_cb, 0, 0, 0, 0, 14, 56 },
  { "X range", 'x',  (Fl_Callback *)xrange_cb, 0, 0, 0, 0, 14, 56  },
  { "Y range", 'y',  (Fl_Callback *)yrange_cb, 0, 0, 0, 0, 14, 56  },
  { "curve legend", 's',  (Fl_Callback *)static_curve_info_cb, 0, 0, 0, 0, 14, 56  },
  { "toggle button display", 'b',  (Fl_Callback *)but_disp_toggle_cb, 0, 0, 0, 0, 14, 56  },
  { "clear static curves", 'c',  (Fl_Callback *)clear_curves_cb, 0, 0, 0, 0, 14, 56  },
  { "close", 'c',  (Fl_Callback *)close_graph_cb, 0, 0, 0, 0, 14, 56  },
  {0}
};

// menu to display mouse coordinates
Fl_Menu_Item graph_mouse_pos[] =  {
  { "mouse position", 0, (Fl_Callback *)NULL },
  {0}
};

// plot the entire data set
void Graph::reset_view(void)
{
  set_range( xmin, xmax, ymin, ymax, true );
  make_labels();
  redraw();
}


/** make a static copy of the current curve, allocating the necessary
 *  memory to make the copy
 *
 *  \param curve index to copy
 */
void Graph::copy_curve( int c )
{

  if ( numset>=max_num_sets )
    return;

  np[numset] = np[c];

  double *tx = new double[np[c]];
  double *ty = new double[np[c]];

  for ( int i=0; i<np[c]; i++ ) {
    tx[i] = xv[c][i];
    ty[i] = yv[c][i];
  }

  xv[numset]  = tx;
  yv[numset]  = ty;
  _id[numset] = _id[c];
  numset++;
}


// clear curves
void Graph :: clear_curves()
{
  for ( int c=num_dyn; c<numset; c++ ) {
    delete[] xv[c];
    delete[] yv[c];
    np[c] = 0;
  }
  numset = 2;
  scale();
  redraw();
}

// given the zoom window coordinates, set the new range to plot
void Graph::change_view( int nx, int ny, int nw, int nh )
{
  double x0n, x1n, y0n, y1n;

  to_world( nx, ny, x0n, y1n );
  to_world( nx+nw, ny+nh, x1n, y0n );

  set_range( x0n, x1n, y0n, y1n, true );
  redraw();
}

// main drawing routine
void Graph :: draw()
{

  fl_push_matrix();
  fl_color(230,230,230);			// grey background
  fl_rectf(x(),y(),w(),h());
  // determine drawing transformation
  fl_translate( x(), y() );
  fl_scale( plotw*w()/(x1-x0), -ploth*h()/(y1-y0) );
  fl_translate( -x0+(x1-x0)*l_marg/plotw, -y1-(y1-y0)*top_marg/ploth );
  // draw the curves
  fl_push_clip( x()+int(w()*l_marg), y()+int(h()*top_marg),
                int(w()*plotw), int(h()*ploth) );
  for ( int i=0; i<numset; i++ ) {
    fl_color( setcolour[i] );
    fl_begin_line();
    for ( int j=0; j<np[i]; j++ )
      fl_vertex( xv[i][j], yv[i][j] );
    fl_end_line();
  }
  // drax zero Y axis
  if( zero_yaxis ) {
    fl_color(FL_BLACK );
    fl_begin_line();
      fl_vertex( xmin, 0 );
      fl_vertex( xmax, 0 );
    fl_end_line();
  }
  fl_pop_clip();
  fl_color(FL_BLACK );
  // draw frame
  fl_begin_line();
    fl_vertex( x0, y0 );
    fl_vertex( x1, y0 );
    fl_vertex( x1, y1 );
    fl_vertex( x0, y1 );
    fl_vertex( x0, y0 );
  fl_end_line();
  // draw labels
  fl_font( FL_HELVETICA, 10 );
  for ( int i=0; i<num_labels; i++ ) {
    // x labels
    fl_draw( xlabel[i],
             int(x()+double(w())*(l_marg+i*plotw/nlm1)-fl_width(xlabel[i])/2.),
             int(y()+0.97*h()) );
    // y labels
    fl_draw( ylabel[i], x(),
             int(y()+h()*(1.-0.1/1.15-1./1.15*i/nlm1)),
             int(l_marg*w()), 0, (Fl_Align)(FL_ALIGN_RIGHT) );
    // botom x tics
    fl_begin_line();
      fl_vertex( x0+(x1-x0)*i/nlm1, y0 );
      fl_vertex( x0+(x1-x0)*i/nlm1, y0+(y1-y0)*tic_length );
    fl_end_line();
    // top x tics
    fl_begin_line();
      fl_vertex( x0+(x1-x0)*i/nlm1, y0+(y1-y0) );
      fl_vertex( x0+(x1-x0)*i/nlm1, y0+(y1-y0)*(1.-tic_length) );
    fl_end_line();
    // left y tics
    fl_begin_line();
      fl_vertex( x0, y0+(y1-y0)*i/nlm1 );
      fl_vertex( x0+(x1-x0)*tic_length, y0+(y1-y0)*i/nlm1 );
    fl_end_line();
    // right y tics
    fl_begin_line();
      fl_vertex( x1, y0+(y1-y0)*i/nlm1 );
      fl_vertex( x0+(x1-x0)*(1.-tic_length), y0+(y1-y0)*i/nlm1 );
    fl_end_line();
  }
  fl_pop_matrix();
}

/** Set the data for a set
 *
 * to add a new set, set it to the next available set
 *
 * \note no memeory is allocated, only the pointers are copied

  \param x     abscissa vector
  \param y     ordinate vector
  \param n     size of vectors
  \param setno set\# to change
*/
int
Graph :: set_2d_data(const double *x, const double *y, int n, int setno, int i)
{
  if ( setno >= max_num_sets )
    return 1;

  xv[setno]  = x;
  yv[setno]  = y;
  np[setno]  = n;
  _id[setno] = i;

  scale();

  if ( setno==numset) numset++;

  redraw();
  return 0;
}

//* rotate graph by 90 degrees
void
Graph :: rotate()
{
  for( int i=num_dyn; i<numset; i++ ) {
    const double *t = xv[i];
    xv[i] = yv[i];
    yv[i] = t;
  }
  scale();
}


//* scale the graph
void Graph::scale()
{
  xmin = xmax = xv[0][0];
  ymin = ymax = yv[0][1];

  for ( int s=0; s<numset; s++ )
    for ( int i=0; i<np[s]; i++ ) {
      if ( xv[s][i]<xmin )
        xmin = xv[s][i];
      if ( xv[s][i]>xmax )
        xmax = xv[s][i];
      if ( yv[s][i]<ymin )
        ymin = yv[s][i];
      if ( yv[s][i]>ymax )
        ymax = yv[s][i];
    }

  if ( v_autoscale == true ) {
    set_range( xmin, xmax, ymin, ymax, true );
  }

  make_labels();
}


//! Generate the string labels for the axes
void Graph::make_labels( void )
{

  for ( int i=0; i<num_labels; i++ ) {
    sprintf( xlabel[i], "%.4g", x0 + (x1-x0)*i/((double)num_labels-1.));
    sprintf( ylabel[i], "%.4g", y0 + (y1-y0)*i/((double)num_labels-1.));
  }

}


//! Event handler
int Graph::handle(int event)
{
  static int ix, iy;
  static int dragged;
  static int sx, sy, sw, sh;

  switch (event) {
    case FL_PUSH:
      if ( Fl::event_button() == 3 ||
           ( (Fl::event_button()==1 && Fl::event_key(FL_Control_L)) ||
             (Fl::event_button()==1 && Fl::event_key(FL_Control_R)) ) ) {
        const Fl_Menu_Item* m = graph_pop_menu->popup(Fl::event_x(), Fl::event_y(),NULL,0,0);
        if (m) m->do_callback(this, (void*)m);
        return 1;
      } else if (  Fl::event_button() == 2 ||
                   ( (Fl::event_button()==1 && Fl::event_key(FL_Shift_L)) ||
                     (Fl::event_button()==1 && Fl::event_key(FL_Shift_R)) ) ) {
        double wx, wy;
        to_world( Fl::event_x(),  Fl::event_y(), wx, wy );
        ostringstream mpos;
        mpos << "( " << wx << ", " << wy << " )";
        const string mposc = mpos.str();
        graph_mouse_pos[0].text = mposc.c_str();
        const Fl_Menu_Item* m = graph_mouse_pos->popup(Fl::event_x(), Fl::event_y(),NULL,0,0);
      } else if ( Fl::event_button() == 1 ) {
        ix = Fl::event_x(); if (ix<x()) ix=x(); if (ix>=x()+w()) ix=x()+w()-1;
        iy = Fl::event_y(); if (iy<y()) iy=y(); if (iy>=y()+h()) iy=y()+h()-1;
        dragged = 0;
        return 1;
      }
      break;
    case FL_DRAG:
      dragged = 1;
      int x2,y2;
      x2 = Fl::event_x(); if (x2<x()) x2=x(); if (x2>=x()+w()) x2=x()+w()-1;
      y2 = Fl::event_y(); if (y2<y()) y2=y(); if (y2>=y()+h()) y2=y()+h()-1;
    if (ix < x2) {sx = ix; sw = x2-ix;} else {sx = x2; sw = ix-x2;}
      if (iy < y2) {sy = iy; sh = y2-iy;} else {sy = y2; sh = iy-y2;}
      window()->make_current();
      fl_overlay_rect(sx,sy,sw,sh);
      return 1;
      break;
    case FL_RELEASE:
      if ( Fl::event_button() != 1 || !dragged)
        return 0;
      dragged = 0;
      window()->make_current();
      fl_overlay_clear();
      if ( sw < 3 || sh < 3)
        return 0;
      else
        change_view( sx, sy, sw, sh );
  }
  return 0;
}

/** return the range currently plotted

  \param a  minimum x value
  \param b  maximum x value
  \param c  minimum y value
  \param d  maximum y value
*/
void Graph::range( double& a, double& b, double& c, double& d )
{
  a = x0; b = x1; c = y0; d = y1;
}

/** set the range to plot
 *
 * \param a     minimum x value
 * \param b     maximum x value
 * \param c     minimum y value
 * \param d     maximum y value
 * \param round round axis limits
*/
void Graph::set_range( double a, double b, double c, double d, bool round )
{
  if ( a<b ) {
    x0 = a; x1 = b;
  } else {
    x0 = b; x1 = a;
  }
  if ( c<d ) {
    y0 = c; y1 = d;
  } else {
    y0 = d; y1 = c;
  }
  if( round ) {
    round_axis_limits( x0, x1 );
    round_axis_limits( y0, y1 );
  }

  make_labels();
}

/** output the curves to a file
 *
 *  \param of output stream
 *  \param s  set\#
 */
void Graph::write( ostream& of, int s )
{
  for ( int i=0; i<np[s]; i++ ) of << xv[s][i] << "\t" << yv[s][i] << "\n";
}


/** convert device coordinates to world coordinates
 *
 *  \param x0 device x coord
 *  \param y0 device y coord
 *  \param wx world x
 *  \param wy world y
 */
void
Graph :: to_world( int xn, int yn, double &wx, double &wy )
{
  window()->make_current();

  int    xx0 = int(x()+l_marg*w());    // starting x pixel of graph area
  double  dx = w()*plotw;              // width of graphing area in pixels
  wx = x0 + (x1-x0)*double(xn-xx0)/dx;

  int    yy0 = int(y()+(1.-bot_marg)*h());
  double  dy = h()*ploth;
  wy = y0 + (y1-y0)*double(yy0-yn)/dy;
}


#define NICE_FLOOR -1
#define NICE_CEIL  +1

/**
 * @brief generate rounded limit for axes
 *
 * @param[inout] amin minimum value
 * @param[inout] amax maximum value
 *
 * @return nothing
 */
void
Graph ::  round_axis_limits(double &amin, double &amax )
{
  // this routine stolen from grace
  int nrange;

  if (amin == amax) {
    if( amin == 0 ) {
      amin = -1.0;
      amax = +1.0;
    } else if( amin>0 )  {
      amin /= 2.0;
      amax *= 2.0;
    } else if( amin<0 )  {
      amin *= 2.0;
      amax /= 2.0;
    }
  } 

  if (amin*amax>0) {
    nrange = -rint(log10(fabs(2*(amax - amin)/(amax + amin))));
    nrange = std::max(0, nrange);
  } else {
    nrange = 0;
  }
  amin = nicenum(amin, nrange, NICE_FLOOR);
  amax = nicenum(amax, nrange, NICE_CEIL);
  if (amin*amax>0) {
    if (amax/amin > 5.0) {
      amin = 0.0;
    } else if (amin/amax > 5.0) {
      amax = 0.0;
    }
  }
}


/**
 * @brief round off number
 *
 * @param x      the number
 * @param nrange range to search
 * @param round  how to round, -1=floor,1=ceiling
 *
 * @return the nice number
 */
double
Graph :: nicenum(double x, int nrange, int round)
{
  // this routine stolen from grace

  if (x == 0.0) return(0.0);

  int xsign = x>0 ? 1 : -1;

  x = fabs(x);

  double fexp = floor(log10(x)) - nrange;
  double sx = x/pow(10.0, fexp)/10.0;            /* scaled x */
  double rx = floor(sx);                         /* rounded x */
  double f = 10*(sx - rx);                       /* fraction between 0 and 10 */

  double y;
  if ((round == NICE_FLOOR && xsign == +1) ||
      (round == NICE_CEIL  && xsign == -1)) {
    y = (int) floor(f);
  } else if ((round == NICE_FLOOR && xsign == -1) ||
             (round == NICE_CEIL  && xsign == +1)) {
    y = (int) ceil(f);
  } else {    /* round == NICE_ROUND */
    if (f < 1.5)
      y = 1;
    else if (f < 3.)
      y = 2;
    else if (f < 7.)
      y = 5;
    else
      y = 10;
  }
  sx = rx + (double) y/10.0;

  return (xsign*sx*10.0*pow(10.0, fexp));
}


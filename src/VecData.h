/*
 * VecData class - stores and draws vector data
 *
 * It stores the geometry because it is usually desirable to display the
 * vector data at a lower density than scalar data
 */
#ifndef VECDATA_H
#define VECDATA_H

#include "Colourscale.h"
#include <FL/glu.h>

#ifdef USE_HDF5
#include "ch5/ch5.h"
#endif

enum DataType {Vector,Scalar,FixedVCdata};

void draw_arrow( GLUquadricObj* quado,
                 GLfloat stick, GLfloat head, GLfloat stick_rad, GLfloat head_rad, bool draw_head=true );

class VecData
{
  public:
    VecData(const char*);
    ~VecData();
    int          maxtime(){ return numtm-1; }
    void         draw(int,float);
    void         length( float length ){ _length=length; }
    float        length() const { return _length; }
    void         colourize();
    GLfloat*     colour(){ return _colour; }
    void         colour(float r,float g, float b,float a ){_colour[0]=r;_colour[1]=g;_colour[2]=b;_colour[3]=a;}
    void         display(bool a) {_disp = a; }
    void         length_det( DataType dt ){if(dt!=Scalar||sdata)_length_det=dt;}
    DataType     length_det(void ){return _length_det;}
    void         colour_det( DataType dt ){if(dt!=Scalar||sdata)
	                                          {_colour_det=dt; optimize_cs();} }
    DataType     colour_det(void ){return _colour_det;}
    void         auto_cs(bool a){ autocal=a; }
    void         optimize_cs();
    void         stride( int a ){ _stride=a; }
    void         start( int a ){ _start=a; }
    void         stoch_stride( bool a ){ _stoch = a; }
    void         heads( bool b ){ _draw_heads=b; }
    Colourscale *cs;			// colour scale for display
    VecData&     operator=(const VecData*);
    bool         have_scalar(){ return sdata; }
    int          size(void){ return numpt; }
  private:
    int      numpt       = 0;		// number of spatial points
    int      numtm       = 0;		// number of time instances
    float*   pts         = NULL;	// locations
    float*   vdata       = NULL;    // vector data
    float*   sdata       = NULL;    // scalar data
    float    _length     = 1;		// length of vectors
    GLfloat  _colour[4];	// colour of vectors
    int      _numcol;       // number of colours
    bool     _disp       = true;	// whether or not to draw
    float    maxmag      = 0;		// maximum magnitude of data
    bool     autocal     = false;	// autocalibrate each time
    DataType _length_det = Vector;	// what determines length
    DataType _colour_det = Vector;  // what determines colour
    float scalar_min,		        // scalar data extrema
    scalar_max;
    GLUquadricObj* quado;
    int      _last_tm    = 0;             // last time drawn
    int      _stride     = 1;
    int      _start      = 0;
    bool     _stoch      = false;  // true for stochastic stride
    bool     _3D         = true;
    bool     _draw_heads = true;   // draw arrow heads
#ifdef USE_HDF5
    void     read_vec_HDF5( const char *);
#endif
    void     read_vec_nonHDF5( const char *);
};


// dot 2 vectors
template<class T>
T dot( const T *a, const T *b )
{
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

// cross 2 vectors
template<class T>
T*
cross( const T* a, const T* b, T* c )
{
  c[0] =  a[1]*b[2] - a[2]*b[1];
  c[1] = -a[0]*b[2] + a[2]*b[0];
  c[2] =  a[0]*b[1] - a[1]*b[0];
  return c;
}

template<class T>
T magnitude( const T* a )
{
  return sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2]);
}


template<class T>
T mag2( const T* a )
{
  return a[0]*a[0] + a[1]*a[1] + a[2]*a[2];
}


template<class T>
T* normalize( T* a )
{
  T mag = magnitude(a);
  for ( int i=0; i<3; i++ )
    a[i] /= mag;
  return a;
}


template<class T>
T* add( const T* a, const T* b, T* c )
{
  for ( int i=0; i<3; i++ )
    c[i] = a[i]+b[i] ;
  return c;
}


template<class T>
T* sub( const T* a, const T* b, T* c )
{
  for ( int i=0; i<3; i++ )
    c[i] = a[i]-b[i] ;
  return c;
}


template<class T>
T* scale( T* a, float f )
{
  for ( int i=0; i<3; i++ )
    a[i] *= f;
  return a;
}
#endif

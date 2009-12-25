#ifndef DRAWINGOBJECT_H
#define DRAWINGOBJECT_H

#include "objects.h"
#include <vector>
#include "drawgl.h"
#include "Colourscale.h"
#include "DataOpacity.h"
#include <zlib.h>
#include "Interpolator.h"

gzFile openFile( const char*, const char* );

#define MAX_NUM_SURF 6
#define MAX_NUM_SURF_NODES 4

class DrawingObj
{
  public:
    //! draw single objs
    virtual void   draw( int, GLfloat*, float size=1 )=0;
    //! draw several objs
    virtual void   draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                         int stride=1, dataOpac* dopac=NULL )=0;
    virtual bool   read( const char * ) = 0;
    int    num() const { return _n; }		//!< get \#objects
    DrawingObj( void ): _n(0),_size(1),_3D(false){}
    void   translucency( bool );            //!< set tranlucency
    void   size( float s ){ _size = s; }
    float  size( void ){ return _size; }
    void   threeD( bool b ){ _3D = b; }
    bool   threeD( void ){ return _3D; }
  protected:
    int   _n;			                    //!< \# objects
    float _size;                            //!< size to draw objects
    bool  _3D;
};


class Point: public DrawingObj
{
  public:
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    void     register_vertex( int, vector<bool>& );
    const   GLfloat* pt( int p=0 ){ return _pts+p*3; }
    Point(): _pts(NULL), _base1(false) {}
    ~Point() { if ( _n ) free(_pts); }
    const   GLfloat* pt( int p ) const { return _pts+p*3; }
    void     setVis( bool v ){ if (v) _visible=&_allvis; }
    void     setVis( vector<bool>* v ){ _visible=v; }
    bool     vis( int n ) const { return (*_visible)[n]; }
    bool     vis( int n, bool b ) const { (*_visible)[n]=b; }
    const vector<bool>* vis() const { return _visible; }
    const   GLfloat* offset() const { return _offset; }
    void    offset( const GLfloat*o )  { memcpy(_offset,o,sizeof(GLfloat)*3); }
    void    base1(bool b){ _base1 = b; }
    void    add( GLfloat *, int n=1 );
    const GLfloat* operator[] (int i){ return _pts+3*i; }
  private:
    GLfloat*     _pts;		  //!< point list
    vector<bool>*_visible;    //!< points which get drawn
    GLfloat      _offset[3];  //!< centering offset
    bool         _base1;      //!< true for base 1
    vector<bool>_allvis;	  //!< all true
};


class MultiPoint : public DrawingObj
{
  public:
    MultiPoint( Point *p, int n, int e ):_pt(p),_ptsPerObj(n),_node(NULL),
                                 _nedge(e){}
    ~MultiPoint(){ if ( _n ) delete[] _node; }
    const int* obj( int n=0 ) { return _node+n*_ptsPerObj; }
    int     ptsPerObj(){ return _ptsPerObj; }
    void    register_vertices(int, int, vector<bool>& );
    void    add( int *n );
    const   Point* pt(){ return _pt; }
    void    define( const int *nl, int n=1 );
    MultiPoint **isosurf( DATA_TYPE *d, DATA_TYPE val, int &, 
                                    vector<Interpolator<DATA_TYPE>*> *a=NULL );
    virtual const int*iso_polys(unsigned int)=0; 
  protected:
    int *  _node;			//!< list of nodes defining objects
    Point* _pt;             //!< pointer to point list
    int    _ptsPerObj;		//!< \#nodes to define one object
    int    _nedge;          //!< \#edges
};


class Connection : public MultiPoint
{
  public:
    Connection(Point *p):MultiPoint(p,2,1) {}
    void add( int, int );	//!< add a connection
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual DrawingObj *isosurf( DATA_TYPE *d, DATA_TYPE val ){}
    virtual bool     read( const char * );
    const int *iso_polys(unsigned int index){return NULL;}
};


//! Cable of Contiguous points
class ContCable : public MultiPoint
{
  public:
    ContCable(Point *p):MultiPoint(p,1,1) {}
    const int* obj( int n=0 ) { return _node+n; }
    void add( int, int );
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    void             register_vertices( int, int, vector<bool>& );
    const int *iso_polys(unsigned int index){return NULL;}
};


// closed convex polygons
class SurfaceElement : public MultiPoint
{
  public:
    SurfaceElement(Point *p, int n):MultiPoint(p,n,n),_nrml(NULL) {}
    virtual void     compute_normals( int, int )=0;
    const   void     nrml( GLfloat *n ){ _nrml=n; };
  inline  const   GLfloat* nrml( int a=0 ) {return _nrml==NULL?NULL:_nrml+3*a; }
    virtual void     draw( int, GLfloat*, float=1 )=0;
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride, dataOpac* dopac, const GLfloat * )=0;
    void     read_normals( int, int, const char * );
    void     vertnorm( GLfloat *a ){ _ptnrml=a; }
    ~SurfaceElement(){ if ( _nrml!=NULL ) delete[] _nrml; }
  protected:
    GLfloat* _nrml;
    GLfloat* _ptnrml;
};

class PolyGon : public SurfaceElement
{
  public:
    PolyGon( Point *p, int n ):SurfaceElement(p,n) {}
    virtual void compute_normals( int a, int b );
    virtual void draw( int, GLfloat*, float size=1 );
    virtual void draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride, dataOpac* dopac, const GLfloat * );
    virtual void draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool read( const char * ){}
	static const int  _zero;
    const int* iso_polys(unsigned int index){return &_zero;}
};


class Triangle : public SurfaceElement
{
  public:
    Triangle(Point *p):SurfaceElement(p,3) {}
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride, dataOpac* dopac, const GLfloat *);
    virtual bool     read( const char * );
    virtual void     compute_normals( int, int );
    virtual DrawingObj *isosurf( DATA_TYPE *d, DATA_TYPE val ){}
    bool     add( const char * );
    const int*       iso_polys(unsigned int);
  protected:
    int         countInFile( const char * );
};


class Quadrilateral : public SurfaceElement
{
  public:
    Quadrilateral(Point *p):SurfaceElement(p,4) {}
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride, dataOpac* dopac, const GLfloat *);
    virtual bool     read( const char * );
    virtual void     compute_normals( int, int );
    bool     add( const char * );
    const int*       iso_polys(unsigned int);
  protected:
    int         countTrisInFile( const char * );
};


//! Volume elements
class VolElement : public MultiPoint
{
  public:
    VolElement( Point *p, int n, int e ):MultiPoint( p, n, e ) {}
    const   int* region() const { return _region; }
    int  region(int a) const { return _region[a]; }
    void region(int a, int b) { _region[a] = b; }
    void add( int *n, int r=-1 );
    virtual void draw_out_face( int )=0;
    virtual SurfaceElement *cut( char*, GLfloat*, Interpolator<DATA_TYPE>*&, int e=0 )=0;
    virtual int surfaces(int **, int v=0) = 0;
  protected:
    int*        _region;		//!< region for each element
    Connection* _edges;         //!< egdes for drawing elements
    SurfaceElement* planecut( char*, GLfloat*, Interpolator<DATA_TYPE>*&, int, const int [][2], int e  );
    int make_surf_nodelist(int, int**, const int, int, const int **);
};


class Tetrahedral : public VolElement
{
  public:
    Tetrahedral(Point *p ): VolElement(p,4,6) {}
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual void     draw_out_face( int );
    virtual SurfaceElement* cut(char*,GLfloat*,Interpolator<DATA_TYPE>*&,int=0);
    const int* iso_polys( unsigned int );
    virtual int surfaces(int **, int v=0);
};

class Prism : public VolElement
{
  public:
    Prism(Point *p ): VolElement(p,6,9) {}
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual void draw_out_face( int );
    virtual SurfaceElement* cut(char *,GLfloat*,Interpolator<DATA_TYPE>*&,int);
    const   int*     iso_polys(unsigned int);
    virtual int surfaces(int **, int v=0);
};


class Hexahedron : public VolElement
{
  public:
    Hexahedron(Point *p ): VolElement(p,8,12) {}
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual void draw_out_face( int );
    virtual SurfaceElement* cut(char *,GLfloat*,Interpolator<DATA_TYPE>*&,int);
    const int* iso_polys( unsigned int );
    virtual int surfaces(int **, int v=0);
};

class Pyramid : public VolElement
{
  public:
    Pyramid(Point *p ): VolElement(p,5,8) {}
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual void     draw_out_face( int );
    virtual SurfaceElement* cut(char*, GLfloat*, Interpolator<DATA_TYPE>*&,int);
    const int* iso_polys( unsigned int );
    virtual int surfaces(int **, int v=0);
};
#endif

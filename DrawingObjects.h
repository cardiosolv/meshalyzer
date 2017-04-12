#ifndef DRAWINGOBJECT_H
#define DRAWINGOBJECT_H

#include "objects.h"
#include <vector>
#include "drawgl.h"
#include "Colourscale.h"
#include "DataOpacity.h"
#include <zlib.h>
#include "Interpolator.h"
#include "DataAllInMem.h"
#include "ThreadedData.h"
#ifdef USE_HDF5
#include <ch5/ch5.h>
#endif

gzFile openFile( const char*, const char* );

#define MAX_NUM_SURF 6          // per element
#define MAX_NUM_SURF_NODES 4    // per element

class DrawingObj
{
  public:
    DrawingObj() {}
    virtual ~DrawingObj() {}

    //! draw single objs
    virtual void   draw( int, GLfloat*, float size=1 )=0;
    //! draw several objs
    virtual void   draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                         int stride=1, dataOpac* dopac=NULL )=0;
    virtual bool   read( const char * ) = 0;
    int    num() const { return _n; }		//!< get \#objects

    void   translucency( bool );            //!< set tranlucency
    void   size( float s ){ _size = s; }
    float  size( void ){ return _size; }
    void   threeD( bool b ){ _3D = b; }
    bool   threeD( void ){ return _3D; }
  protected:
    int   _n = 0;			                    //!< \# objects
    float _size = 1;                            //!< size to draw objects
    bool  _3D = false;
};


class PPoint: public DrawingObj
{
  public:
    PPoint() : _pts(NULL), _base1(false),_dynPt(NULL), _tm(-1) {}
    virtual ~PPoint() { if ( _pts ) free(_pts); _pts = 0; }

    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
#ifdef USE_HDF5
    virtual bool     read( hid_t );
#endif
    void    register_vertex( int, vector<bool>& );
    const   GLfloat* pt( int p=0 ){ return _pts+p*3; }
    const   GLfloat* pt( int p ) const { return _pts+p*3; }
    void    setVis( bool v ){ if (v) _visible=&_allvis; }
    void    setVis( vector<bool>* v ){ _visible=v; }
    bool    vis( int n ) const { return (*_visible)[n]; }
    bool    vis( int n, bool b ) const { (*_visible)[n]=b; }
    const   vector<bool>* vis() const { return _visible; }
    const   GLfloat* offset() const { return _offset; }
    void    offset( const GLfloat*o )  { memcpy(_offset,o,sizeof(GLfloat)*3); }
    void    base1(bool b){ _base1 = b; }
    void    add( GLfloat *, int n=1 );
    const   GLfloat* operator[] (int i){ return _pts+3*i; }
    void    time( int a );
    int     time(){ return _tm; }
    int     dynamic( const char *, int );
    int     num_tm(){ return _dynPt? _dynPt->max_tm()+1:0; }
  private:
    GLfloat*     _pts;		  //!< point list
    vector<bool>*_visible;    //!< points which get drawn
    GLfloat      _offset[3];  //!< centering offset
    bool         _base1;      //!< true for base 1
    vector<bool>_allvis;	  //!< all true
    int         _maxtm;       //!< maximum time
    int         _tm;          //!< current time
    DataClass<float>* _dynPt; //!< dynamic point data
};


class MultiPoint : public DrawingObj
{
  public:
    MultiPoint( PPoint *p, int n, int e ):_pt(p),_ptsPerObj(n),_node(NULL), _nedge(e) {}
    virtual ~MultiPoint() { if ( _node ) delete[] _node; _node = 0; }

    const int* obj( int n=0 ) { return _node+n*_ptsPerObj; }
    int     ptsPerObj(){ return _ptsPerObj; }
    void    register_vertices(int, int, vector<bool>& );
    void    add( int *n );
    const   PPoint* pt(){ return _pt; }
    void    define( const int *nl, int n=1 );
    MultiPoint **isosurf( DATA_TYPE *d, DATA_TYPE val, int &, 
                                    vector<Interpolator<DATA_TYPE>*> *a=NULL );
    virtual const int*iso_polys(unsigned int)=0; 
    virtual int bytes()=0;
  protected:
    int *  _node;			//!< list of nodes defining objects
    PPoint* _pt;             //!< pointer to point list
    int    _ptsPerObj;		//!< \#nodes to define one object
    int    _nedge;          //!< \#edges
};


class Connection : public MultiPoint
{
  public:
    Connection(PPoint *p):MultiPoint(p,2,1) {}
    virtual ~Connection() {}

    void add( int, int );	//!< add a connection
    void add( int, int* );   //!< add connections
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual DrawingObj *isosurf( DATA_TYPE *d, DATA_TYPE val ){}
    virtual bool     read( const char * );
    virtual int      bytes(){return sizeof(Connection);}
#ifdef USE_HDF5
    virtual bool     read( hid_t hdf_file );
#endif
    const int *iso_polys(unsigned int index){return NULL;}
};


//! Cable of Contiguous points
class ContCable : public MultiPoint
{
  public:
    ContCable(PPoint *p):MultiPoint(p,1,1) {}
    virtual ~ContCable() {}

    const int* obj( int n=0 ) { return _node+n; }
    void add( int, int );
    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual int      bytes(){ return sizeof(ContCable);}
    
#ifdef USE_HDF5
    virtual bool     read( hid_t hdf_file );
#endif
    void             register_vertices( int, int, vector<bool>& );
    const int *iso_polys(unsigned int index){return NULL;}
};


// closed convex polygons
class SurfaceElement : public MultiPoint
{
  public:
    SurfaceElement(PPoint *p, int n):MultiPoint(p,n,n),_nrml(NULL),_ptnrml(NULL) {}
    virtual ~SurfaceElement(){if(_nrml) delete[] _nrml;if(_ptnrml)delete[] _ptnrml;}

    virtual void     compute_normals( int, int )=0;
    const   void     nrml( GLfloat *n ){ _nrml=n; };
  inline  const   GLfloat* nrml( int a=0 ) {return _nrml==NULL?NULL:_nrml+3*a; }
  inline  const   GLfloat* ptnrml( int a=0 ) {return _ptnrml==NULL?NULL:_ptnrml+3*a; }
    virtual void     draw( int, GLfloat*, float=1 )=0;
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride, dataOpac* dopac, const GLfloat * )=0;
    virtual void     draw( int, GLfloat*, Colourscale*, DATA_TYPE*,
                         dataOpac* dopa, const GLfloat *, bool )=0;
    void     read_normals( int, int, const char * );
    void     vertnorm( GLfloat *a ){ _ptnrml=a; }
    
  protected:
    GLfloat* _nrml;
    GLfloat* _ptnrml;
};

class PolyGon : public SurfaceElement
{
  public:
    PolyGon( PPoint *p, int n ) : SurfaceElement(p,n) {}
    virtual ~PolyGon() {}

    virtual void compute_normals( int a, int b );
    virtual void draw( int, GLfloat*, float size=1 );
    virtual void draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride, dataOpac* dopac, const GLfloat * );
    virtual void draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual void draw( int, GLfloat*, Colourscale*, DATA_TYPE*,
                           dataOpac* dopac, const GLfloat *, bool lightson ){}
    virtual bool read( const char * ){}
	static const int  _zero;
    const int* iso_polys(unsigned int index){return &_zero;}
    int     bytes(){ return sizeof(PolyGon);}
};


class Triangle : public SurfaceElement
{
  public:
    Triangle(PPoint *p):SurfaceElement(p,3) {}
    virtual ~Triangle() {}

    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride, dataOpac* dopac, const GLfloat *);
    virtual void draw( int, GLfloat*, Colourscale*, DATA_TYPE*,
                           dataOpac* dopac, const GLfloat *, bool lightson );
    virtual bool     read( const char * );
    virtual void     compute_normals( int, int );
    virtual DrawingObj *isosurf( DATA_TYPE *d, DATA_TYPE val ){}
    bool     add( const char * );
    const int*       iso_polys(unsigned int);
    int     bytes(){ return sizeof(Triangle);}
  protected:
    int         countInFile( const char * );
};


class Quadrilateral : public SurfaceElement
{
  public:
    Quadrilateral(PPoint *p):SurfaceElement(p,4) {}
    virtual ~Quadrilateral() {}

    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride, dataOpac* dopac, const GLfloat *);
    virtual void draw( int, GLfloat*, Colourscale*, DATA_TYPE*,
                           dataOpac* dopac, const GLfloat *, bool lightson );
    virtual bool     read( const char * );
    virtual void     compute_normals( int, int );
    bool     add( const char * );
    const int*       iso_polys(unsigned int);
    int     bytes(){ return sizeof(Quadrilateral);}
  protected:
    int         countTrisInFile( const char * );
};


//! Volume elements
class VolElement : public MultiPoint
{
  public:
    VolElement( PPoint *p, int n, int e ):MultiPoint( p, n, e ) {}
    virtual ~VolElement() {}

    const   int* region() const { return _region; }
    int  region(int a) const { return _region[a]; }
    void region(int a, int b) { _region[a] = b; }
    void add( int *n, int r=-1 );
    virtual void draw_out_face( int )=0;
    virtual SurfaceElement *cut( char*, GLfloat*, Interpolator<DATA_TYPE>*&, int e=0 )=0;
    virtual int surfaces(int [][MAX_NUM_SURF_NODES+1], int v=0) = 0;
  protected:
    int*        _region;		//!< region for each element
    Connection* _edges;         //!< egdes for drawing elements
    SurfaceElement* planecut( char*, GLfloat*, Interpolator<DATA_TYPE>*&, int, const int [][2], int e  );
    int make_surf_nodelist(int, int[][MAX_NUM_SURF_NODES+1], const int, int, const int **);
};


class Tetrahedral : public VolElement
{
  public:
    Tetrahedral(PPoint *p ): VolElement(p,4,6) {}
    virtual ~Tetrahedral() {}

    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual void     draw_out_face( int );
    virtual SurfaceElement* cut(char*,GLfloat*,Interpolator<DATA_TYPE>*&,int=0);
    const int* iso_polys( unsigned int );
    virtual int     bytes(){ return sizeof(Tetrahedral);}
    virtual int surfaces(int [][MAX_NUM_SURF_NODES+1], int v=0);
};

class Prism : public VolElement
{
  public:
    Prism(PPoint *p ): VolElement(p,6,9) {}
    virtual ~Prism() {}

    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual void draw_out_face( int );
    virtual SurfaceElement* cut(char *,GLfloat*,Interpolator<DATA_TYPE>*&,int);
    const   int*     iso_polys(unsigned int);
    virtual int surfaces(int [][MAX_NUM_SURF_NODES+1], int v=0);
    virtual int     bytes(){ return sizeof(Prism);}
};


class Hexahedron : public VolElement
{
  public:
    Hexahedron(PPoint *p ): VolElement(p,8,12) {}
    virtual ~Hexahedron() {}

    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual void draw_out_face( int );
    virtual SurfaceElement* cut(char *,GLfloat*,Interpolator<DATA_TYPE>*&,int);
    const int* iso_polys( unsigned int );
    virtual int surfaces(int [][MAX_NUM_SURF_NODES+1], int v=0);
    virtual int     bytes(){ return sizeof(Hexahedron);}
};

class Pyramid : public VolElement
{
  public:
    Pyramid(PPoint *p ): VolElement(p,5,8) {}
    virtual ~Pyramid() {}

    virtual void     draw( int, GLfloat*, float size=1 );
    virtual void     draw( int, int, GLfloat*, Colourscale*, DATA_TYPE*,
                           int stride=1, dataOpac* dopac=NULL );
    virtual bool     read( const char * );
    virtual void     draw_out_face( int );
    virtual SurfaceElement* cut(char*, GLfloat*, Interpolator<DATA_TYPE>*&,int);
    const int* iso_polys( unsigned int );
    virtual int surfaces(int [][MAX_NUM_SURF_NODES+1], int v=0);
    virtual int      bytes(){ return sizeof(Pyramid);}
};

#endif

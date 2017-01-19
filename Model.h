#ifndef MODEL_H
#define MODEL_H
#include "DrawingObjects.h"
#include <string>
#include <cstring>
#include <vector>
#include <zlib.h>
#include "drawgl.h"
#include "Colourscale.h"
#include "IGBheader.h"
#include "HiLiteWinInfo.h"
#include "Surfaces.h"
#include "Region.h"
#ifdef USE_HDF5
#include <ch5/ch5.h>
#endif


class DataOpacity;

class Model
{
  public:
    Model();
    ~Model();
    bool         read(const char *fn, bool base1, bool no_elems);
#ifdef USE_HDF5
    bool         read(hid_t hdf_file, bool base1, bool no_elems);
    bool         read_instance( hid_t, unsigned int, unsigned int, float *& );
#endif
    bool         read_instance( gzFile, gzFile );
    int          add_surface_from_tri( const char * );
    int          add_surface_from_surf( const char * );
    int          add_surface_from_elem( const char *fn );
    int          add_region_surfaces( void );
    int          add_cnnx_from_elem( string );
    inline int   reg_first( int s, Object_t t ){return _region[s]->first(t); }
    Surfaces* surface(int s){ return _surface[s]; }
	void      surfKill( int s );
    RRegion*   region(int s){ return _region[s]; }
    const GLfloat*  pts(int t=0)const {return pt.pt(0);} // in future, pts move
    int    number( Object_t );
    const int*   volEle(int a=0)   const { return _vol[a]->obj(0); }
    int    numVol(){ return _numVol; }
    SurfaceElement*  element(int s,int a=0) const{ return _surface[s]->ele(a); }
    const int*   cable(int a=0)   const { return _cable->obj(a); }
    const int*   cnnx(int a=0)    const { return _cnnx->obj(a); }
    float  maxdim()         const { return _maxdim; }
    void   draw_tet( int, bool, DATA_TYPE* );
    void   hilight( HiLiteInfoWin*, int * );
    void   showobj( Object_t obj, bool *, bool f );
    bool   showobj( Object_t o, int s ) {return _region[s<0?0:s]->show(o);}
    inline bool visibility( int s ){return _region[s<0?0:s]->visible();}
    inline GLfloat opacity( int s )
                         { return (_region[s<0?0:s]->get_color(Surface))[3]; }
    GLfloat* get_color( Object_t obj, int s=0 );
    void  set_color( Object_t obj, int s, float r, float g, float b, float a );
    void  visibility( int, bool a );
    void  opacity( int s, float opac );
    void  randomize_color( Object_t obj );
    void  hilight_info( HiLiteInfoWin*, int*, DATA_TYPE *d=NULL );
    void  stride( Object_t o, int s ){_outstride[o]=s;}
    int   stride( Object_t o ) const {return _outstride[o]; }
    const GLfloat* pt_offset() const {return pt.offset();}
    bool  base1() const {return _base1; }
    const GLfloat* vertex_normals(Surfaces*);
    int   maxtm(){ return _numtm-1 ; }
    void  threeD( Object_t o, int r, bool b ){ _region[r<0?0:r]->threeD(o,b); }
    bool  threeD( Object_t o, int r ){ return _region[r<0?0:r]->threeD(o); }
    void  size( Object_t o, int r, float s ){ _region[r<0?0:r]->size(o, s); }
    float size( Object_t o, int r ){ return _region[r<0?0:r]->size(o); }
    string file()const{return _file;}
    bool  twoD(){return _2D; }

    PPoint             pt;
    Connection*      _cnnx=NULL;
    ContCable*       _cable=NULL;
    VolElement**     _vol=NULL;
    int              _numVol=0;
    int               numSurf(void){return _surface.size();}
    int              _numReg=0;
    int               localElemnum(int, int&);
    int               globalElemnum(int, int);
    MultiPoint*      _elems;
  private:
    void             read_region_file( gzFile, const char * );
    void             read_normals( gzFile, const char * );
    int             _outstride[maxobject];// stride to use when outputting
    void             find_max_dim_and_bounds();
    void             determine_regions();
    bool             read_elem_file(const char *);
    void             increase_ele( int );
    bool             check_element( SurfaceElement *e );
#ifdef USE_HDF5
    bool             read_elements(hid_t);
    bool             add_elements(hid_t hdf_file);
    void             add_regions(hid_t hdf_file);
    void             add_surfaces(hid_t hdf_file);
    void             add_surfaces(int *elements, int count, int max_width, char *name);
#endif
    
    RRegion**        _region=NULL;
    vector<Surfaces*> _surface;
    float           _maxdim;		     // maximum physical dimension
    bool            _base1=false;  	     //whether node numbering starts at 1
    GLfloat*        _vertnrml=NULL;		 //!< vertex normals
    vector<bool>     allvis;
    int             _numtm;
    int              new_region_label();
    string          _file;               //!< base file name
    bool            _2D=false;
};

#endif

#ifndef TBmeshWin_h
#define TBmeshWin_h
#include "drawgl.h"
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include "Colourscale.h"
#include <zlib.h>
#include "trimesh.h"
#include "ClipPlane.h"
#include "DataAllInMem.h"
#include "ThreadedData.h"
#include "VecData.h"
#include "AuxGrid.h"
#include "Model.h"
#include "Sequence.h"
#include "CutSurfaces.h"
#include "Interpolator.h"
#include "plottingwin.h"
#include "isosurf.h"
#include "IsoSurface.h"
#include "IsoLines.h"
#include "TimeLink.h"
#include "DeadDataGUI.h"
#ifdef USE_HDF5
#include "hdf5.h"
#endif


// for displaying data
const unsigned int Vertex_flg=1;
const unsigned int Cable_flg=2;
const unsigned int Cnnx_flg=4;
const unsigned int SurfEdge_flg=8;
const unsigned int Surface_flg=16;
const unsigned int VolEle_flg=32;
const unsigned int ObjFlg[] = { Vertex_flg, Cable_flg, Cnnx_flg, SurfEdge_flg, Surface_flg, VolEle_flg };

class Sequence;
class HiLiteInfoWin;
class Controls;
class DataOpacity;
class ClipPlane;
class IsosurfControl;

enum GridType { ScalarDataGrid, VecDataGrid, AuxDataGrid, DynPtGrid, NoDataGrid };

class TBmeshWin:public Fl_Gl_Tb_Window
{
  private:
    HiLiteInfoWin* hinfo;			     // window with highlight details
    bool           fill_assc_obj = true; // fill associated object
    Controls*      contwin;
    Fl_Window*     flwin;
    string         flwintitle;
  public:
    TBmeshWin(int x, int y, int w, int h, const char *l = 0);
    virtual ~TBmeshWin();

    Model*         model;
    virtual void draw();
    virtual int handle( int );
    void read_model(Fl_Window*, const char *fn, bool no_elems, bool base1=false);
#ifdef USE_HDF5
    void read_model(Fl_Window*, hid_t hdf_file, bool no_elems, bool base1=false);
#endif
    void highlight(Object_t obj, int a);
    inline void set_hilight( bool a ){ hilighton=a; redraw(); }
    inline void dispmode( Display_t d ){ disp = d; redraw(); }
    inline Display_t dispmode(void){ return disp; }
    void showobj( Object_t o, bool *r, bool a ){model->showobj(o,r,a);redraw();}
    inline bool showobj( Object_t o, int s ){ return model->showobj(o,s);}
    GLfloat* get_color( Object_t obj, int s );
    void set_color( Object_t, int, float, float, float, float );
    void visibility( bool *, bool a );
    inline bool visibility( int s ){return model->visibility(s);}
    void opacity( int s, float opac );
    inline GLfloat opacity( int s ){return model->opacity(s);}
    inline unsigned int datify(){ return datadst; }
    inline void datify( Object_t obj ){ if(obj==All)datadst=(1<<maxobject)-1;else datadst |= ObjFlg[obj]; redraw(); }
    inline void undatify( Object_t obj ){if(obj==All)datadst=0;else datadst&=~ObjFlg[obj];redraw();}
    Colourscale *cs;
    int add_surface( const char * );   	// add a surface
    int get_data( const char *, Myslider* mslider=NULL );	// get data file
    inline void bgd( float w ){bc[0]=bc[1]=bc[2]=w;valid(0);redraw();}
    const GLfloat* bgd(){return bc;}
    void optimize_cs( void );		// calibrate colour scale
    void randomize_color( Object_t );	// randomize surface colours
    inline float get_maxdim(void){ return model->maxdim(); }
    inline double vertex_val( int a ){ if (have_data!=NoData) return data[a]; 
                                                               else return 0;}
    inline void  assc_obj( Object_t o, bool fo=false )  { vert_asc_obj=o; 
                                           fill_assc_obj=fo; redraw(); }
    inline int reg_first( int s, Object_t t ) { return  model->reg_first( s,t ); }
    void hiliteinfo();
    void select_hi( int );
    void controlwin( Controls *c ){ contwin = c; }
    void solid_hitet( bool a ){ fill_hitet = a; }
    void stride( Object_t t, int s ){ if (s) {model->stride(t, s);redraw();} }
    int  stride( Object_t t ){ return model->stride(t); }
    void output_png( const char *, Sequence *sq=NULL );
    void output_pdf( char *fn, bool PS );
    void revdraworder( bool a ){ revDrawOrder=a; redraw(); }
    void animate_delay(float a){ frame_delay = a; }
    void animate_skip( int a, void *, bool = false );
    bool set_time(int a);
    int  time(){return tm;}
    void autocolour( bool a ){autocol = a;}
    void lights( bool a ){lightson = a;redraw();}
    friend void animate_cb( void *v );
    DataOpacity *dataopac;			// data opacity
    ClipPlane* cplane;
    int       getVecData(void *, const char *vptsfile=NULL);
    VecData  *vecdata = NULL;
    void      select_vertex();
    PlotWin  *timeplotter;
    void      timeplot();
    bool      recording = false;			   // true if recording events
    void      record_events( char * ); // output the frame buffer after each change
    void      dump_vertices();
    void      compute_normals();
    void      region_vis( int *, int, bool *);
    friend    class Sequence;
    void      facetshade( bool a ){ facetshading=a;valid(0);redraw(); }
    void      cull( bool a ){ backface_culling=a;valid(0);redraw(); }
    void      bf_light( int a ){ lit_bf=a;valid(0);redraw(); }
    void      headlamp( bool a ){ headlamp_mode=a; redraw(); }
    void      surfVis( vector<int>&, bool );
    void      surfFilled( vector<int>&, bool );
    void      surfOutline( vector<int>&, bool );
    void      surfOutColor( vector<int>&, GLfloat * );
    void      surfFillColor( vector<int>&, GLfloat *);
    void      determine_cutplane( int cp );
    void      threeD( Object_t o, int r, bool b ){model->threeD(o,r,b); redraw();}
    bool      threeD( Object_t o, int r ){ return model->threeD(o,r); }
    void      size( Object_t o, int r, float b ){model->size(o,r,b); redraw();}
    float     size( Object_t o, int r ){ return model->size(o,r); }
    IsosurfControl *isosurfwin;
    int       readAuxGrid( void *, const char* agfile );
    AuxGrid  *auxGrid = NULL;
    TimeLink *tmLink;
    void      signal_links( int );
    void      transBgd( bool a ){ bgd_trans=a;valid(0); }
    bool      transBgd( ){ return bgd_trans; }
    void      norot(bool a){_norot=a;}
    void      CheckMessageQueue();
    int       ProcessLinkMessage(const LinkMessage::CmdMsg& msg);
    void      SendViewportSyncMessage();
    void      SendTimeSyncMessage();
    void      SendColourSyncMessage();
    void      SendClipSyncMessage();
    int       read_dynamic_pts( const char *, Myslider * );
    bool      compat_tm( int t ){ return (t==1 || max_time()==0 || max_time()==t-1); }
    int       max_time( GridType g=NoDataGrid );
    void      forceThreadData(bool a){forcedThreaded=a;}
    void      branch_cut(double,double,float);
    void      ctr_on_vtx( int vtx );
    bool      anim_loop = false;
    void      axes( int v ) { _axes = v; redraw(); }
    void      saveAux( char *fn, int s );
    friend class Controls;
    friend class HDF5DataBrowser;
    friend class Frame;
    friend class ObjProps;
    friend class colourChoice;
    friend class IsosurfControl;
        
  private:
    int        hilight[maxobject];	// which object to highlight
    bool	   hilighton = false;	// whether to highlight
    Display_t  disp = asSurface;	// type of display
    unsigned int   datadst;			// which object gets data coloured
    GLfloat tet_color[4]     = {1.,1.,1.,1.},
            hitet_color[4]   = {1.,0.,0.,1.},
            hiele_color[4]   = {0.,0.,1.,1.},
            hicable_color[4] = {0.,1.,1.,1.},
            hicnnx_color[4]  = {1.,1.,0.,1.}, 
            hipt_color[4]    = {1.,0.,1.,1.},
            hiptobj_color[4] = {1.,0.8,0.,1.},
            bc[4];
    DataClass<DATA_TYPE>*  dataBuffer = NULL;
    DATA_TYPE* data = NULL;			    // data to display
    Data_Type_t  have_data = NoData;    // true if data file has been read
    GLfloat*   colourize( float );	    // set GL colour for data value
    Object_t   vert_asc_obj = SurfEle;	// object to draw with vertex
    double     solid_angle3Dpt( int v, int a, int b, int c );
    bool	   fill_hitet =  false;     // true to fill hilighted tetrahedron
    bool       revDrawOrder = false;    // draw surfaces in reverse orderc++ convert int to string
    float      frame_delay = 0.01;      // delay between frames
    int        frame_skip = 0;     	    // direction and #frames to skip
    int        tm = 0;
    int        numframes = 0;
    bool	   autocol = false;		// set colour range every frame

    bool	  lightson = true;				// light or not
    void      illuminate(GLfloat);			// light the model
    bool      draw_surfaces(Surfaces *, short);
    void      draw_elements(Surfaces *);
    void      draw_sorted_elements( vector<vtx_z> &elems );
    void      draw_cables(RRegion *);
    void      draw_cnnx(RRegion *);
    void      draw_vertices(RRegion *);
    void      draw_axes(const GLfloat *);
    void      draw_clip_plane( int cp );
    void      draw_cut_planes( RRegion * );
    void      draw_iso_surfaces( );
    void      draw_iso_lines();
    GLenum    renderMode = GL_RENDER;// mode for drawing
    vector<bool> ptDrawn;			 // was a point drawn
    vector<bool> ptVisible;          // is a point is a visible region?
    void      register_vertex( int i ); // for picking
    vector<GLuint> hitbuffer;
    void      process_hits();		// determine picked node
    DATA_TYPE* timevec = NULL;
    unsigned long framenum;			// keep track if a frame is drawn
    bool      dump_vert_list = false;
    DataReaderEnum getReaderType( const char * );
    bool      facetshading = false;	// do not blend over surface elements
    bool      backface_culling = false;	// do not cull CW elements
    int       lit_bf = 0; // ligth backface?
    bool      headlamp_mode = true;	// headlamp lighting mode
    CutSurfaces **_cutsurface;      // clipped surfaces
    IsoLine    *isoline = NULL;
    set<int>   timeLinks;           // other processes linked to this one
    bool       bgd_trans = false;   //!< transparent background
    bool       _norot = false;      //!< allow rotations
    float      _dt;                 //!< increment between time slices
    void       set_windows( Fl_Window*, const char * );
    bool      forcedThreaded = false;  // force threaded reading of data
    // constants
    static unsigned int MAX_MESSAGES_READ;
    bool   _branch_cut = false;
    double _branch_range[2];
    DeadDataGUI *deadData;
    bool   _axes=false;
    bool   _eleBasedData=false;
};

#include "DataOpacity.h"
#endif

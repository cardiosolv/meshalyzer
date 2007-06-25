#ifndef TBmeshWin_h
#define TBmeshWin_h
#include "drawgl.h"
#include <string>
#include <cstring>
#include <vector>
#include "Colourscale.h"
#include <zlib.h>
#include "trimesh.h"
#include "ClipPlane.h"
#include "DataAllInMem.h"
#include "ThreadedData.h"
#include "VecData.h"
#include "Model.h"
#include "Sequence.h"
#include "CutSurfaces.h"
#include "Interpolator.h"
#include "plottingwin.h"

class Sequence;
class HiLiteInfoWin;
class Controls;
class DataOpacity;
class ClipPlane;

class TBmeshWin:public Fl_Gl_Tb_Window {
	private:
		HiLiteInfoWin* hinfo;			// window with highlight details
		bool           fill_assc_obj;	// fill associated object
		Controls*      contwin;
		Fl_Window*     flwin;
		string         flwintitle;
	public:
        TBmeshWin(int x, int y, int w, int h, const char *l = 0);
		Model*         model;
		virtual void draw();
		virtual int handle( int );
		void read_model( Fl_Window*, const char *fn=0, bool base1=false);
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
		inline Object_t datify(){ return datadst; }
		inline void datify( Object_t obj ){ datadst=obj; redraw(); }
		Colourscale *cs;
		int add_surface( const char * );   	// add a surface
		void get_data( const char *, Myslider* mslider=NULL );	// get data file
		inline void bgd( float w ){bc[0]=bc[1]=bc[2]=w;valid(0);redraw();}
		const GLfloat* bgd(){return bc;}
		void optimize_cs( void );		// calibrate colour scale
		void randomize_color( Object_t );	// randomize surface colours
		inline float get_maxdim(void){ return model->maxdim(); }
		inline double vertex_val( int a ){ if(have_data!=NoData) return data[a]; else return 0;}
		inline void  assc_obj( Object_t o, bool fo=false ) 
			{ vert_asc_obj=o; fill_assc_obj=fo; redraw(); }
		inline int reg_first( int s, Object_t t ){
		  								return  model->reg_first( s,t ); }
		void hiliteinfo();	
		void select_hi( int );
		void controlwin( Controls *c ){ contwin = c; }
		void solid_hitet( bool a ){ fill_hitet = a; }
		void stride( Object_t t, int stride ) {
		  				if(stride){model->stride(t, stride);redraw();} }
		void output_png( const char *, Sequence *sq=NULL );
		void output_pdf( char *fn, bool PS );
		void revdraworder( bool a ){ revDrawOrder=a; redraw(); }
		void animate_delay(float a){ frame_delay = a; }
		void animate_skip( int a, void * );
		void set_time(int a){tm=a;if(timeplotter!=NULL)timeplotter->highlight(tm);redraw();}
		void autocolour( bool a ){autocol = a;}
		void lights( bool a ){lightson = a;redraw();}
		friend void animate_cb( void *v );
		DataOpacity *dataopac;			// data opacity
		ClipPlane* cplane;
		int  getVecData(void *, char *vptsfile=NULL);
		VecData  *vecdata;
		void select_vertex();
		PlotWin  *timeplotter;
		void      timeplot();
		bool      recording;			   // true if recording events
		void      record_events( char * ); // output the frame buffer after each change
        void      dump_vertices();
		void      compute_normals();
		void      region_vis( int *, int, bool *);
		friend    class Sequence;
		void      facetshade( bool a ){ facetshading=a;valid(0);redraw(); }
		void      headlamp( bool a ){ headlamp_mode=a; redraw(); };

		void      surfVis( int, bool );
		void      surfFilled( int, bool );
		void      surfOutline( int, bool );
		void      surfOutColor( int, GLfloat * );
		void      surfFillColor( int, GLfloat *);
        void      determine_cutplane( int cp );
	private:
		int        hilight[maxobject];	// which object to highlight
		bool	   hilighton;			// whether to highlight
		Display_t  disp;					// type of display
		Object_t   datadst;				// which object gets data coloured
		GLfloat    tet_color[4], hitet_color[4], hiele_color[4],
				   hicable_color[4], hicnnx_color[4], hipt_color[4], bc[4],
				   hiptobj_color[4]; 
		DataClass<DATA_TYPE>*  dataBuffer;
		DATA_TYPE* data;				// data to display
		Data_Type_t  have_data;			// true if data file has been read
		GLfloat*   colourize( float );	// set GL colour for data value
		Object_t   vert_asc_obj;			// object to draw with vertex
		double     solid_angle3Dpt( int v, int a, int b, int c );
		bool	   fill_hitet;			// true to fill hilighted tetrahedron
		bool       revDrawOrder;			// draw surfaces in reverse order
		float      frame_delay;			// delay between frames
		int        frame_skip;         	// direction and #frames to skip
		int        tm;
		int        numframes;
		bool	   autocol;				// set colour range every frame

		bool	  lightson;				// light or not
		void      illuminate(GLfloat);			// light the model
		void      draw_surfaces(Surfaces *);
		void      draw_elements(Surfaces *);
		void      draw_cables(Region *);
		void      draw_cnnx(Region *);
		void      draw_vertices(Region *);
		void      draw_axes();
		void      draw_clip_plane( int cp );
		void      draw_cut_planes( Region * );
		int       max_time();
		GLenum    renderMode;			// mode for drawing
		vector<bool> ptDrawn;			// was a point drawn
		vector<bool> ptVisible;         // is a point is a visible region?
        void      register_vertex( int i ); // for picking
		GLuint*   hitbuffer;
		void      process_hits();		// determine picked node
		DATA_TYPE* timevec;
		unsigned long framenum;			// keep track if a frame is drawn
		bool      dump_vert_list;
        DataReaderEnum getReaderType( const char * );
		bool      facetshading;			// do not blend over surface elements
		bool      headlamp_mode;		// headlamp lighting mode
		CutSurfaces **_cutsurface;      // clipped surfaces
};

#include "DataOpacity.h"
#endif

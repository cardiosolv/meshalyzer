#include "trimesh.h"
#include <signal.h>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Progress.H>
#include <fstream>
#include <sstream>
#include <set>
#include <algorithm>
#include <vector>
#ifdef __WIN32__
#  include <GL/glext.h>
#endif
#ifdef HAVE_GL2PS
#include "gl2ps.h"
#endif
#ifdef USE_HDF5
#include <ch5/ch5.h>
#endif
#include "Frame.h"

#define HITBUFSIZE   10000
#define OPAQUE_LIMIT 0.95       //!< consider opaque if alpha level above this
#define MAX_SURFELE_REALTIME 400000 //!< max \#ele's to draw while moving

unsigned int TBmeshWin::MAX_MESSAGES_READ = 100;

const int   NUM_CP = 6;
const GLenum CLIP_PLANE[] =
  {
    GL_CLIP_PLANE0, GL_CLIP_PLANE1, GL_CLIP_PLANE2,
    GL_CLIP_PLANE3, GL_CLIP_PLANE4, GL_CLIP_PLANE5
  };

bool translucency( bool b );

void animate_cb( void *v )
{
  TBmeshWin* mw = (TBmeshWin *)(((void **)v)[0]);
  Controls* ctrl = (Controls *)(((void **)v)[1]);
  if ( mw->frame_skip == 0 )
    return;

  int ntm = mw->tm + mw->frame_skip;
  if ( !mw->anim_loop && (ntm<0 || ntm>ctrl->tmslider->maximum()) ) {
    ntm -= mw->frame_skip;
    mw->frame_skip = 0;
  } else {
    if ( ntm<0 ) 
      ntm = ctrl->tmslider->maximum();
    else if( ntm>ctrl->tmslider->maximum() )
      ntm = 0;

    mw->signal_links( 1 );
  }
  mw->set_time( ntm );
  Fl::add_timeout( mw->frame_delay, animate_cb, v );
}


// determine the action if the play button is pressed on the control widget
void
TBmeshWin::animate_skip( int fs, void *ctrl, bool repeat )
{
  static void* vp[2];

  if ( fs && dataBuffer != NULL)dataBuffer->increment( fs );

  if ( frame_skip ) {
    frame_skip = fs;
    return;
  }

  if ( ctrl==NULL )
    return;

  vp[0] = this;
  vp[1] = ctrl;
  frame_skip = fs;
  anim_loop = repeat;

  if ( tm+frame_skip >= contwin->tmslider->maximum() )
    tm = -frame_skip;
  else if ( tm+frame_skip < 0 )
    tm = int(contwin->tmslider->maximum())-frame_skip;
  animate_cb( &vp );
}


void TBmeshWin::highlight( Object_t obj, int a )
{
  hilight[obj] = a;
  redraw();
  if ( hinfo->window->visible() ) hiliteinfo();
  if ( obj == Vertex ) {
    if( timeplotter->window->shown() ) timeplot();
    if( timeplotter->graph->crvi_vis() )static_curve_info_cb(timeplotter->graph);
    if( have_data != NoData) contwin->vertvalout->value( data[a] );
  }
}


TBmeshWin ::TBmeshWin(int x, int y, int w, int h, const char *l )
    : Fl_Gl_Tb_Window(x, y, w, h, l), datadst(ObjFlg[Surface]), cs( new Colourscale( 64 ) ),
    hinfo(new HiLiteInfoWin(this)), dataopac(new DataOpacity( this )),
    cplane(new ClipPlane( this )), hitbuffer(HITBUFSIZE),
    timeplotter(new PlotWin("Time series")), _cutsurface(new CutSurfaces*[NUM_CP] ),
    isosurfwin(new IsosurfControl(this)), deadData(new DeadDataGUI(this))
{
  model = new Model();
  memset( hilight, 0, sizeof(int)*maxobject );
  bgd( 1. );
  for ( int i=0; i<NUM_CP; i++ ) _cutsurface[i]=NULL;

  tmLink = new TimeLink( timeLinks );
}

TBmeshWin::~TBmeshWin()
{
  if (tmLink != NULL){
    delete tmLink;
  }

  // need to delete all stack allocated objects
  if (model != NULL){
    delete model;
  }

  if (iso0 != NULL){
    delete iso0;
  }

  if (iso1 != NULL){
    delete iso1;
  }
}

void TBmeshWin :: draw()
{
  if (!valid()) {
    valid(1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPointSize( 10.0 );
    glLineWidth( 1. );
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glPolygonMode( GL_FRONT, GL_LINE );
    glPolygonOffset( 2., 2. );
    glEnable( GL_DEPTH_TEST );
    glEnable(GL_NORMALIZE);
    if ( facetshading ) {			// faster but no anti-aliasing
      glDisable( GL_POINT_SMOOTH );
      glDisable( GL_LINE_SMOOTH );
      glDisable( GL_MULTISAMPLE );
      glShadeModel(GL_FLAT);
    } else {
      glEnable( GL_POINT_SMOOTH );
      glEnable( GL_LINE_SMOOTH );
      glEnable( GL_MULTISAMPLE );
      glShadeModel(GL_SMOOTH);
    }
    glDepthFunc( GL_LEQUAL );
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#ifdef HAVE_GL2PS
    gl2psBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
    glViewport(0,0,w(),h());
    glClearColor( bc[0], bc[1], bc[2], bgd_trans?0:1 );
    if ( renderMode == GL_SELECT ) {		// for picking and vertex list
      glInitNames();
      glPushName(~(GLuint)0);
      if ( !dump_vert_list ) {
        GLint viewport[4];
        glGetIntegerv (GL_VIEWPORT, viewport);
        gluPickMatrix (Fl::event_x(),h()-1-Fl::event_y(),10.0,10.0,viewport);
      }
    }
    float maxdim = model->maxdim();
    glOrtho( -maxdim, maxdim, -maxdim, maxdim, -maxdim*10, maxdim*10 );
  }

  framenum++;

  glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
  glPushMatrix();

  if ( headlamp_mode )
    illuminate( model->maxdim() );
  trackball.DoTransform();
  if ( !headlamp_mode )
    illuminate( model->maxdim() );

  for ( int i=0; i<NUM_CP; i++ ) {
    if ( cplane->on(i) ) {
      glClipPlane( CLIP_PLANE[i], cplane->plane(i) );
      glEnable( CLIP_PLANE[i] );
    } else
      glDisable( CLIP_PLANE[i] );
  }
  cplane->setView( trackball.qRot );

  // since it is really slow to draw tetras, don't draw them if we are
  // moving the trackball
  Display_t actual_disp;
  if ( disp==asTetMesh && Fl::event_state(FL_BUTTON1|FL_BUTTON2|FL_BUTTON3) )
    actual_disp = disp; //asSurface;
  else
    actual_disp = disp;

  if ( have_data != NoData ) {
    if ( (data = dataBuffer->slice(tm)) == NULL )
      data = dataBuffer->slice(dataBuffer->max_tm());
    contwin->dispVertVal(data[hilight[Vertex]]);
  }

  if ( autocol == true ) {
    optimize_cs();
    contwin->mincolval->value(cs->min());
    contwin->maxcolval->value(cs->max());
  }

  if ( actual_disp == asTetMesh ) {

    glLineWidth(0.5);
    glColor3fv( tet_color );
    const bool datcolor = datadst&VolEle_flg && have_data!=NoData;

    for ( int r=0; r<model->_numReg; r++ ) {

      if ( !model->region(r)->visible() ) continue;

      model->pt.setVis(&model->region(r)->pt_membership());

      for ( int i=0; i<model->numVol(); i++ )
        model->_vol[i]->draw( 0, model->_vol[i]->num()-1,
                              model->region(r)->get_color(VolEle),
                              cs, datcolor?data:NULL );
    }

  } else if ( actual_disp == asSurface ) {

    /* We used to do the following:
     * if there are translucent objects, we do 2 passes through the draw loop,
     * the first time drawing opaque objects (translucent=2)
     * and the second time translucent objects (translucent=1)
     */

    // display regions
    for ( int s=0; s<model->_numReg; s++ ) {

      RRegion *reg = model->region(revDrawOrder ? model->_numReg-s-1 : s);

      if ( !reg->visible() ) continue;

      model->pt.setVis(&reg->pt_membership());

      if ( reg->show(Cable) && model->_cable->num() )
        draw_cables(reg);
      if ( reg->show(Cnnx) == true && model->_cnnx->num())
        draw_cnnx(reg);
      if ( reg->show(Vertex) )
        draw_vertices(reg);

      draw_cut_planes( reg );
    }
  }

  draw_iso_surfaces( );

  // draw surfaces
  model->pt.setVis( true );
  vector<vtx_z> trans_elems;
  for ( int s=0; s<model->numSurf(); s++ ) {

    short sindex = revDrawOrder ? model->numSurf()-s-1 : s;
    Surfaces *sf = model->surface(sindex);

    if ( !sf->visible() ) continue;

    sf->set_material();

    if ( sf->filled() && model->numSurf() ) {
      if( draw_surfaces(sf, sindex) ) {
        // merge elements into list keeping order
        int te_sz = trans_elems.size();
        trans_elems.resize( te_sz+sf->zl_sz() );
        std::copy( sf->zl_begin(), sf->zl_end(), trans_elems.begin()+te_sz );
        std::inplace_merge( trans_elems.begin(), trans_elems.begin()+te_sz, 
                    trans_elems.end(), 
                    [](const vtx_z a, const vtx_z b){return a.z<b.z;} ); 
      }
    }
    if ( sf->outline() && model->numSurf() ) {
      draw_elements(sf);
    }
  }
  draw_sorted_elements( trans_elems );

  draw_iso_lines();


  if ( hilighton ) {
    // draw highlighted tetrahedron
    if ( model->numVol() ) {
      if ( fill_hitet ) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        model->_vol[hilight[VolEle]]->draw(0,  hiptobj_color, 2 );
        model->_vol[hilight[VolEle]]->draw(0, bc );
      } else
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        model->_vol[hilight[VolEle]]->draw( 0, hitet_color, 2 );
    }
    if (model->numVol() && vert_asc_obj==VolEle ) {
      //draw volume elements associated with highlighted node

      if ( fill_assc_obj ) {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        for ( int i=0; i<model->numVol(); i++ )
          for ( int j=0; j<model->_vol[i]->ptsPerObj(); j++ )
            if ( model->_vol[i]->obj()[j] == hilight[Vertex] ) {
              GLfloat col[4] = { 1, 0, 1, 1 };
              model->_vol[i]->draw( 0, col );
              glColor3fv( hiptobj_color );
              model->_vol[i]->draw_out_face( hilight[Vertex] );
            }
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glEnable(GL_POLYGON_OFFSET_FILL );
      }
      GLfloat black[]= { 0, 0, 0, 1 };
      for ( int i=0; i<model->numVol(); i++ )
        for ( int j=0; j<model->_vol[i]->ptsPerObj(); j++ )
          if ( model->_vol[i]->obj()[j] == hilight[Vertex] ) {
            model->_vol[i]->draw( 0, fill_assc_obj?black:hiptobj_color, 2 );
          }
      glDisable(GL_POLYGON_OFFSET_FILL );

    } else if ( model->numSurf() && vert_asc_obj != Nothing &&
                (!model->numVol() || vert_asc_obj==SurfEle) ) {
      //draw elements associated with highlighted node
      for ( int s=0; s<model->numSurf(); s++ ) {
        vector<SurfaceElement*>ele = model->surface(s)->ele();
        for ( int i=0; i<model->surface(s)->num(); i++ ) {
          for ( int j=0; j<ele[i]->ptsPerObj(); j++ )
            if ( ele[i]->obj()[j] == hilight[Vertex] ) {
              if ( fill_assc_obj ) {
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                GLfloat col[4] = { 1, 0, 1, 1 };
                ele[i]->draw( 0, col );
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                glEnable(GL_POLYGON_OFFSET_FILL );
              }
              ele[i]->draw( 0, hiptobj_color );
            }
        }
      }
      glEnd();
    }
    if ( model->numSurf() ) {
      int lsurf, lele;
      lele = model->localElemnum( hilight[SurfEle], lsurf );
      model->surface(lsurf)->ele(lele)->draw( 0, hiele_color );
    }
    if ( model->_cable->num() )
      model->_cable->draw( hilight[Cable], hicable_color,  model->max_size(Cable)+2  );
    if ( model->_cnnx->num() )
      model->_cnnx->draw( hilight[Cnnx], hicnnx_color,  model->max_size(Cnnx)+2  );
    if ( model->pt.num() ) 
      model->pt.draw( hilight[Vertex], hipt_color, model->max_size(Vertex)+2 );
  }

  for (int i=0;i<NUM_CP;i++)
    if (cplane->on(i)&&cplane->visible(i)) draw_clip_plane(i);

  if ( _axes ) TBmeshWin::draw_axes( model->pt.offset() );

  glDepthMask(GL_TRUE);
#ifdef HAVE_GL2PS
  gl2psEnable(GL2PS_BLEND);
#endif
  glEnable(GL_BLEND);

  if( vecdata != NULL ) vecdata->draw(tm,model->maxdim());

  if( auxGrid) {
    auxGrid->draw( tm );
    contwin->auxhivert->maximum( auxGrid->num_vert()-1 );
    if(contwin->hiAuxVert->value()) {
      float val;
      if( auxGrid->highlight_vertex(contwin->auxhivert->value(),val,false) )
        contwin->auxvertval->value(val);
    }
  }

  glPopMatrix();
}


// draw_iso_surfaces()
void TBmeshWin::draw_iso_surfaces()
{
  if( have_data == NoData ) return;

  bool on_tr;

  if( isosurfwin->isoOn0->value() ) {
    bool dirty =  isosurfwin->issDirty(0);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    for ( int s=0; s<model->_numReg; s++ ) {
      RRegion *reg = model->region(s);
      if( reg->_iso0 && ( dirty || reg->_iso0->tm() != tm) ){
        delete iso0;
        reg->_iso0 = NULL;
      }
      if( reg->_iso0==NULL ) 
        reg->_iso0 = new IsoSurface( model, data, isosurfwin->isoval0->value(),
                reg->ele_membership(), tm, _branch_cut?_branch_range:NULL );
      reg->_iso0->color( isosurfwin->issColor(0) );
      on_tr = translucency( reg->_iso0->color()[3]<OPAQUE_LIMIT );
      reg->_iso0->draw();
      if( on_tr ) translucency( false );
    } 
    
    glPopAttrib();
  }

  if( !isosurfwin->isoOn1->value() ) return;

  bool dirty =  isosurfwin->issDirty(1);
  glPushAttrib(GL_ALL_ATTRIB_BITS);

  for ( int s=0; s<model->_numReg; s++ ) {
    RRegion *reg = model->region(s);
	if( reg->_iso1 && ( dirty || reg->_iso1->tm() != tm) ){
	  delete iso1;
	  reg->_iso1 = NULL;
	}
	if( reg->_iso1==NULL ) 
	  reg->_iso1 = new IsoSurface( model, data, isosurfwin->isoval1->value(),
			  reg->ele_membership(), tm, _branch_cut?_branch_range:NULL );
	reg->_iso1->color( isosurfwin->issColor(1) );
    on_tr = translucency( reg->_iso1->color()[3]<OPAQUE_LIMIT );
	reg->_iso1->draw();
    if( on_tr ) translucency( false );
  } 

  glPopAttrib();
}



/**
 * @brief if transparency, sort elements by z-depth
 *        if opaque, draw the surface
 *
 * @param sf     surface
 * @param sindex index into surface list of \p sf
 *
 * @return whether surface contains opaque elements
 */
bool
TBmeshWin::draw_surfaces(Surfaces* sf, short sindex)
{
  int stride = 1;

  if ( (Fl::event_button1()||Fl::event_button2()||Fl::event_button3()) &&
       Fl::focus()==flwin &&
       (sf->num()>MAX_SURFELE_REALTIME))
    stride =  (sf->num())/MAX_SURFELE_REALTIME+1;

  glPushAttrib(GL_POLYGON_BIT);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  bool showData=datadst&Surface_flg && have_data!=NoData; 

  bool on_tr = dataopac->dop[Surface].on() || 
               sf->fillcolor()[3]<OPAQUE_LIMIT;
  if( on_tr ) translucency(true);

  sf->sort( stride, on_tr, sindex );
  if( !on_tr ) 
    sf->draw( sf->fillcolor(), cs, showData?data:NULL,
 		dataopac->dop+Surface, 
		facetshading?NULL:model->vertex_normals(sf) );

  if ( on_tr ) 
    translucency(false);

  glPopAttrib();
  return on_tr;
}


/**
 * @brief Draw elements which have been sorted by z depth
 *
 * @param elems sorted element list
 */
void TBmeshWin::draw_sorted_elements( vector<vtx_z> &elems)
{
  if( !elems.size() ) return;

  glPushAttrib(GL_POLYGON_BIT);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  GLboolean lightson; 
  glGetBooleanv( GL_LIGHTING, &lightson );

  bool showData=datadst&Surface_flg && have_data!=NoData; 
  translucency( true );
  bool tris = true;
  int prev_s = -1;
  const GLfloat *vn=NULL;
  Surfaces* sf;

  glBegin(GL_TRIANGLES);
  for( auto &e : elems ) {
    if( e.s != prev_s ) {
      sf =  model->surface(e.s);
      prev_s = e.s;
      sf->set_material();
      vn = facetshading?NULL:model->vertex_normals(sf); 
    }
    sf->draw_elem( e.i, sf->fillcolor(), cs, showData?data:NULL,
 		dataopac->dop+Surface, vn, tris );
  }
  glEnd();
  translucency( false );
  glPopAttrib();
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}


// draw surface element outlines
void TBmeshWin::draw_elements(Surfaces* sf)
{
  glPushAttrib(GL_POLYGON_BIT);
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  glLineWidth(2.);

  bool datacol=true;
  if ( !(datadst&SurfEdge_flg) || have_data==NoData )
    datacol = false;

  if ( renderMode == GL_RENDER )
    sf->draw( sf->outlinecolor(), cs, datacol?data:NULL,
              dataopac->dop+SurfEle, NULL	);
  else
    sf->register_vertices( ptDrawn );

  glPopAttrib();
}


// draw cables
void TBmeshWin::draw_cables(RRegion* sf)
{
  if ( renderMode == GL_RENDER ) {
    glPushAttrib(GL_POLYGON_BIT);
    model->_cable->size( sf->size(Cable) );
    model->_cable->threeD( sf->threeD(Cable) );
    bool on_tr = translucency( dataopac->dop[Cable].on() );
    model->_cable->draw( 0, model->_cable->num()-1, sf->get_color(Cable),
                         cs, datadst&Cable_flg?data:NULL,
                         model->stride(Cable), dataopac->dop+Cable );
    if ( on_tr ) translucency(false);
    glPopAttrib();
  } else
    model->_cable->register_vertices(0,model->_cable->num()-1,ptDrawn);
}


// draw connections
void TBmeshWin::draw_cnnx(RRegion* sf)
{
  if ( renderMode == GL_RENDER ) {
    glPushAttrib(GL_POLYGON_BIT);
    model->_cnnx->size( sf->size(Cnnx) );
    model->_cnnx->threeD( sf->threeD(Cnnx) );
    glColor4fv( sf->get_color( Cnnx ) );
    model->_cnnx->draw( 0, model->_cnnx->num()-1, sf->get_color(Cnnx),
                        cs, datadst&Cnnx_flg?data:NULL, 
                        model->stride(Cnnx), dataopac->dop+Cnnx       );
    glPopAttrib();
  } else
    model->_cnnx->register_vertices( 0, model->_cnnx->num()-1, ptDrawn );
}


//draw vertices
void TBmeshWin::draw_vertices(RRegion* reg)
{

  if ( renderMode==GL_RENDER ) {
    glPushAttrib(GL_POLYGON_BIT);
    glColor4fv( reg->get_color(Vertex) );
    GLfloat opac = reg->get_color(Vertex)[3];
    model->pt.size( reg->size(Vertex) );
    model->pt.threeD( reg->threeD(Vertex) );
    if ( dataopac->dop[Vertex].on() ) translucency(true);
    model->pt.draw( 0, model->pt.num()-1, reg->get_color(Vertex),
                    cs, datadst&Vertex_flg?data:NULL,
                    model->stride(Vertex), dataopac->dop+Vertex );
    if ( dataopac->dop[Vertex].on() ) translucency(false);
    glPopAttrib();
  } else
    for ( int i=0; i<=model->pt.num(); i++ )
      if ( model->pt.vis(i) )
        model->pt.register_vertex( i, ptDrawn );
}


// draw_axes
void TBmeshWin::draw_axes(const GLfloat *offset)
{
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  for ( int i=0; i<NUM_CP; i++ ) glDisable(CLIP_PLANE[i]);
  V3f     org  = -trackball.GetTranslation();
  GLfloat size = model->maxdim()/2./trackball.GetScale();

  GLUquadricObj* quado = gluNewQuadric();
  gluQuadricDrawStyle( quado, GLU_FILL );
  gluQuadricOrientation(quado, GLU_INSIDE);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  const char *axis_label[] = { "X", "Y", "Z" };

  for ( int i=0; i< 3; i++ ) {
    glPushMatrix();
    V3f  axis( i==0, i==1, i==2 );
    V3f  zaxis( 0., 0., 1. );
    float angle = -acos( axis.Dot( zaxis ))*180./M_PI;
    V3f rotvect = axis.Cross(zaxis);
    org *= trackball.GetScale();
    //imps_list.txtglTranslatef( org.X(), org.Y(), org.Z() );
    glTranslatef( offset[0], offset[1], offset[2] );
    glPushMatrix(); // save the translation but not rotation
    glRotatef( angle, rotvect.X(), rotvect.Y(), rotvect.Z() );
    glColor3f( 0., 0., 1. );
    draw_arrow( quado, size, size/3., size/20., size/10. );
    //glColor3f( 1., 0., 1. );
    //glTranslatef( 0, 0, size/6. );
    //gluCylinder( quado, size/20., 0, size/6., 10, 2 );
    glPopMatrix();
    //glColor3f( 0., 0., 1. );
    axis *= size*1.4;			// just beyond the total; arrow length
    glRasterPos3f( axis.X(), axis.Y(), axis.Z() );
    gl_font( FL_HELVETICA_BOLD, 50 );
    gl_draw( axis_label[i] );
    glPopMatrix();
  }

  gluDeleteQuadric( quado );

  glEnable( GL_DEPTH_TEST );
  glPopAttrib();
}


const int shift_time_scale=10;

int TBmeshWin::handle( int event )
{
  if ( renderMode==GL_SELECT ) {
    if ( event==FL_PUSH ) {      //picking
      glRenderMode( renderMode );
      valid(0);
      redraw();
      Fl::flush();
      glFlush();
      process_hits();
    }
    return 1;					// ignore all other events in this mode

  } else if ( event == FL_KEYBOARD || event == FL_SHORTCUT ) {

    int newtm;
    int k=Fl::event_key();

    switch ( Fl::event_key()) {
      case FL_Right:
        newtm = int(contwin->tmslider->value()+
                    contwin->frameskip->value()*
                    ((Fl::event_state()&FL_SHIFT)?shift_time_scale:1));
        if ( newtm <= contwin->tmslider->maximum() && newtm>=0 ) {
          signal_links( 1 );
          set_time( newtm );
        }
        return 1;
        break;
      case FL_Left:     
        newtm = int(contwin->tmslider->value()-
                    contwin->frameskip->value()*
                    ((Fl::event_state()&FL_SHIFT)?shift_time_scale:1));
        if ( newtm <= contwin->tmslider->maximum() && newtm>=0 ) {
          signal_links( -1 );
          contwin->tmslider->redraw();
          set_time( newtm );
        }
        return 1;
        break;
      case 'o':   // optimize colour scale
        optimize_cs();
        contwin->mincolval->value(cs->min());
        contwin->maxcolval->value(cs->max());
        return 1;
        break;
      case 'p':     // select a vertex
        contwin->pickvert->color(128);
        contwin->pickvert->redraw();
        select_vertex();
        return 1;
        break;
      case 'r':    // reread data
        if( !dataBuffer ) return 1;
        fl_cursor( FL_CURSOR_WAIT );
        Fl::check();
        get_data( dataBuffer->file().c_str(), contwin->tmslider );
        if ( timeplotter->window->shown() ) timeplot();
        fl_cursor( FL_CURSOR_DEFAULT );
        Fl::check();
        return 1;
        break;
      case 'c':  // put controls on top of window
        contwin->window->hide();
        contwin->window->position( flwin->x_root(), flwin->y_root() );
        contwin->window->show();
        return 1;
        break;
      case 't': // time sync keyboard shortcut
		SendTimeSyncMessage();
		return 1;
		break;
      case 'v': // view port keyboard shortcut 
		SendViewportSyncMessage();
		return 1;
		break;
      default:
        return 0;
        break;
    }
  } else										    // do trackball thing
    if( !_norot || Fl::event_button()!=FL_LEFT_MOUSE  || Fl::event_state(FL_SHIFT) ||
                   Fl::event_state(FL_CTRL)                                            )
      return Fl_Gl_Tb_Window::handle(event);
}


/** set trackball and window information when reading a new model
 *  
 *  \param flwindow window identifier
 *  \param modname  model name
 */
void  TBmeshWin::set_windows( Fl_Window *flwindow, const char *modname )
{
  ptDrawn.resize(model->pt.num());

  flwin = flwindow;
  flwintitle = modname;
  flwin->label( flwintitle.c_str() );

  // set the dimensions for the trackball
  float maxdim = model->maxdim();
  const GLfloat *poff = model->pt_offset();
  trackball.mouse.SetOglPosAndSize(-maxdim, maxdim, 2*maxdim, 2*maxdim );
  trackball.size = maxdim;
  trackball.SetOrigin( -poff[0], -poff[1], -poff[2] );
  cplane->set_dim( maxdim*1.3 );

  if( model->twoD() ){ 
    _norot=true;
    contwin->norot->set();
  }
    
  disp = asSurface;
}


/** read in the geometrical description
 *
 * \param flwindow
 * \param fnt      base file to open
 * \param base1    points begin numbering at 0
 * \param no_elems do not read element file
 */
void TBmeshWin::read_model( Fl_Window *flwindow, const char* fnt, 
		bool no_elems, bool base1 )
{
  string fname = fnt;

  if ( fnt == NULL || !fname.length() ) {
      
    // if available, go to the first Model Dir directory
    char  *moddir=NULL;
    if( getenv("MESHALYZER_MODEL_DIR") ) {
      moddir = strdup( getenv("MESHALYZER_MODEL_DIR") );
      char *p = strchr( moddir, ':' );
      if( p ) *p = '\0';
    }
    
    Fl_File_Chooser modchooser( moddir, "*.pts*", Fl_File_Chooser::SINGLE, "Pick one" ); 
    modchooser.show();
    while( modchooser.shown() )
      Fl::wait();

    fname = modchooser.value();
    if (fname == "") {
      fprintf(stderr, "No file selected.  Exiting.\n");
      exit(0);
    }
  }

#ifdef USE_VTK
  if ( fname.length()>4 && fname.substr(fname.length()-4) == ".vtu" )
    {
      if ( !model->read_vtu( fname.c_str(), no_elems ) ) return;
    }
  else
#endif
    {if ( !model->read( fname.c_str(), base1, no_elems ) ) return;}

  string wintitle =  fnt;
  string::size_type  i0=wintitle.rfind("/");
  if ( i0 < string::npos ) wintitle= wintitle.substr(i0+1,string::npos);
  if (wintitle.rfind(".pts") == wintitle.size()-4 ) wintitle=wintitle.substr(0,wintitle.size()-4);

  set_windows( flwindow, wintitle.c_str() );
}


#ifdef USE_HDF5
void TBmeshWin::read_model(Fl_Window* flwindow, hid_t hdf_file, bool no_elems,
    bool base1)
{
  if (!model->read(hdf_file, base1, no_elems)) return;
  
  char *modelname;
  ch5_meta_get_name(hdf_file, &modelname);
  set_windows( flwindow, modelname );
  free(modelname);
}
#endif


/** add a surface by reading in a .tri file, also try reading a normal file
 *
 * \param file     file containing tri's
 *
 * \return \#surfaces added
 */
int TBmeshWin :: add_surface( const char *fn )
{
  int nsa; 
  if ( (nsa=model->add_surface_from_tri( fn ))<=0 )
    if( (nsa=model->add_surface_from_surf( fn )) <= 0 )
      fl_alert( "Incompatible surface found in %s", fn );
  return nsa;
}


/*
 * return color of object for surface s
 */
GLfloat* TBmeshWin:: get_color( Object_t obj, int s )
{
  if ( s<0 ) s=0;

  if ( obj == VolEle )
    return tet_color;
  else if ( obj == SurfEle && model->numSurf() )
    return model->surface(s)->outlinecolor();
  else if ( obj == Surface && model->numSurf() )
    return model->surface(s)->fillcolor();
  else
    return model->get_color(obj, s );
}


void TBmeshWin:: set_color( Object_t obj, int s, float r, float g, float b, float a )
{
  if ( obj==VolEle ) {
    tet_color[0] = r;
    tet_color[1] = g;
    tet_color[2] = b;
    tet_color[3] = a;
  } 
  model->set_color( obj, s, r, g, b, a );
}


void TBmeshWin :: visibility( bool* reg, bool a )
{
  for ( int i=0; i<model->_numReg; i++ )
    if ( reg[i] )
      model->visibility( i, a );
  // redetermine the interpolated cutting planes
  for ( int i=0; i<NUM_CP; i++ )
    if ( cplane->drawIntercept(i) )
      determine_cutplane(i);
  redraw();
}


void TBmeshWin :: opacity( int s, float opac )
{
  model->opacity( s, opac );
  redraw();
}


/** determine the type of data reader to open
 *
 *  if the file is too big to fit into memory, use a threaded reader
 *
 * \param fn filename
 *
 */
DataReaderEnum TBmeshWin::getReaderType( const char *fn )
{
  long long getFreePages();
  int       getNumberTimes( const char * );

  long long memoryAvail = getFreePages() +
                     numframes*model->pt.num()*sizeof(DATA_TYPE)/getpagesize();

  long long memreq = getNumberTimes( fn );

  memreq *= model->pt.num()*sizeof(DATA_TYPE)/ getpagesize();

  if ( memoryAvail < memreq )
    return Threaded;
  else
    return AllInMem;
}

/** read in a data file 
 *
 * \param fn     file name
 * \param mslide time slider widget
 *
 * \returns non-zero iff no error
 */
int
TBmeshWin :: get_data( const char *fn, Myslider *mslide )
{
  DataClass<DATA_TYPE>* newDataBuffer=NULL;

  try {

    switch ( forcedThreaded ? Threaded : getReaderType( fn ) ) {
      case AllInMem:
        newDataBuffer = new DataAllInMem<DATA_TYPE>( fn, model->pt.num(),
                        model->base1() );
        break;
      case Threaded:
        newDataBuffer = new ThreadedData<DATA_TYPE>( fn, model->pt.num() );
        break;
      default:
        throw 1;
    }
  } 
  catch( CompressedFileExc cf ) {
    fl_alert( "Please uncompress data file: %s", cf.file.c_str() );
    return 1;
  }
  catch ( PointMismatch pm ) {
    fl_alert( "%s\nPoints number mismatch: expected %d but got %d", fn, pm.expected, pm.got );  
    return 2;
  }
  catch (...) {
    fl_alert("Unable to open data file: %s", fn );
    return 3;
  }

  if( max_time(ScalarDataGrid)>0 && newDataBuffer->max_tm()>0 && 
                                         newDataBuffer->max_tm()!=max_time() ) {
    fl_alert("%s","Incompatible number of frames in data" );
    delete newDataBuffer;
    return 4;
  }

  if ( dataBuffer != NULL ) delete dataBuffer;
  dataBuffer = newDataBuffer;

  numframes = dataBuffer->max_tm()+1;
  if ( numframes >1)
    have_data = Dynamic;
  else if ( numframes == 1 )
    have_data = Static;
  else
    return 0;
  if ( tm>=numframes )
    tm = 0;
  data = dataBuffer->slice(tm);

  timevec = (DATA_TYPE *)realloc( timevec, numframes*sizeof(DATA_TYPE) );

  if ( mslide != NULL ) {
    mslide->maximum( max_time() );
    mslide->value( tm );
    mslide->redraw();
  }

  if ( contwin->read_recalibrate->value() ) {
    optimize_cs();
    contwin->mincolval->value(cs->min());
    contwin->maxcolval->value(cs->max());
  }

  string fname = fn;
  string::size_type i0 = fname.rfind("/");
  if ( i0 < string::npos ) fname= fname.substr(i0+1,string::npos);
  fname = flwintitle + " --- " + fname;
  flwin->label( fname.c_str() );
  
  isosurfwin->islDirty(true);
  isosurfwin->issDirty(true);

  timeplotter->datafile( fn );
  if ( timeplotter->window->shown() ) 
    timeplot();

  redraw();

  return 0;
}

void TBmeshWin :: optimize_cs( void )
{
  if( dataBuffer==NULL ) return;
  cs->calibrate( dataBuffer->min(tm), dataBuffer->max(tm) );
  redraw();
}

void TBmeshWin :: randomize_color( Object_t obj )
{
  model->randomize_color( obj );
  redraw();
}


void TBmeshWin :: hiliteinfo()
{
  model->hilight_info( hinfo, hilight, data );
}


/** when something is clicked in the hilight info window, determine if it is a new object to 
 *  be highlighted
 *
 *  \param n number of selected line in window
 */
void
TBmeshWin :: select_hi( int n )
{
  if ( !n ) return;
  const char *txt = hinfo->text(n);
  Fl::copy( txt, strlen(txt), 2 );

  // only process selected line if first character is a number
  int d;
  if ( sscanf( txt, "%d", &d )!=1 && sscanf( txt, "in %*s %d", &d)!=1 )
    return;

  int i=n;
  Object_t objtype;
  MyValueInput *obin;

  while ( i>=1 )
    if ( strstr(  hinfo->text(i), "node" ) ) {
      objtype=Vertex;
      obin = contwin->verthi;
      break;
    } else if ( strstr(  hinfo->text(i), "surface" ) ) {
      objtype=SurfEle;
      obin = contwin->elehi;
      break;
    } else if ( strstr(  hinfo->text(i), "volume" ) ) {
      objtype=VolEle;
      obin = contwin->tethi;
      break;
    } else if ( strstr(  hinfo->text(i), "cable" ) ) {
      objtype=Cable;
      obin = contwin->cabhi;
      break;
    } else if ( strstr(  hinfo->text(i), "connection" ) ) {
      objtype=Cnnx;
      obin = contwin->cnnxhi;
      break;
    } else if ( strstr(  hinfo->text(i), "surf" ) ) {
      objtype=Surface;
      i = 0;				// ignore selection
      break;
    } else
      i--;

  int ho;
  if ( i>1 &&
       ( sscanf( txt, "%d", &ho )==1 || sscanf( txt, "%*s %*s %d", &ho)==1 ) ) {
    int lineno = hinfo->topline();
    obin->value(ho);
    highlight(objtype, ho );
    hinfo->topline( lineno );
  }
}

void TBmeshWin::output_png( const char* fn, Sequence *seqwidget )
{
  bool sequence = (seqwidget!=NULL);
  int start=tm;
  int stop;
  if ( sequence ) {
    Fl::flush();
    seqwidget->movieprog->minimum( 0 );
    seqwidget->movieprog->maximum( 1. );
    seqwidget->movieprog->value( 0 );
    seqwidget->movieprog->redraw();
    seqwidget->window->label(fn);
    seqwidget->window->show();
    seqwidget->window->redraw();
    Fl::flush();
    stop = (int)seqwidget->lastFrameNum->value();
  } else
    stop = tm;

  int width =  w();
  int height = h();
  flwin->show();
  int update_period = 5;	// number of frames after which to update progress
  int last_update=tm-update_period-10;
  int frameskip=int(contwin->frameskip->value());
  Frame frame( this );
  for ( ; tm<=stop; tm+=frameskip ) {

    if( model->pt.num_tm() ) model->pt.time(tm);//dynamic points

    string foutname( fn );

    if ( sequence ) {
      string::size_type i0 = foutname.rfind(".");
      if ( i0 < string::npos ) foutname= foutname.substr(0, i0);
      char number[6];
      sprintf( number, "%05d", tm );
      foutname = foutname + number + ".png";

      if ( tm-last_update>update_period ) {
        seqwidget->movieprog->value( (float)(tm-start)/(stop-1.));
        sprintf( number, "%.0f%%", 100.*(tm-start+1)/(stop-start+1) );
        seqwidget->movieprog->label(number);
        Fl::check();
        last_update = tm;
      }
    }
    
    frame.dump( w(), h(), foutname );
  }
  if ( sequence ) {
    seqwidget->movieprog->label("100%");
    seqwidget->movieprog->value(1.);
    Fl::check();
  }
  tm= start;
  if( model->pt.num_tm() ) model->pt.time(tm);//dynamic points
  redraw();
  Fl::flush();
}


/** control lighting in the model
 *
 *  \param max maximum model dimension
 *
 *  \Todo Lighting is messed up big time. For so,e reason, back and front are
 *        mixed up but if I switch the normals, it does not reverse front
 *        and back face lighting, but mmakes it all bad
 */
void
TBmeshWin::illuminate( GLfloat max )
{
  if ( !lightson ) {
    glDisable(GL_LIGHTING);
    glDisable(GL_RESCALE_NORMAL);
    glDisable(GL_COLOR_MATERIAL);
    GLfloat modamb[] = { .2, .2, .2, 1 };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, modamb );
    return;
  }
  glEnable(GL_RESCALE_NORMAL);

  const GLfloat diff_intensity = contwin->diffuseslide->value();
  const GLfloat spec_intensity = contwin->specularslide->value();
  const GLfloat am             = contwin->ambientslide->value();

  // Create a Directional Light Source
  GLfloat dir[] = { (GLfloat)contwin->lightx->value(),
                    (GLfloat)contwin->lighty->value(), 
                    (GLfloat)contwin->lightz->value(), 0. };
  normalize( dir );
  if ( headlamp_mode ) {
    glLightfv(GL_LIGHT2, GL_POSITION, dir);
    for ( int i=0; i<3; i++ ) dir[i] = -dir[i];
  }
  glLightfv(GL_LIGHT1, GL_POSITION, dir);

  GLfloat ambient0[] = { 0., 0., 0., 1. };
  glLightfv(GL_LIGHT1, GL_AMBIENT, ambient0);
  glLightfv(GL_LIGHT2, GL_AMBIENT, ambient0);
  GLfloat diffuse0[] = { diff_intensity, diff_intensity, diff_intensity, 1. };
  glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse0);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, diffuse0);
  GLfloat specular0[] = { spec_intensity, spec_intensity, spec_intensity, 1. };
  glLightfv(GL_LIGHT1, GL_SPECULAR, specular0);
  glLightfv(GL_LIGHT2, GL_SPECULAR, specular0);

  GLfloat modamb[] = { am, am, am, 1. };
  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, modamb );
  glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, 0 );
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);

  // draw an arrow showing the light direction
  if ( contwin->showLightDir->value() == 1 ) {
    float maxdim = model->maxdim();
    // save GL state
    glPushAttrib( GL_POLYGON_BIT|GL_LIGHTING_BIT );
    glDisable(GL_LIGHTING);

    GLUquadricObj* quado = gluNewQuadric();
    gluQuadricDrawStyle( quado, GLU_FILL );
    gluQuadricOrientation(quado, GLU_INSIDE);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glPushMatrix();

    glColor3f(1,0,0);
    // determine angle between z-axis and data vector
    GLfloat rotvect[4], zaxis[] = {0., 0., 1., 0};
    float angle = -acos( dot( dir, zaxis ))*180./M_PI;
    // determine rotation axis
    cross( dir, zaxis, rotvect );
    // draw arrow through center of object
    const GLfloat *poff = model->pt_offset();
    glTranslatef( -dir[0]*maxdim*1.1+poff[0],
                  -dir[1]*maxdim*1.1+poff[1], -dir[2]*maxdim*1.1+poff[2] );
    if ( headlamp_mode )
      glRotatef( angle, rotvect[0], rotvect[1], rotvect[2] );
    else
      glRotatef( -angle, rotvect[0], rotvect[1], rotvect[2] );
    draw_arrow( quado, 2.2*maxdim, maxdim/4., maxdim/20., maxdim/10. );
    // restore GL state
    gluDeleteQuadric( quado );
    glPopMatrix();
    glPopAttrib();
  }

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT1);
  if ( headlamp_mode )
    glEnable(GL_LIGHT2);
  else
    glDisable(GL_LIGHT2);
}


/** read in vector data
 * 
 * \param vp      slider
 * \param vptfile basename of vector data file
 *
 * \return nonzero if an error
 */
int
TBmeshWin::getVecData( void *vp, const char* vptfile )
{
  VecData* newvd;

  try {
    newvd = new VecData( vptfile );
  } catch (...) {
    return 1;
  }
  // make sure there is data and it matches the number of time instances
  if( max_time(VecDataGrid)>0 && newvd->maxtime()>0 && 
                                            newvd->maxtime() != max_time() ) {
    fl_message( "Number of times in vector data does not agree" );
    delete newvd;
    return 2;
  }

  if ( vecdata != NULL ) {
    *newvd = vecdata;
    delete vecdata;
  }

  vecdata = newvd;
  ((Myslider *)vp)->maximum( max_time() );
  contwin->update_vecdata( vecdata );
  contwin->window->redraw();
  redraw();
  return 0;
}


// read in Auxiliary grid
// return nonzero if an error
int
TBmeshWin::readAuxGrid( void *vp, const char* agfile )
{
  AuxGrid* newAuxGrid;

  try {
    newAuxGrid = new AuxGrid( agfile, auxGrid );
  } catch (...) {
    return 1;
  }

  // make sure there is data and it matches the number of time instances
  if( max_time(AuxDataGrid)>0 && newAuxGrid->num_tm()>1 && 
                                        newAuxGrid->num_tm()-1 != max_time() ) {
    fl_message( "Number of times in Aux Grid does not agree" );
    delete newAuxGrid;
    return 2;
  }
  if( auxGrid ) delete auxGrid;

  auxGrid = newAuxGrid;
  ((Myslider *)vp)->maximum( max_time() );
  contwin->window->redraw();
  redraw();
  return 0;
}


/** return maximum time to display
 *
 * \note There are possibly 4 grids which must all agree on the number of time
 *       instances. Grids with no data or only 1 instance of data
 *       are compatible with any number of time frames since they are
 *       static
 */
int TBmeshWin::max_time( GridType ignore )
{
  if( ignore!=ScalarDataGrid && have_data == Dynamic )
    return numframes-1;
  
  if( ignore!=VecDataGrid && vecdata && vecdata->maxtime()>0 )
    return vecdata->maxtime();

  if( ignore!=AuxDataGrid && auxGrid && auxGrid->num_tm()>0 )
    return auxGrid->num_tm()-1;

  if( ignore!=DynPtGrid && DynPtGrid && model->pt.num_tm()>0 )
    return model->pt.num_tm()-1;

  return 0;
}


// enter "vertex selection by mouse" mode
void
TBmeshWin:: select_vertex()
{
  ptDrawn.assign(model->pt.num(),false); 	// clear list of drawn vertices
  glSelectBuffer( HITBUFSIZE, &hitbuffer[0] );
  renderMode = GL_SELECT;
}


// exit select mode and figure out what was selected
void
TBmeshWin::process_hits()
{
  // recolour the pick button
  contwin->pickvert->color(132);
  contwin->pickvert->redraw();

  GLint hits = glRenderMode(renderMode=GL_RENDER);
  if ( hits == -1 ) {
    cerr << "Selection Buffer too small";
    return;
  }

  // process the hits and select the one with the smallest z-value
  if ( hits>0 ) {

    GLuint* ptr = &hitbuffer[0];
    int minvert = -1, vertname;
    float minz;

    for ( int i=0; i<hits; i++ ) {

      GLuint numnames = *ptr++;
      float z1 = *ptr++/0x7fffffff;
      ptr++; 							// skip z2
      for (int j = 0; j < numnames; j++)
        vertname = *ptr++;

      if ( (minvert == -1 || z1<minz) && vertname != -1 ) {
        minvert = vertname;
        minz = z1;
      }
    }
    // if there was a selection, highlight it
    if ( minvert != -1 ) {
      highlight( Vertex, minvert );
      contwin->verthi->value( minvert );
      if ( timeplotter->window->shown() ) timeplot();
    }

  }
  valid(0);		// reset the viewport and all that for the next redraw
}


/** dump the clipping planes and vertices which were drawn into a file
 *
 *  the file has the format: \n
 *  no_clipping_planes   \n
 *  a b c d   (coefficients for the plane 1 as defined in OpenGL) \n
 *  a b c d   (coefficients for the plane 2 as defined in OpenGL) \n
 *  .  \n
 *  .  \n
 *  a b c d   (coefficients for last plane as defined in OpenGL) \n
 *  visible_node1   \n
 *  visible_node2   \n
 *  visible_node3   \n
 *  .   \n
 *  .   \n
 *  .   \n
 *  visible_node?
 */
void TBmeshWin::dump_vertices()
{
  GLint hits;
  do {
    glSelectBuffer( hitbuffer.size(), &hitbuffer[0] );
    glRenderMode( renderMode=GL_SELECT );

    ptDrawn.assign(model->pt.num(),false); 	// clear list of drawn vertices
    dump_vert_list=true;
    valid(0);
    redraw();
    Fl::flush();
    glFlush();
    dump_vert_list=false;
    hits = glRenderMode(renderMode=GL_RENDER);
    
    if ( hits == -1 ) {
      hitbuffer.resize(hitbuffer.size()*2);
    }
  } while (hits == -1);

  char *fname=fl_file_chooser( "Choose vertex file", "*", "vertices.dat" );
  if ( fname == NULL ) return;
  ofstream of( fname );

  ptDrawn.assign(model->pt.num(),false); 	// clear list of drawn vertices

  // process the hits and extract the names
  GLuint* ptr = &hitbuffer[0];
  for ( int i=0; i<hits; i++ ) {
    GLuint numnames = *ptr;
    ptr += 3;							//skip z1 and z2
    for (int j=0; j<numnames; j++, ptr++)
      if (*ptr != -1) ptDrawn[*ptr]=true;
  }

  //output the clipping planes
  int cp_on = 0;
  for ( int i=0; i<NUM_CP; i++ ) 
	if( cplane->on(i) )
	  cp_on++;
  of << cp_on << endl;
  for ( int i=0; i<NUM_CP; i++ ) {
    GLdouble *cp_coeff;
	GLfloat   ctr[4];
    if ( cplane->on(i) ) {
      cp_coeff = cplane->plane(i);
	  model->pt.offset(ctr);
      for ( int j=0; j<3; j++ ) of << cp_coeff[j] << " ";
	  // adjust interecept for centering
	  of << cp_coeff[3]+V3d(cp_coeff).Dot(V3f(ctr)) << endl;
    } 
  }

  // output the vertices
  int vtx_on=0;
  for ( int i=0; i<model->pt.num(); i++ )
    if ( ptDrawn[i] ) 
      vtx_on++;
  of << vtx_on << endl;
  for ( int i=0; i<model->pt.num(); i++ ) if ( ptDrawn[i] ) of<< i << endl;

  of.close();
  valid(0);		// reset the viewport and all that for the next redraw
}


// output a PS or PDF file
void TBmeshWin::output_pdf( char *fn, bool PDF )
{
  GLint  format;

#ifdef HAVE_GL2PS

  // gl2ps does not work

  if ( PDF ) {
    fl_alert("Warning: Transparency may not appear correctly");
    format = GL2PS_PDF;
  } else  {
    format = GL2PS_EPS;
    fl_alert("Warning: Transparency is not supported in this output format");
  }

  FILE *fp = fopen( fn, "w" );
  GLint buffsize = 0, state = GL2PS_OVERFLOW;
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  while ( state == GL2PS_OVERFLOW ) {
    buffsize += 1024*1024;
    gl2psBeginPage ( "Title", "Meshalyzer", viewport,
                     format, GL2PS_BSP_SORT,
                     GL2PS_SIMPLE_LINE_OFFSET | GL2PS_SILENT |
                     GL2PS_OCCLUSION_CULL | GL2PS_BEST_ROOT | 
                     GL2PS_DRAW_BACKGROUND,
                     GL_RGBA, 0, NULL, 0, 0, 0, buffsize,
                     fp, fn );
    draw();
    state = gl2psEndPage();
  }
  fclose( fp );
#endif
}


// plot the time series for a vertex
void TBmeshWin::timeplot()
{
  if ( have_data==NoData || numframes<2 ) return;

  dataBuffer->time_series( hilight[Vertex], timevec );
  timeplotter->window->show();
  timeplotter->set_data(  hilight[Vertex], numframes, timevec, tm, 
                               dataBuffer->dt(), dataBuffer->t0() );
  timeplotter->window->redraw();
}


/** Draw the clipping plane
 *
 * \param cp the clipping plane
 */
void TBmeshWin::draw_clip_plane( int cp )
{
  const GLfloat clipPlaneOpacity = 0.3;

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_LIGHTING);
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glDepthMask(GL_FALSE);
#ifdef HAVE_GL2PS
  gl2psEnable(GL2PS_BLEND);
#endif
  glEnable(GL_BLEND);
  for ( int i=0; i<NUM_CP; i++ ) glDisable(CLIP_PLANE[i]);

  GLdouble* x = cplane->plane(cp);
  //GLdouble  mag = magnitude( x );
  GLdouble  mag = 1.;
  GLdouble  cpc[4];
  for ( int j=0; j<4; j++ ) cpc[j] = x[j]/mag;

  GLfloat  planeColor[4] = { 0, 1, 0,  clipPlaneOpacity }; //translucent green

  GLfloat vert[3];
  int v0, v1, vf;
  if ( fabs(x[2])>fabs(x[0]) && fabs(x[2])>fabs(x[1]) ) {//mostly in z-direction
    v0 = 0; v1 = 1; vf = 2;
  } else if ( fabs(x[1]) > fabs(x[0]) ) {	// mostly in y direction
    v0 = 2; v1 = 0; vf = 1;
  } else {									// mostly in x direction
    v0 = 1; v1 = 2; vf = 0;
  }

  glBegin(GL_POLYGON);
  glColor4fv( planeColor);
  const GLfloat *poff = model->pt_offset();
  for ( int i=0; i<4; i++ ) {
    vert[v0] = 2*(2*(!i||i==3)-1)*model->maxdim()+poff[v0];
    vert[v1] = 2*(2*(i>1)-1)*model->maxdim()+poff[v1];
    vert[vf] = -(cpc[v0]*vert[v0]+cpc[v1]*vert[v1]+cpc[3])/cpc[vf];
    glVertex3fv( vert );
  }
  glEnd();

  glPopAttrib();
}


/** write out the frame buffer after a change to it is has been made
 *
 * \param fname base name for output files
 */
void TBmeshWin::record_events( char* fn )
{
  unsigned long old_framenum=framenum;
  int           num=0;

  Frame frame( this );
  while ( recording ) {
    Fl::wait();						 // process events one at a time
    if ( old_framenum != framenum ) { // see if the draw routine has been called
      old_framenum = framenum;
      string fname = fn;
      string::size_type i0 = fname.rfind(".");
      if ( i0 < string::npos ) fname = fname.substr(0, i0);
      char fnum[32];
      sprintf( fnum, "%05d", num++ );
      fname = fname + fnum + ".png";
      frame.dump( w(), h(), fname );
    }
  }
  ostringstream msg;
  msg << num << " frames output";
  fl_alert( "%s", msg.str().c_str() );
}


/** change visibility of a region
 *
 *  \param region lists of region integers
 *  \param nr     \#regions
 *  \param on     whether a region is visible
 */
void TBmeshWin::region_vis( int *region, int nr, bool* on )
{
  for ( nr--; nr>=0; nr-- )
    model->region(nr)->visible( on[nr] );
  redraw();
}


/** turn translucency on/off
 *
 * \param b true for on
 */
bool translucency( bool b )
{
  if ( b ) {
    glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
#ifdef HAVE_GL2PS
    gl2psEnable(GL2PS_BLEND);
#endif
    glEnable(GL_BLEND);
  } else {
    GLint sd;
    glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &sd );
    if ( sd ) glPopAttrib();
#ifdef HAVE_GL2PS
    gl2psDisable(GL2PS_BLEND);
#endif
  }
  return b;
}


/** surface visibility
 *
 * \param l list of surfaces affected
 * \param v visibility
 */
void
TBmeshWin::surfVis( vector<int>&l, bool v )
{
  for ( int i=0; i<l.size(); i++ )
    model->surface(l[i])->visible(v);
  redraw();
}


/** surface element filled
 *
 * \param l list of surfaces affected
 * \param v visibility
 */
void
TBmeshWin::surfFilled( vector<int>& l, bool f )
{
  for ( int i=0; i<l.size(); i++ )
      model->surface(l[i])->filled(f);
  redraw();
}


/** surface element outlined
 *
 * \param l list of surfaces affected
 * \param f outlined
 */
void
TBmeshWin::surfOutline( vector<int>&l, bool f )
{
  for ( int i=0; i<l.size(); i++ )
      model->surface(l[i])->outline(f);
  redraw();
}


/** surface element outline color
 *
 * \param l list of surfaces affected
 * \param c colour
 */
void
TBmeshWin::surfOutColor( vector<int>&l, GLfloat* c )
{
  for ( int i=0; i<l.size(); i++ )
    model->surface(l[i])->outlinecolor(c[0],c[1],c[2],c[3]);
  redraw();
}


/** surface element fill color
 *
 * \param l list of surfaces affected
 * \param c colour
 */
void
TBmeshWin::surfFillColor( vector<int>&l, GLfloat *c )
{

  for ( int i=0; i<model->numSurf(); i++ )
    model->surface(l[i])->fillcolor(c[0],c[1],c[2],c[3]);
  redraw();
}


/** draw the cut planes
 *
 * \param reg the region under consideration
 */
void
TBmeshWin::draw_cut_planes( RRegion *reg )
{
  GLfloat elecol[]  = {1,0,0,1};
  GLfloat linecol[] = {0,0,0,1};

  glPushAttrib( GL_ALL_ATTRIB_BITS );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  glEnable(GL_BLEND);
  glShadeModel(GL_SMOOTH);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_ONE, GL_ZERO);
  glEnable( GL_LINE_SMOOTH );
  glEnable( GL_POLYGON_SMOOTH );

  if( isosurfwin->isolineOn->value() )
    glEnable(GL_POLYGON_OFFSET_FILL );

  bool showData=true;
  if ( !(datadst&Surface_flg) || have_data==NoData )
    showData = false;

  bool on_tr = dataopac->dop[Surface].on();

  for ( int i=0; i<NUM_CP; i++ ) {
    if ( cplane->drawIntercept(i) ) {

      glDisable( CLIP_PLANE[i] );

      for ( int e=0; e<_cutsurface[i]->num(); e++ ) {

        // copy normal for all nodes
        GLfloat n[_cutsurface[i]->ele(e)->ptsPerObj()*3 ];
        memcpy( n, _cutsurface[i]->norm(e), 3*sizeof(GLfloat ) );
        for ( int v=1; v<_cutsurface[i]->ele(e)->ptsPerObj(); v++ )
          memcpy( n+3*v, n, 3*sizeof(GLfloat ) );

        // interpolate data
        DATA_TYPE idata[_cutsurface[i]->ele(e)->ptsPerObj()];
        if ( showData && cplane->datafied(i) ) {
          for ( int v=0; v<_cutsurface[i]->ele(e)->ptsPerObj(); v++ ) {
            if( _branch_cut )
              idata[v] = _cutsurface[i]->interpolate( e, data, v, _branch_range );
            else
              idata[v] = _cutsurface[i]->interpolate( e, data, v );
          }
        }
        _cutsurface[i]->ele(e)->draw( 0, 0, elecol, cs, showData?idata:NULL,
                              1, dataopac->dop+Surface, facetshading?NULL:n );
      }
      glEnable( CLIP_PLANE[i] );
    }
  }

  if ( on_tr ) translucency(false);

  glPopAttrib();
}


/** determine the cutting plane
 *
 *  \param cp index of cutting plane
 *
 *  \note We are assuming the number of points does not change between
 *        calls
 */
void
TBmeshWin :: determine_cutplane( int cp )
{
  static bool fst=true;
  static char* cpvis;
  if ( fst ) {
    cpvis = new char[model->pt.num()];
    fst = false;
  }
  memset( cpvis, 0, model->pt.num()*sizeof(char) );

  if ( _cutsurface[cp] != NULL ) delete _cutsurface[cp];
  _cutsurface[cp] = new CutSurfaces;

  GLdouble* cpd = cplane->plane(cp);
  GLdouble  mag = magnitude( cpd );
  GLfloat   cpf[4];
  for ( int j=0; j<4; j++ ) cpf[j] = cpd[j]/mag;

  // first determine which points belong to visible regions
  for ( int r=0; r<model->_numReg; r++ ) {
    if ( !model->region(r)->visible() ) continue;
    vector<bool>& memb = model->region(r)->pt_membership();
    for ( int i=0; i<model->pt.num(); i++ )
      if ( memb[i] ) cpvis[i]=true;
  }

  // determine the points clipped by the plane
  // for visible points: ax+by+cz+d > 0
  const GLfloat *pp = model->pt.pt();
  for ( int j=0; j<model->pt.num(); j++ )
    if ( cpvis[j] && (dot( pp+3*j, cpf )<-cpf[3]) )
      cpvis[j] = false;

  for ( int r=0; r<model->_numReg; r++ ) {
    if( !model->region(r)->visible() ) 
      continue;
    for ( int e=0; e<model->_numVol; e++ ) {
      if( model->region(r)->ele_member(e) ) {
        Interpolator<DATA_TYPE> *interp;
        SurfaceElement *se = model->_vol[e]->cut( cpvis, cpf, interp );
        if ( se!= NULL ) {
          _cutsurface[cp]->addEle( se, cpf, interp );
        }
      }
    }
  }
}


/** draw isovalue lines
 *
 *  we check if the values in the widget have changed so that we do not
 *  recompute lines needlessly
 */
void
TBmeshWin::draw_iso_lines()
{
  IsosurfControl *isc = isosurfwin;

  if( have_data==NoData || !isc->isolineOn->value() )
    return;

  glPushAttrib( GL_POLYGON_BIT );

  if( isc->islDirty() || tm!=isoline->tm() ){
    delete isoline;
    isoline=new IsoLine( isc->isolineVal0->value(), isc->isolineVal1->value(),
            isc->isoNumLines->value(), tm );
    if( _branch_cut ) 
      isoline->branch( true, _branch_range[0], _branch_range[1] );
    for ( int s=0; s<model->numSurf(); s++ ) 
      isoline->process( model->surface(s), data, !isc->isoLineRestrict->value() );
  }
  isoline->color( isc->islColor() );
  isoline->draw( isc->islDatify->value()?cs:NULL, isc->islThickness() );
  isoline->threeD( isc->threeD() );
  
  // we need to turn off clipping when we draw on the clipping plane or we will 
  // get z-fighting 
  for( int i=0; i<NUM_CP; i++ )
    if( _cutsurface[i] != NULL ){
      glDisable( CLIP_PLANE[i] );
      IsoLine cutline( isc->isolineVal0->value(), isc->isolineVal1->value(),
                       isc->isoNumLines->value(), tm );
      cutline.branch( _branch_cut, _branch_range[0], _branch_range[1] );
      cutline.process( _cutsurface[i], data );
      cutline.draw( isc->islDatify->value()?cs:NULL, isc->islThickness() );
      glEnable( CLIP_PLANE[i] );
    }

  glPopAttrib( );
}


void TBmeshWin::CheckMessageQueue(){
  
  LinkMessage::CmdMsg msg;
  
  unsigned int numberOfMsgRead = 0;
   
  while ((tmLink->ReceiveMsg(msg) == 0) &&
	   (numberOfMsgRead < MAX_MESSAGES_READ)) {
	
	ProcessLinkMessage(msg);

	numberOfMsgRead++;
  }  
}


int TBmeshWin::ProcessLinkMessage(const LinkMessage::CmdMsg& msg)
{
  if ( msg.command == LinkMessage::LINK ) {
    tmLink->link(msg.newlink);
  } else if( msg.command == LinkMessage::UNLINK ) {
    tmLink->unlink(msg.senderPid);
  } else if(  msg.command == LinkMessage::DIFFUSE_LINK ) {
     tmLink->diffuse_link( msg.senderPid );
  } else if( msg.command == LinkMessage::VIEWPORT_SYNC ) {
    trackball.SetScale(msg.trackball.scale);
    V3f modtrans = msg.trackball.trans*model->maxdim();
    trackball.SetTranslation(modtrans.X(), modtrans.Y(), modtrans.Z() );
    trackball.qRot = msg.trackball.qRot*model->syncRefRot();
    trackball.qSpin = msg.trackball.qSpin;
    redraw();
  } else if( msg.command == LinkMessage::TIME_SYNC ) {
    int newTm = msg.sliderTime;
    if (newTm > contwin->tmslider->maximum()) {
      newTm = contwin->tmslider->maximum();
    } else if (newTm < 0) {
      newTm = 0;
    }
    contwin->tmslider->value(newTm);
    set_time(newTm);
  } else if( msg.command == LinkMessage::COLOUR_SYNC ) { 
    cs->calibrate( msg.colour.min, msg.colour.max );
    cs->size( msg.colour.levels );
    contwin->mincolval->value(cs->min());
    contwin->maxcolval->value(cs->max());
    contwin->numcolev->value(cs->size());
    contwin->cstype->value( msg.colour.scale );
    contwin->cstype->mvalue()->do_callback(contwin->cstype);
    redraw();
  } else if( msg.command == LinkMessage::CLIP_SYNC ) {
    stringstream cpinfo;
    Quaternion Qrefinv = model->syncRefRot().GetConjugate();
    for( int i=0; i<6; i++ ) {
      V3f cn = Qrefinv.Rotate(msg.clip.cnorm[i]);
      cpinfo << cn.X() << " " << cn.Y() << " " << cn.Z() << " " 
             << msg.clip.inter[i] << " " << msg.clip.state[i] << endl;
    }
    cplane->set_CPs( cpinfo );
    redraw();
  } else {
    return -1;
  }
  return 0;
}

void TBmeshWin::SendClipSyncMessage()
{
  stringstream cpinfo;
  LinkMessage::CmdMsg msg;

  cplane->get_CPs( cpinfo );
  for( int i=0; i<6; i++ ) {
    V3f cpnorm;
    cpinfo >> cpnorm;
    msg.clip.cnorm[i]  = model->syncRefRot().Rotate(cpnorm);
    cpinfo >> msg.clip.inter[i];
    cpinfo >> msg.clip.state[i];
  }
  msg.command = LinkMessage::CLIP_SYNC;
  tmLink->SendMsgToAll(msg);
}


void TBmeshWin::SendViewportSyncMessage()
{
   // create a viewport sending
  float scale = 0.0;
  V3f v3f_trans;
  V3f p3f_origin;
  Quaternion qSpin;
  Quaternion qRot;

  // retrieve vals
  scale = trackball.GetScale();
  v3f_trans = trackball.GetTranslation();
  qRot = trackball.GetRotation(); 
  qSpin = trackball.qSpin;

  LinkMessage::CmdMsg msgToSend;
  msgToSend.trackball.scale = scale;
  msgToSend.trackball.trans = v3f_trans/model->maxdim();
  msgToSend.trackball.qSpin = qSpin;
  msgToSend.trackball.qRot = qRot*model->syncRefRot().GetConjugate();
  
  msgToSend.command = LinkMessage::VIEWPORT_SYNC;
  
  tmLink->SendMsgToAll(msgToSend);
}

void TBmeshWin::SendTimeSyncMessage()
{
  LinkMessage::CmdMsg msgToSend;
  msgToSend.sliderTime = tm;
  
  msgToSend.command = LinkMessage::TIME_SYNC;
  
  tmLink->SendMsgToAll(msgToSend);
}

void TBmeshWin::SendColourSyncMessage()
{
  LinkMessage::CmdMsg msgToSend;
  msgToSend.command       = LinkMessage::COLOUR_SYNC;
  msgToSend.colour.min    = cs->min();
  msgToSend.colour.max    = cs->max();
  msgToSend.colour.scale  = contwin->cstype->value();
  msgToSend.colour.levels = cs->size();
  
  tmLink->SendMsgToAll(msgToSend);
}


extern sem_t *meshProcSem;

/** signal the time linked meshalyzer instances
 *
 *  \param dir direction
 */
void
TBmeshWin::signal_links( int dir ) 
{
  if( meshProcSem==SEM_FAILED ) return;

  // make sure we start at zero
  int numsem=0;
  sem_getvalue( meshProcSem, &numsem );
  for( int i=0; i<numsem; i++ )
    sem_wait( meshProcSem ); 

  int num_pending=0;

  for( set<int>::iterator it=timeLinks.begin(); it!=timeLinks.end(); it++ ){
    if( kill( *it, dir>0?SIGUSR1:SIGUSR2 ) )
      timeLinks.erase( *it );
    else
      num_pending++;
  }

  for( int i=0; i<num_pending; i++ )
    sem_wait( meshProcSem );
}


/** update the time
 *
 * \param a new time index
 *
 * \pre a is a valid time index
 * \return false if not a valid time index
 */
bool
TBmeshWin:: set_time(int a)
{
  if( a > max_time() )
    return false;

  tm=a;
  if (timeplotter!=NULL)timeplotter->highlight(tm);
  contwin->tmslider->value(tm);
  contwin->tmslider->redraw();
  if ( timeplotter->window->shown() ) timeplotter->highlight( tm );
  redraw();
  if( auxGrid && auxGrid->data() && contwin->auxautocalibratebut->value() ) {
    contwin->auxmincolval->value( auxGrid->cs.min() );
    contwin->auxmaxcolval->value( auxGrid->cs.max() );
  }
  if( model->pt.num_tm() )
    model->pt.time(tm);
  if( model->pt.dynamic() )
    for( int i=0; i<6; i++ )
        if(cplane->drawIntercept(i)) determine_cutplane(i);
  return true;
}


/** read in the dynamic points file
 *
 * \param fn the file name 
 *
 * \return nonzero on failure
 *
 * \note the dynamic points file is an IGB file of data type IBG_VEC3_f
 */
int
TBmeshWin :: read_dynamic_pts( const char *fn, Myslider *mslide )
{
  try {
    model->pt.dynamic( fn, numframes );
  }
  catch( FrameMismatch fm ) {
    fl_alert( "Incompatible number of time frames. Expected %d but got %d\n", fm.expected, fm.got );
    return 1;
  }
  catch( PointMismatch pm ) {
    fl_alert( "Incompatible number of points. Expected %d but got %d\n", pm.expected, pm.got );
    return 2;
  }

  model->pt.time(tm);
  mslide->maximum( max_time() );
  numframes = mslide->maximum()+1;
  mslide->redraw();

  return 0;
}


/** manage branch cuts 
 *
 * \param min
 * \param max
 * \param tol
 */
void
TBmeshWin :: branch_cut(double min, double max, float tol)
{
  if( min != 0. || max != 0. ){
    _branch_cut = true; 
    _branch_range[0] = min;
    _branch_range[1] = max;
  } else
    _branch_cut = false; 

  isosurfwin->islDirty(true);
  isosurfwin->issDirty(true);

  redraw();
}


/**
 * @brief center view on specified vertex
 *
 * @param vtx index of vertex
 */
void
TBmeshWin :: ctr_on_vtx( int vtx )
{
  const GLfloat *p = model->pt.pt(vtx);
  trackball.SetOrigin( -p[0], -p[1], -p[2] );
  trackball.SetTranslation( 0., 0., 0. );
  redraw();
}

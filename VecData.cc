#include "VecData.h"

#include <cstdio>
#include <cstdlib>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Color_Chooser.H>
#include <zlib.h>
#include <cstring>
#include <iostream>
#include "IGBheader.h"

using namespace std;

const float vecsize = 0.1;		// relative length of maximum vector by default
const float REL_MIN_VEC_SIZE = 100.;

template< class S>
void read_IGB_vec_data( S* vdata, S* sdata, IGBheader& h )
{
  S* vd=NULL;
  
  if( h.num_components() == 4 )
    vd = new S[h.slice_sz()*h.num_components()];

  int nread;

  for( int i=0; i<h.t(); i++ ){
	if( !vd ) 
	  nread = h.read_data( vdata+i*h.num_components()*h.slice_sz() );
	else {
	  nread = h.read_data( vd );
	  for( int j=0; j<h.slice_sz(); j++ ) {
		for( int k=0; k<3; k++ )
		  vdata[3*j+k+i*h.slice_sz()*3] = vd[j*4+k];
		sdata[j+i*h.slice_sz()] = vd[j*4+3];
	  }
	}
	if( nread != h.slice_sz() ) {
	  h.t(i);
	  break;
	}
  }

  if( vd ) delete[] vd;
}


/** just copy all nondata fields, the ones
 *  related to display
 *
 * \param v vector data
 */
VecData &
VecData::operator=( const VecData* v )
{
  if( this == v )
    return *this;

  _length     = v->_length;
  maxmag      = v->maxmag;
  _stride     = v->_stride;
  _length_det = v->_length_det;
  _colour_det = v->_colour_det;
  autocal     = v->autocal;
  _stoch      = v->_stoch;
  _draw_heads = v->_draw_heads;
  _3D         = v->_3D;
  memcpy( _colour, v->_colour, 4*sizeof(GLfloat) );
  cs->size(v->cs->size());

  if( !sdata ) {
    if( _length_det == Scalar ) 
      _length_det = Vector;
    if( _colour_det == Scalar ) 
      _colour_det = Vector;
  }

  return *this;
}


/** draw an arrow
   \param quado      GLU quadric object
   \param stick      length of stick portion
   \param head       length of head
   \param stick_rad  radius of stick
   \param head_rad   radius of base
*/
void draw_arrow( GLUquadricObj* quado, GLfloat stick, GLfloat head,
		               GLfloat stick_rad, GLfloat head_rad, bool draw_head )
{
  gluCylinder( quado, stick_rad, stick_rad, stick, 10, 2 );
  if( !draw_head )
    return;
  glTranslatef( 0, 0, stick );
  gluDisk( quado, stick_rad, head_rad, 2, 4 );
  gluCylinder( quado, head_rad, 0, head, 10, 2 );
}


VecData::VecData(const char* vptfile)
{
  _colour[1] = _colour[2] = 0.;
  _colour[0] = _colour[3] = 1.;

  cs = new Colourscale;

  if ( vptfile==NULL )
    if ( (vptfile=fl_file_chooser("Pick pts file", "*.vpts*", NULL)) == NULL )
      throw -1;

#ifdef USE_HDF5
  if( strstr( vptfile, ".datH5:vector/" ) )
    read_vec_HDF5( vptfile );
  else
#endif
    read_vec_nonHDF5( vptfile );

  //determine the largest magnitude vector and scalar extrema
  maxmag = magnitude( vdata );
  float  tmp;
  for ( int i=1; i<numpt; i++ )
    if ( (tmp=magnitude( vdata+i*3 )) > maxmag )
      maxmag = tmp;
  cs->calibrate(0,maxmag);

  if ( sdata != NULL ) {
    scalar_min = scalar_max = sdata[0];
    for ( int i=1; i<numpt; i++ ) {
      if ( sdata[i] > scalar_max ) scalar_max = sdata[i];
      if ( sdata[i] < scalar_min ) scalar_min = sdata[i];
    }
  }

  // get a quadric for OpenGL rendering;
  quado = gluNewQuadric();
  gluQuadricDrawStyle( quado, GLU_FILL );
  gluQuadricOrientation(quado, GLU_INSIDE);
}


#ifdef USE_HDF5
/** read HDF5 vector data 
 *
 * \param vecspec vector grid specification
 */
void
VecData::read_vec_HDF5( const char*vecspec )
{
  unsigned int    indx;
  string gtype;
  hid_t  hin;
  ch5s_vector_grid info;
  string vn = vecspec;
  int parse_HDF5_grid( const char*, string&, unsigned int& );

  if( ch5_open( vn.substr(0,vn.find_last_of(":")).c_str(), &hin ) )
    throw 1;

  if( parse_HDF5_grid( vn.c_str(), gtype, indx ) || gtype!="vector" ||
          ch5s_vector_grid_info(hin, indx, &info ) )
    throw 1;

  numpt = info.num_vectors;
  pts   = new GLfloat[numpt*3];
  numtm = info.time_steps;
  ch5s_vector_read_points( hin, indx, pts );

  vdata = (float *)realloc( vdata, 3*info.num_vectors*info.time_steps*
                                                        sizeof(float) );
  if( info.num_components==3 ) {  // no scalar data
    ch5s_vector_read( hin, indx, 0, info.time_steps-1, vdata );

  } else {
    sdata = (float *)realloc( sdata,
                           info.num_vectors*info.time_steps*sizeof(float) );
    float *tdata = new float[info.num_vectors*info.num_components];
    for( int i=0; i<info.time_steps; i++ ) {
      ch5s_vector_read( hin, indx, i, i, tdata );
      for( int j=0; j<info.num_vectors; j++ ){
        memcpy( vdata+(i*info.num_vectors+j)*3, tdata+4*j, sizeof(float)*3 );
        sdata[i*info.num_vectors+j] = tdata[4*j+3];
      }
    }
    delete[] tdata;
  }
  ch5_close( hin );
}
#endif


/** read  IGB or ASCII vector data
 *
 * \param vptfile vector points file
 */
void
VecData::read_vec_nonHDF5( const char *vptfile )
{
  const int bufsize=1024;
  char fn[1024], buff[bufsize];

  strcpy( fn, vptfile );

  // read in the vector points file (.vpts[.gz])
  gzFile in;
  if ( (in=gzopen(fn, "r" )) == NULL )
    if ( (in=gzopen(strcat(fn,".gz"),"r")) == NULL ) {
      fn[strlen(fn)-3] = '\0';
      if ( (in=gzopen(strcat(fn,".vpts"),"r")) == NULL )
        if ( (in=gzopen(strcat(fn,".gz"),"r")) == NULL ) {
          fn[strlen(fn)-7] = '\0';
          if ( (in=gzopen(strcat(fn,"vpts"),"r")) == NULL )
            if ( (in=gzopen(strcat(fn,".gz"),"r")) == NULL ) {
              cerr << "Error: cannot open file: " << fn << endl;
              throw -1;
            }
        }
    }
  if ( gzgets(in, buff, bufsize) == Z_NULL ) return;
  if ( sscanf( buff, "%d", &numpt ) != 1 ) return;
  pts = new GLfloat[numpt*3];
  for ( int i=0; i<3*numpt; i+=3 ) {
    if ( gzgets(in, buff, bufsize) == Z_NULL ) return;
    if ( sscanf( buff, "%f %f %f", pts+i, pts+i+1, pts+i+2 ) != 3 ) return;
  }
  gzclose( in );

  // read in the vector data (.vec[.gz])
  // first figure out the name of the data file
  char* sp=strstr( fn, "vpts.gz" );
  if ( sp == NULL ) sp = strstr( fn, "vpts" );
  *sp='\0';

  if ( (in=gzopen( strcat(fn, "vec"), "r")) == NULL )
    if ( (in=gzopen( strcat(fn, ".gz"), "r")) == NULL ) {
      cerr << "No data file found to match points file: " << fn << endl;
      throw -1;
    }

  IGBheader h(in);
  if( !h.read() ) {               // IGB file
	vdata = (float *)realloc( vdata, 3*h.t()*h.slice_sz()*sizeof(float) );
    if( h.type() == IGB_VEC4_f || h.type() == IGB_VEC4_d ) {  // scalar data
      sdata = (float *)realloc( sdata, h.t()*h.slice_sz()*sizeof(float) );
	}
	read_IGB_vec_data( vdata, sdata, h );
	numtm = h.t();

  } else {                           // text file
	gzrewind(in);
	// determine if scalar data
	if ( gzgets( in, buff, bufsize ) == Z_NULL ) return;
	float tscal[4];
	int npl = sscanf(buff,"%f %f %f %f", tscal, tscal+1, tscal+2, tscal+3 );
	const char *scanstr;
	if ( npl==4 )
	  scanstr="%f %f %f %f";
	else if ( npl==3 )
	  scanstr="%f %f %f";
	else
	  return;
	gzrewind( in );

	int nread;
	do {
	  vdata = (float *)realloc( vdata, 3*(++numtm)*numpt*sizeof(float) );
	  if ( npl==4) sdata = (float *)realloc( sdata, numtm*numpt*sizeof(float) );
	  float* vdp = vdata + 3*(numtm-1)*numpt;
	  float* sdp = sdata + (numtm-1)*numpt;
	  for ( nread=0; nread<numpt; nread++ ) {
		if ( gzgets( in, buff, bufsize ) == Z_NULL ) break;
		if ( sscanf( buff, scanstr, vdp, vdp+1, vdp+2, sdp ) != npl ) break;
		vdp += 3;
		sdp++;
	  }
	} while ( nread == numpt );
	numtm--;
	if ( !numtm ) {
	  delete[] pts;
	  cerr << "Not enough data in file" << endl;
	  throw -1;
	}
  }
}



VecData::~VecData()
{
  if ( pts ) delete[] pts;
  if ( vdata != NULL ) {free( vdata ); vdata = NULL; }
  if ( sdata != NULL ) {free( sdata ); sdata = NULL; }
  gluDeleteQuadric( quado );
}


/*
 * choose the colour to draw the vectors
 */
void
VecData::colourize()
{
  double r=_colour[0],g=_colour[1],b=_colour[2];
  if ( fl_color_chooser( "Vector colour", r, g, b ) ) {
    _colour[0] = r;
    _colour[1] = g;
    _colour[2] = b;
  }
}


// draw the vectors
// given the time to draw and the maximum dimension of the drawn region
void
VecData::draw(int tm, float maxdim)
{
  if ( !_disp ) return;
  if ( tm >= numtm ) tm=numtm;								// for now
  _last_tm = tm;

  int    offset = tm*numpt;
  float* vdp    = vdata + 3*offset;
  float* sdp    = sdata + offset;

  // determine vector length
  float base_size = maxdim*vecsize*_length/2.;
  if ( _length_det == Vector )
    base_size /= maxmag;
  else if ( _length_det == Scalar && sdata != NULL )
    base_size /= scalar_max-scalar_min;

  glPushAttrib( GL_POLYGON_BIT|GL_LINE_BIT );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  
  if ( _colour_det == FixedVCdata  || sdata==NULL ) glColor3fv( _colour );

  // GLU objects are drawn starting at the origin and extending in the z direction
  // We must move translate and rotate the world to get the proper orientation
  GLfloat rotvect[3], zaxis[]={0,0,1};

  //for ( int i=0; i<3*numpt; i+=3*_stride ) {
  for ( int i=0; i<3*numpt; i+=3 ) {

    if( _stride>1 ) {
      if( _stoch ) {
        if( random() > RAND_MAX/_stride )
          continue;
      } else {
        if( (i/3+_start)%_stride )
          continue;
      }
    }

    // determine size to draw vectors
    float size = base_size;
    if ( _length_det == Vector ) {
      size *= magnitude( vdp+i );
      if ( maxmag*base_size/size > REL_MIN_VEC_SIZE ) continue; // don't draw tiny vectors
    } else if ( _length_det == Scalar && sdata != NULL )
      size *= (sdp[i/3]-scalar_min);

    // determine vector colour
    if ( _colour_det==Vector)
      cs->colourize( magnitude(vdp+i) );
    else if ( _colour_det==Scalar && sdata!=NULL )
      cs->colourize( sdp[i/3] );

    // translate, rotate and draw
    if( _3D ) {
    glPushMatrix();
    glTranslatef( pts[i], pts[i+1], pts[i+2] );
    // determine angle between z-axis and data vector
    float angle = -acos( dot( vdp+i, zaxis )/magnitude(vdp+i) )*180./M_PI;
    // determine rotation axis
    cross( vdp+i, zaxis, rotvect );
    glRotatef( angle, rotvect[0], rotvect[1], rotvect[2] );
    draw_arrow( quado, size, size, size/10., size/5., _draw_heads );
    glPopMatrix();
    } else {
      GLfloat vec[3]={vdp[i],vdp[i+1],vdp[i+2]};
      scale(normalize(vec),size);
      glLineWidth(size/20.);
      glBegin(GL_LINES);
        glVertex3f( pts[i], pts[i+1], pts[i+2] );
        glVertex3f( pts[i]+vec[0], pts[i+1]+vec[1], pts[i+2]+vec[2] );
      glEnd();
    }
  }
  glPopAttrib();
}


void
VecData::optimize_cs()
{
  if ( _colour_det==Vector) {
    float *vdp = vdata + 3*_last_tm*numpt;
    float magn;
    maxmag = magnitude( vdp );
    for ( int i=0; i<numpt; i++ ) {
      if ( (magn=magnitude(vdp)) > maxmag )
        maxmag = magn;
      vdp += 3;
    }
    cs->calibrate(0, maxmag );
  } else if ( _colour_det==Scalar ) {
    float *sdp = sdata + _last_tm*numpt;
    scalar_min=*sdp;
    scalar_max=*sdp;
    for ( int i=1; i<numpt; i++ ) {
      if ( sdp[i]<scalar_min ) scalar_min = sdp[i];
      if ( sdp[i]>scalar_max ) scalar_max = sdp[i];
    }
    cs->calibrate( scalar_min, scalar_max );
  }
}

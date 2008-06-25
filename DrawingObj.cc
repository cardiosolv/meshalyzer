#include "DrawingObjects.h"
#include "gl2ps.h"
#include "Vector3D.h"
#include "VecData.h"

enum lpint_enum{ BOTH_ON_PLANE, NO_INTERSECTION };

/** find the intersection of a plane with a line
 *
 *  \param a  first point of line
 *  \param b  second point of line
 *  \param pc coefficients describing plane: ax+by+cz+d=0
 *  \param pp point on plane
 *  \param ip intersection point
 *
 *  \return where along a,b the intersection occurs
 */
float find_line_plane_intersect( const GLfloat* a, const GLfloat* b, 
		const GLfloat* pc, const GLfloat* pp, GLfloat* ip )
{
  Vector3D<GLfloat>  ppv(pp), p0(a), p1(b);
  GLfloat tmp1[3];
  float t = (ppv-p0).Dot(pc)/(p1-p0).Dot(pc);
  add( p0.e,((p1-p0)*t).e, ip );
  if( t<0 || t>1 ) {
	if( isinf(t) )
	  throw BOTH_ON_PLANE;
	else
	  throw NO_INTERSECTION;
  }
  return t;
}


/** turn translucency on/off
 *
 * \param b true for on
 */
void DrawingObj :: translucency( bool b )
{
  if( b==true ) {
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_FALSE);
	gl2psEnable(GL2PS_BLEND);
	glEnable(GL_BLEND);
  } else {
	GLint sd;
	glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &sd );
	if( sd ) glPopAttrib();
	gl2psDisable(GL2PS_BLEND);
  }
}

/** try a bunch of file names to open a file 
 *
 * \param base basename of file
 * \param ext  extension for type of file without a .
 *
 * \return a pointer to the open file
 */
gzFile openFile( const char *base, const char* ext )
{
  gzFile in;
  string fn = base;
  
  if( (in=gzopen( fn.c_str(), "r" )) == NULL ) {
	fn += ".gz";
	if( (in=gzopen( fn.c_str(), "r")) == NULL ) {
	  fn  = base;
	  fn += ext;
	  if( (in=gzopen( fn.c_str(), "r")) == NULL ) {
		fn += ".gz";
		if( (in=gzopen( fn.c_str(), "r")) == NULL ) {
	  	  fn = base;
	      fn += ".";
	      fn += ext;
		  if( (in=gzopen( fn.c_str(), "r")) == NULL ) {
			fn += ".gz";
			if( (in=gzopen( fn.c_str(), "r")) == NULL ) 
			  throw 1;
		  }
		}
	  }
	}
  }
  return in;
}


/** register vertices so that they can be picked
 *
 * \param p0      start index
 * \param p1      stop  index
 * \param ptDrawn list of points drawn
 */
void MultiPoint::register_vertices( int p0, int p1, vector<bool>& ptDrawn )
{
  for( int i=p0; i<=p1; i++ ) {
	int j, p = _ptsPerObj*i;
	for( j=0; j<_ptsPerObj; j++ )
	  if( !_pt->vis( _node[p+j] ) )
		break;
	if( j != _ptsPerObj )
	  continue;
	for( j=0; j<_ptsPerObj; j++ )
	  if( !ptDrawn[_node[p+j]] ) {
		ptDrawn[_node[p+j]] = true;
		glLoadName( _node[p+j] );
		glBegin(GL_POINTS);
		  glVertex3fv( _pt->pt(_node[p+j]) );
		glEnd();
	  }
  }
}


/** try reading in a normals file - one for each element
 *
 * \param e0 first element
 * \param e1 last element
 * \param fnb base file name endinging in "."
 */
void SurfaceElement::read_normals( int e0, int e1, const char *fnb ) 
{
  gzFile in;

  in=openFile( fnb, "nrml" );

  const int bufsize=1024;
  char buff[bufsize];
  for( int i=e0; i<=e1; i++ ) {
	gzgets(in, buff, bufsize);
	sscanf( buff, "%f %f %f", _nrml+i*3, _nrml+i*3+1, _nrml+i*3+2 );
  }
  gzclose(in );
}


/** define an object
 *
 * \param n list of nodes defining object
 * \param a object index
 *
 * \pre \p nl must be at least _ptsPerObj*\p n long
 * \note \p n must not be deallocated and declared with new
 */
void  MultiPoint::define( int *nl, int n )
{
  _node = nl;
  _n = n;
}


/** add an object to the list 
 *
 * \param n list of nodes
 *
 */
void MultiPoint::add( int *n )
{
  _n++;
  if( _n>1 ) {
    int *newnode = new int[_n*_ptsPerObj];
    memcpy( newnode, _node, _n*_ptsPerObj*sizeof(int) );
	delete[] _node;
	_node = newnode;
  } else
	_node = new int[_ptsPerObj];
  
  for( int j=0; j<_ptsPerObj; j++ )
	_node[_ptsPerObj*_n-_ptsPerObj+j] = n[j];
}


/** add a volume element
 *
 * \param n list of nodes
 * \param r region
 */
void VolElement::add( int *n, int r )
{
  MultiPoint::add( n );
  if( _n>1 ) {
    int *newreg = new int[_n*_ptsPerObj];
    memcpy( newreg, _region, _n*_ptsPerObj*sizeof(int) );
	delete[] _region;
	_region = newreg;
  } else
	_region = new int[1];
  _region[_n-1] = r;
}


/** determine if a clipping plane intersects an element.
 *  
 *  \param pd      visible points
 *  \param cp      normalized clip plane, cp[0]x + cp[1]y +cp[2]z + cp[3] = 0
 *  \param e       the element in the list
 *  \param interp  construct to interpolate data
 *  \param numedge number of edges for element
 *  \param edges   list of nodes defining edges
 *
 *  \return a surface element if intersection, NULL otherwise
 */
SurfaceElement* 
VolElement::planecut( char *pd, GLfloat* cp, 
    Interpolator<DATA_TYPE>* &interp, int numedge,
								const int edges[][2], int e=0 )
{
  // return if all vertices visible or all not visible
  int  p, *ndpt=_node+e*_ptsPerObj;
  bool visible= pd[*ndpt++];
  for( p=1; p<_ptsPerObj; p++ )
	if( pd[*ndpt++] != visible )
	  break;
  if( p==_ptsPerObj ) return NULL;

  //find an arbitrary point on the plane
  GLfloat ptOnPlane[3] = {1,1,1};
  if( cp[0] ) 
	ptOnPlane[0] = -(cp[3]+cp[1]*cp[2])/cp[0];
  else if( cp[1] )
	ptOnPlane[1] = -(cp[3]+cp[2]*cp[0])/cp[1];
  else
	ptOnPlane[2] = -(cp[3]+cp[0]*cp[1])/cp[2];

  // get intersections and centroid of intersections of plane with edges
  GLfloat intersect[6*numedge];
  float   d[2*_ptsPerObj];
  int     inode[4*_ptsPerObj];
  int     num_int=0;
  Vector3D<GLfloat> centroid;
  for( int i=0; i<numedge; i++ ) {
	if( (pd[_node[edges[i][0]]] + pd[_node[edges[i][1]]]) == 1 ) {
	  inode[2*num_int]   = _node[edges[i][0]];
	  inode[2*num_int+1] = _node[edges[i][1]];
	  try{ 
        d[num_int] = find_line_plane_intersect(_pt->pt(_node[edges[i][0]]),
		   _pt->pt(_node[edges[i][1]]), cp, ptOnPlane, intersect+3*num_int );
	  } catch( lpint_enum raison ) {
		if( raison==BOTH_ON_PLANE ) {		// add both points
		  d[num_int] = d[num_int+1] = 1;
		  for( int p=0; p<3; p++ ) {
			intersect[3*num_int+p]   = _pt->pt(_node[edges[i][0]] )[p];
			intersect[3*num_int+3+p] = _pt->pt(_node[edges[i][1]] )[p];
		  }
		  inode[2*num_int]   = inode[2*num_int+3] = _node[edges[i][0]];
		  inode[2*num_int+1] = inode[2*num_int+2] = _node[edges[i][1]];
	      centroid += intersect+3*num_int++;
		} else
		  cerr << "Weirdness" << endl;
	  }
	  centroid += intersect+3*num_int++;
	}
  }
  centroid /= (GLfloat)num_int;

  // interpolation information
  if( num_int ) interp = new Interpolator<DATA_TYPE>[num_int];
  for( int i=0; i<num_int; i++ ) 
	interp[i].set( inode[2*i], inode[2*i+1], d[i] );

  if( num_int>3 ) {
	// determine angular position on cutting plane
	float angle[num_int];
	for( int i=0; i<num_int; i++ ) {
	  Vector3D<GLfloat>rays(intersect+i*3);
	  rays -= centroid;
	  if( fabs(cp[2]) > fabs(cp[1]) && fabs(cp[2])>fabs(cp[0]) )
		angle[i] = atan2( rays.Y(), rays.X() );
	  else if( fabs(cp[1]) > fabs(cp[0]) )
		angle[i] = atan2( rays.X(), rays.Z() );
	  else
		angle[i] = atan2( rays.Z(), rays.Y() );
	}

	// simple sort the points by angle
	for( int i=0; i<num_int-1; i++ ) {
	  float min = angle[i];
	  int   minind = i;
	  for( int j=i+1; j<num_int; j++ ) 
		if( angle[j]<min )
		  min = angle[minind=j];
	  float  tmp = angle[i]; angle[i] = angle[minind]; angle[minind] = tmp;
	  Interpolator<DATA_TYPE> itmp = interp[i]; interp[i] = interp[minind]; 
	  interp[minind] = itmp;
	  Vector3D<GLfloat> tp( intersect+3*i );
	  memcpy( intersect+3*i, intersect+3*minind, 3*sizeof(GLfloat));
	  memcpy( intersect+3*minind, tp.e, 3*sizeof(GLfloat));
	}
  }

  // make new element
  Point *pt = new Point;
  pt->add( intersect, num_int );
  pt->setVis( true );
  SurfaceElement *se;
  if( num_int==3 )
    se = new Triangle( pt );
  else
    se = new PolyGon( pt, num_int );
  int nl[num_int];
  for( int i=0; i<num_int; i++ ) nl[i]=i;
  se->add( nl );
  GLfloat *normal = new GLfloat[3];
  memcpy( normal, cp, 3*sizeof(GLfloat) );
  se->nrml( normal );

  return se;
}

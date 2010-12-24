#include "DrawingObjects.h"
#include "gl2ps.h"
#include "Vector3D.h"
#include "VecData.h"

enum lpint_enum{ BOTH_ON_PLANE, NO_INTERSECTION };

static const int simple_index[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13};

/** find the intersection of a plane with a line
 *
 *  \param a  first point of line
 *  \param b  second point of line
 *  \param pc coefficients describing plane: ax+by+cz+d=0
 *  \param ip intersection point
 *
 *  \return fraction along a,b the intersection occurs
 */
float find_line_plane_intersect( const GLfloat* a, const GLfloat* b,
                             const GLfloat* pc, GLfloat* ip )
{
  float d1 = dot( a, pc )+pc[3];
  float d2 = dot( b, pc )+pc[3];
  float t  = d1/(d1-d2);

  GLfloat v[3];
  add( a, scale( sub(b,a,v), t ), ip );

  if ( t<0 || t>1 ) {
    if ( isinf(t) )
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
  if ( b==true ) {
    glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
    gl2psEnable(GL2PS_BLEND);
    glEnable(GL_BLEND);
  } else {
    GLint sd;
    glGetIntegerv(GL_ATTRIB_STACK_DEPTH, &sd );
    if ( sd ) glPopAttrib();
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

  if ( (in=gzopen( fn.c_str(), "r" )) == NULL ) {
    fn += ".gz";
    if ( (in=gzopen( fn.c_str(), "r")) == NULL ) {
      fn  = base;
      fn += ext;
      if ( (in=gzopen( fn.c_str(), "r")) == NULL ) {
        fn += ".gz";
        if ( (in=gzopen( fn.c_str(), "r")) == NULL ) {
          fn = base;
          fn += ".";
          fn += ext;
          if ( (in=gzopen( fn.c_str(), "r")) == NULL ) {
            fn += ".gz";
            if ( (in=gzopen( fn.c_str(), "r")) == NULL )
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
  for ( int i=p0; i<=p1; i++ ) {
    int j, p = _ptsPerObj*i;
    for ( j=0; j<_ptsPerObj; j++ )
      if ( !_pt->vis( _node[p+j] ) )
        break;
    if ( j != _ptsPerObj )
      continue;
    for ( j=0; j<_ptsPerObj; j++ )
      if ( !ptDrawn[_node[p+j]] ) {
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
  for ( int i=e0; i<=e1; i++ ) {
    gzgets(in, buff, bufsize);
    sscanf( buff, "%f %f %f", _nrml+i*3, _nrml+i*3+1, _nrml+i*3+2 );
  }
  gzclose(in );
}


/** define an object
 *
 * \param nl list of nodes defining object(s)
 * \param n  \#object to define
 *
 * \pre  \p nl must be at least _ptsPerObj*\p n long
 * \post \p _n is allocated if need be
 */
void  MultiPoint::define( const int *nl, int n )
{
  delete[] _node;
  _node = new int[n*_ptsPerObj];
  memcpy( _node, nl, n*_ptsPerObj*sizeof(int) );
  _n = n;
}


/** add an object to the list
 *
 * \param n list of nodes defining object
 */
void MultiPoint::add( int *n )
{
  _n++;
  if ( _n>1 ) {
    int *newnode = new int[_n*_ptsPerObj];
    memcpy( newnode, _node, _n*_ptsPerObj*sizeof(int) );
    delete[] _node;
    _node = newnode;
  } else
    _node = new int[_ptsPerObj];

  memcpy( _node, n, _ptsPerObj*sizeof(int) );
}


/** add a volume element
 *
 * \param n list of nodes
 * \param r region
 */
void VolElement::add( int *n, int r )
{
  MultiPoint::add( n );
  if ( _n>1 ) {
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
  for ( p=1; p<_ptsPerObj; p++ )
    if ( pd[*ndpt++] != visible )
      break;
  if ( p==_ptsPerObj ) return NULL;

  // get intersections and centroid of intersections of plane with edges
  GLfloat intersect[6*numedge];
  float   d[2*_ptsPerObj];
  int     inode[4*_ptsPerObj];
  int     num_int=0;
  Vector3D<GLfloat> centroid;
  for ( int i=0; i<numedge; i++ ) {
    if ( (pd[_node[edges[i][0]]] + pd[_node[edges[i][1]]]) == 1 ) {
      inode[2*num_int]   = _node[edges[i][0]];
      inode[2*num_int+1] = _node[edges[i][1]];
      try {
        d[num_int] = find_line_plane_intersect(_pt->pt(_node[edges[i][0]]),
             _pt->pt(_node[edges[i][1]]), cp, intersect+3*num_int );
      } catch ( lpint_enum raison ) {
        if ( raison==BOTH_ON_PLANE ) {		// add both points
          d[num_int] = d[num_int+1] = 1;
          for ( int p=0; p<3; p++ ) {
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
  if ( num_int ) interp = new Interpolator<DATA_TYPE>[num_int];
  for ( int i=0; i<num_int; i++ )
    interp[i].set( inode[2*i], inode[2*i+1], d[i] );

  if ( num_int>3 ) {
    // determine angular position on cutting plane
    float angle[num_int];
    for ( int i=0; i<num_int; i++ ) {
      Vector3D<GLfloat>rays(intersect+i*3);
      rays -= centroid;
      if ( fabs(cp[2]) > fabs(cp[1]) && fabs(cp[2])>fabs(cp[0]) )
        angle[i] = atan2( rays.Y(), rays.X() );
      else if ( fabs(cp[1]) > fabs(cp[0]) )
        angle[i] = atan2( rays.X(), rays.Z() );
      else
        angle[i] = atan2( rays.Z(), rays.Y() );
    }

    // simple sort the points by angle
    for ( int i=0; i<num_int-1; i++ ) {
      float min = angle[i];
      int   minind = i;
      for ( int j=i+1; j<num_int; j++ )
        if ( angle[j]<min )
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
  PPoint *pt = new PPoint;
  pt->add( intersect, num_int );
  pt->setVis( true );
  pt->offset( _pt->offset() );
  SurfaceElement *se;
  if ( num_int==3 )
    se = new Triangle( pt );
  else if( num_int==4 )
    se = new Quadrilateral( pt );
  else
    se = new PolyGon( pt, num_int );
  int nl[num_int];
  for ( int i=0; i<num_int; i++ ) nl[i]=i;
  se->add( nl );
  GLfloat *normal = new GLfloat[3];
  memcpy( normal, cp, 3*sizeof(GLfloat) );
  se->nrml( normal );

  return se;
}


/** linearly interpolate the position of a value along an element edge 
 *
 *  \param p0    vertex 0
 *  \param dat0  data on p0
 *  \param p1    vertex 1
 *  \param dat1  data on p1
 *  \param val   data between dat0 and dat1
 *  \param pint  location of val (computed)
 *
 *  \return relative weight of node 0
 */
float
edge_interp( const GLfloat *p0, DATA_TYPE dat0, const GLfloat *p1,
             DATA_TYPE dat1, DATA_TYPE val, GLfloat* pint )
{
  GLfloat edge[3];
  sub( p1, p0, edge );
  float d = (val-dat0)/(dat1-dat0);
  if( d<0 || d>1 )
    throw NO_INTERSECTION;
  add( p0, scale( edge, d ), pint );
  return 1.-d;
}


/** determine the isosurface for a multipoint object
 *
 *  A row in a table is determined from the nodes above the threshold value
 *  Each row is of the form \n
 *      \#polygon \#sides_poly0 edge0_node0 edge0_node1 edge1_node0
 *      edge1_ node1 ... edgeN_node1 \#sides_poly1  edge0_node0 ...
 *      edgeN_node2
 *
 * \param dat    data for all the nodes
 * \param val    value for the isosurface
 * \param npoly  number of polygons
 * \param interp interpolation values
 *
 *  \return a list of element pointers
 *  \post   npoly is the number of elements in the list
 */
MultiPoint ** MultiPoint::isosurf( DATA_TYPE *dat, DATA_TYPE val, int &npoly,
        vector<Interpolator<DATA_TYPE>*> *interp )
{
  // determine row index into table 
  unsigned int index=0;
  for( int i=_ptsPerObj-1; i>=0; i-- ) {
    index <<= 1;
    
    // Modified: data[_node[i]] > val to this
    if( dat[_node[i]]>=val )
      index += 1;
  }

  const int* poly        = iso_polys(index);
  npoly                  = poly[0];// number of polygons to create
  int       poly_start   = 1;      // first polygon defined after \#polygons
  
  // do not continue if number of polygons are zero
  if (npoly == 0)
  {
    return NULL;
  }
  
  MultiPoint **isoele = new MultiPoint *[poly[0]]; //element pointer list

  for( int n=0; n<npoly; n++ ) {
    int      npts = poly[poly_start];         // \#nodes defining polygon
    GLfloat *pt   = new GLfloat[npts*3];      // local point list
    for( int i=0; i<npts; i++ ) {
      int pindex  = poly_start+1+i*2;
      int n0 = _node[poly[pindex]];
      int n1 = _node[poly[pindex+1]];
      float d = edge_interp( (*_pt)[n0], dat[n0], (*_pt)[n1], 
                                             dat[n1], val, pt+3*i );

      if( interp )
        interp->push_back(new Interpolator<DATA_TYPE>( n0, n1, d )); 
    }
	
    PPoint   *pts = new PPoint;
    pts->add( pt, npts );
    pts->setVis(true);
    pts->offset(_pt->offset());

    switch(poly[poly_start]) {
        case 1:
            assert(0);
            break;
        case 2:
            isoele[n] = new Connection( pts );
            break;
        case 3: 
            isoele[n] = new Triangle( pts );
            break;
        case  4:
            isoele[n] = new Quadrilateral( pts );
            break;
        default:
            isoele[n] = new PolyGon( pts, npts );
            break;
    }
    isoele[n]->define( simple_index );
    if( poly[poly_start]>2 ) {                 // it is a surface element
      SurfaceElement *se = dynamic_cast<SurfaceElement*>(isoele[n]);
      se->compute_normals(0,0);

	  // determine the vertex normal
      GLfloat *ptnrml = new GLfloat[3*poly[poly_start]];
      for( int i=0; i<poly[poly_start]; i++ ) {
        int pindex = poly_start+1+i*2;
        int n0 = _node[poly[pindex]];
        int n1 = _node[poly[pindex+1]];
        
        // fix a up from n0 to 0 and n1 to 1
	   // note: need to fix up the normalization algorithm.
        normalize(sub( pts->pt(0), pts->pt(1), ptnrml+i*3 ));
      }

      se->vertnorm( ptnrml );
    }poly_start += npts*2+1;
  }
  return isoele;
}

/** 
 * return a list of lists of nodes defining the surface of a volume element
 *
 * the node table is filled in by row with the number of nodes and then
 * the global node numbers
 *
 * \param ve   volume element
 * \param ft   table to fill in with the face info
 * \param ns   number of surfaces
 * \param nn   number of nodes per surface
 * \param nl   table of local nodes for each surface
 *
 * \return the number of faces
 */
int 
VolElement:: make_surf_nodelist( int ve, int **ft, const int ns, int nn, 
                                                        const int **nl )
{
  for( int s=0; s<ns; s++ ) {
    int i;
    int *rptr = reinterpret_cast<int*>(nl)+s*nn;
    for( i=0; i<nn; i++ ) {
      int lnode = *rptr++;
      if( lnode == -1 )
        break;
      ft[s][i+1] = _node[ve*_ptsPerObj+lnode];
    }
    ft[s][0] = i;
  }
  return ns;
}



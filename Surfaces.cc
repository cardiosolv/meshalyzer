#include "Surfaces.h"
#include <string.h>
#include "VecData.h"
#include <algorithm>
#include <vector>
#include <iterator>

/** find the z depth of a point (assume w=1)
 *
 * \param m projection matrix or row of the ModelViewProjection 
 * \param v vertex (x,y,z)
 *
 * \return the z depth buffer coordinate
 */
GLfloat 
z_proj( const GLfloat* m, const GLfloat *v )
{
  GLfloat z=m[3];
  for( int i=0; i<3; i++ ) z += m[i]*v[i];
  return z;
}


Surfaces::Surfaces( PPoint *pl ) : _p(pl), is_visible(true),_filled(true),
    _outline(false),_vertnorm(NULL)
{
  fillcolor( 1., 0.5, 0.1 );
  outlinecolor( 0.125, 0.8, 0.7 );
}


/**
 * @brief  set the material colour properties
 */
void
Surfaces :: set_material()
{
  GLfloat diffuseb[4], specularb[4];

  CSET( diffuseb,  _diffuse[0]*_backlight,  _diffuse[3]  );
  CSET( specularb, _specular[0]*_backlight, _specular[3] );

  glDisable(GL_COLOR_MATERIAL);
  glMaterialfv(GL_FRONT, GL_SPECULAR,  _specular         );
  glMaterialf (GL_FRONT, GL_SHININESS, _shiny            );
  glMaterialfv(GL_FRONT, GL_DIFFUSE,   _diffuse          );
  glMaterialfv(GL_BACK,  GL_SPECULAR,  specularb         );
  glMaterialf (GL_BACK,  GL_SHININESS, _shiny*_backlight );
  glMaterialfv(GL_BACK,  GL_DIFFUSE,   diffuseb          );
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
}


void Surfaces :: fillcolor( float r, float g, float b, float a )
{
  _fillcolor[0] = r;
  _fillcolor[1] = g;
  _fillcolor[2] = b;
  _fillcolor[3] = a;
}


//! set outline colour
void Surfaces :: outlinecolor( float r, float g, float b, float a )
{
  _outlinecolor[0] = r;
  _outlinecolor[1] = g;
  _outlinecolor[2] = b;
  _outlinecolor[3] = a;
}


/** get the vertex normals for the surface
 *
 * \param vm vector of normals for all points
 *
 */
void Surfaces::get_vert_norms( GLfloat *vn )
{
  for ( int i=0; i<_vert.size(); i++  ) 
    memcpy( vn+_vert[i]*3, _vertnorm+i*3, 3*sizeof(GLfloat) );
}


/** set the vertex normals for the surface
 *
 * \param pt all points
 *
 */
void Surfaces::determine_vert_norms( PPoint& pt )
{
  vector<bool> has_norm(pt.num());  // if elements attached to node

  GLfloat *tvn = new GLfloat[pt.num()*3];
  memset(  tvn, 0, pt.num()*3*sizeof(GLfloat) );

  const GLfloat* n;

  for ( int i=0; i<_ele.size(); i++ ) {
    if ( (n=_ele[i]->nrml(0)) == NULL ) continue;
    const int *pnt = _ele[i]->obj();
    for ( int j=0; j<_ele[i]->ptsPerObj(); j++ ) {
      for ( int k=0; k<3; k++ ) tvn[3*pnt[j]+k] += n[k];
      has_norm[pnt[j]] = true;
    }
  }
  // count \# nodes in surface
  int numvert = 0;
  for ( int i=0; i<pt.num(); i++ ) {
    if ( has_norm[i] ) normalize(tvn+i*3);
    numvert++;
  }

  _vert.resize(numvert);
  _vertnorm = new GLfloat[3*numvert];
  numvert = 0;
  for ( int i=0; i<pt.num(); i++ ) {
    if ( has_norm[i] ) {
      memcpy( _vertnorm+3*numvert, tvn+i*3, sizeof(GLfloat)*3 );
      _vert[numvert++] = i;
    }
  }
  delete[] tvn;
}


/** draw the surface
 *  \param fill     fill colour
 *  \param cs       colour scale
 *  \param dat      nodal data (NULL for nodata display)
 *  \param stride   draw every n'th element
 *  \param dataopac data opacity
 *  \param ptnrml   vertex normals (NULL for none)
 *  \param sort     draw from back to front
 */
void Surfaces::draw( GLfloat *fill, Colourscale *cs, DATA_TYPE *dat,
                     int stride, dataOpac* dataopac, const GLfloat*ptnrml,
                     bool sort )
{
  GLboolean lightson;
  glGetBooleanv( GL_LIGHTING, &lightson );

  if( stride != _oldstride ) _zlist.resize( (_ele.size()+stride-1)/stride );

  if( sort ) {

    // build modelview projection matrix - only compute row to determine z
    GLfloat proj[16], modview[16], mvp[4]{};
    glGetFloatv(GL_PROJECTION_MATRIX, proj );
    glGetFloatv(GL_MODELVIEW_MATRIX, modview );
    for( int i=0; i<4; i++ )
      for(int j=0; j<4; j++ )
        mvp[i] += modview[2+j*4]*proj[4*i+j]; // column major order

    if( memcmp(mvp, _oldmvp, sizeof(mvp) ) || stride != _oldstride) {
      memcpy( _oldmvp, mvp, sizeof(mvp) );
#pragma omp parallel for 
      for ( int i=0; i<_ele.size(); i+=stride ){
        const PPoint *pts = _ele[i]->pt();
        _zlist[i/stride]  = {i,0};
        const int *nn     = _ele[i]->obj();
        for( int k=0; k<3; k++ )
          _zlist[i/stride].z += z_proj(mvp,pts->pt(nn[k]));
      }
      std::sort( _zlist.begin(), _zlist.end(), 
                    [](const vtx_z a, const vtx_z b ){return a.z > b.z;} );
    }

  } else if( stride != _oldstride )
    for ( int i=0; i<_ele.size(); i+=stride )
      _zlist[i/stride].i = i;

  _oldstride = stride;

  glBegin(GL_TRIANGLES);
  for ( auto a :_zlist ) 
    if( _ele[a.i]->ptsPerObj() == 3 )
      _ele[a.i]->draw( 0, fill, cs, dat, dataopac, ptnrml, lightson );
  glEnd();
  glBegin(GL_QUADS);
  for ( auto a :_zlist ) 
    if( _ele[a.i]->ptsPerObj() == 4 )
      _ele[a.i]->draw( 0, fill, cs, dat, dataopac, ptnrml, lightson );
  glEnd();
}

/** redraw elements through which the branch cut passes with flat shading 
 *
 * \param range  range of cut
 * \param data   data to display
 * \param cs     colour scale
 * \param stride output stride
 * \param opac   data opacity    
 */
void 
Surfaces ::correct_branch_elements( GLdouble *range, DATA_TYPE *data,
                         Colourscale *cs, int stride, dataOpac *opac )
{
#define BRANCH_TOL  0.2
  bool cross_branch( DATA_TYPE *d, int n, double min, double max, double tol );

  int shade;
  glGetIntegerv(GL_SHADE_MODEL, &shade );
  glShadeModel( GL_FLAT );
  GLboolean lightson;
  glGetBooleanv( GL_LIGHTING, &lightson );

  DATA_TYPE d[MAX_NUM_SURF_NODES*2];
  for ( int i=0; i<_ele.size(); i+=stride ) {
    const int *n = _ele[i]->obj();
    for( int j=0; j<_ele[i]->ptsPerObj(); j++ )
      d[j] = data[n[j]];
    if( cross_branch( d, _ele[i]->ptsPerObj(), range[0], range[1], BRANCH_TOL ) )
      _ele[i]->draw( i, _fillcolor, cs, data, opac, _vertnorm, lightson );
  }

  glShadeModel( shade );
}



/** register the vertices
 *
 *  \param vb true if point already registered
 */
void Surfaces::register_vertices( vector<bool>& vb )
{
  for( auto e : _ele ) 
    e->register_vertices( 0, 0, vb );
}

/** flip the normals
 */
void Surfaces :: flip_norms()
{
  for( int i=0; i<3*(_vert.size()); i++ )
    _vertnorm[i] *=-1;
}


/** write the surface to a file 
 * \param of output stream
 */
void
Surfaces :: to_file( ofstream &of )
{
    of << num() << "  " << label() << endl;
 
    for( auto e : _ele ) {
      const int* n=e->obj();
      if( e->ptsPerObj()==3 ) 
        of << "Tr";
      else
        of << "Qd";
      for( int k=0; k< e->ptsPerObj(); k++ )
        of << " " << n[k];
      of << endl;
    }  
}


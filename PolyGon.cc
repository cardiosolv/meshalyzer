#include "DrawingObjects.h"

/** draw many Triangles
 *
 *  \param p0       first index of point to draw
 *  \param p1       last index of point to draw
 *  \param colour   colour to use if no data
 *  \param cs       colour scale
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 *  \param dataopac data opacity
 */
void PolyGon::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                    DATA_TYPE* data, int stride, dataOpac* dataopac )
{
  draw( p0, p1, colour, cs, data, stride, dataopac, NULL );
}


/** draw one polygon object
 *
 *  \param p0       index of first point to draw
 *  \param p1       index of last point to draw
 *  \param colour   colour to use if no data
 *  \param cs       colour scale
 *  \param data     data associated with nodes (NULL for no data display)
 *  \param stride   draw every n'th point
 *  \param dataopac data opacity
 *  \param ptnrml   vertex normals (NULL for none)
 */
void PolyGon::draw( int p0, int p1, GLfloat *colour, Colourscale* cs,
                    DATA_TYPE* data, int stride, dataOpac* dataopac,
                    const GLfloat* ptnrml )
{
  GLboolean lightson;
  glGetBooleanv( GL_LIGHTING, &lightson );

  int i = 0;
  for ( i=0; i<_ptsPerObj; i++ )
    if ( !_pt->vis(_node[i]) )
      return;

  if ( lightson && ptnrml==NULL) glNormal3fv( nrml(0) );

  glBegin(GL_POLYGON);

  if ( data!=NULL ) {
    if ( dataopac->on() ) {		// data opacity
      for ( int j=0; j<_ptsPerObj; j++ ) {
        cs->colourize( data[_node[j]], dataopac->alpha(data[_node[j]]));
        if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[j]*3 );
        glVertex3fv( _pt->pt(_node[j]) );
      }
    } else {		// no data opacity
      for ( int j=0; j<_ptsPerObj; j++ ) {
        cs->colourize( data[_node[j]], colour[3] );
        if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[j]*3 );
        glVertex3fv( _pt->pt(_node[j]) );
      }
    }
  } else {							// no data
    glColor4fv( colour );
    for ( int j=0; j<_ptsPerObj; j++ ) {
      if ( lightson && ptnrml ) glNormal3fv( ptnrml+_node[j]*3 );
      glVertex3fv( _pt->pt(_node[j]) );
    }
  }

  glEnd();
}


/** draw a polygon
 *
 *  \param e      element to highlight
 *  \param colour colour to use
 *  \param size   size of element
 */
void PolyGon :: draw( int e, GLfloat *colour, float size )
{
  if ( e<_n ) {

    int eind = _ptsPerObj*e;

    for ( int i=0; i<_ptsPerObj; i++ )
      if ( !_pt->vis(_node[i]) )
        return;

    glLineWidth( size );
    glColor4fv( colour );

    glBegin(GL_POLYGON );
    for ( int j=0; j<_ptsPerObj; j++ ) {
      Vector3D<GLfloat> a = _pt->pt(_node[eind]);
      glVertex3fv( _pt->pt(_node[eind++]) );
    }
    glEnd();
  }
}


/** for a surface with ordered node numbers, compute the normals
 *
 * \param e0 first element
 * \param e1 last element
 */
void PolyGon::compute_normals(int e0, int e1)
{
  int e, n;

  if ( _nrml==NULL ) _nrml=new GLfloat[e1*3+3];

  for ( e=e0; e<=e1; e++ ) {

    const int* ele = _node+3*e;
    Vector3D<GLfloat> a(_pt->pt(ele[0]));
    Vector3D<GLfloat> b(_pt->pt(ele[1]));
    Vector3D<GLfloat> c(_pt->pt(ele[2]));

    b -= a;
    b = b.Cross(c-a);
    b.Normalize();
    _nrml[3*e]   = b.X();
    _nrml[3*e+1] = b.Y();
    _nrml[3*e+2] = b.Z();
  }
}


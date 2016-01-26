#ifndef SURFACES_H
#define SURFACES_H

#include <stdio.h>
#include <stdlib.h>
#include "DrawingObjects.h"
#include <string>
#include <fstream>

class Surfaces
{
  public:
    Surfaces( PPoint *pl);
    Surfaces():_p(NULL){}
    GLfloat* fillcolor( ){ return _fillcolor; }
    void     fillcolor(float r, float g, float b, float a=1);
    GLfloat* outlinecolor( ){ return _outlinecolor; }
    void     outlinecolor(float r, float g, float b, float a=1);
    inline bool visible(){return is_visible;}
    inline void visible(bool a){ is_visible=a; }
    inline bool filled( void ){  return _filled; }
    inline void filled( bool a ){ _filled=a; }
    inline bool outline( void ){  return _outline; }
    inline void outline( bool a ){ _outline=a; }
    void get_vert_norms( GLfloat *vn );
    void determine_vert_norms( PPoint & );
    SurfaceElement*& ele( int a ){ return _ele[a]; }
    void addele(int a,SurfaceElement*e){_ele[a]=e;}
    int  num() const {return _ele.size();}
    void num(int a){_ele.resize(a);}
    vector<SurfaceElement*>& ele(){return _ele;}
    void draw(GLfloat*,Colourscale*,DATA_TYPE*,int,
              dataOpac*,const GLfloat*);
    void   register_vertices( vector<bool>& );
    void   label( string s ){ _label=s; }
    string label( void ) { return _label; }
    void   flip_norms();
    void   correct_branch_elements( GLdouble *, DATA_TYPE *, 
                          Colourscale *, int, dataOpac * );
    void  to_file( ofstream &of );
  protected:
    PPoint   *_p;
    GLfloat  _fillcolor[4];
    GLfloat  _outlinecolor[4];
    bool      is_visible;
    bool     _filled;         //!< draw filled
    bool     _outline;        //!< draw the outline
    GLfloat* _vertnorm;   //!< vertex normals
    int*     _vert;       //!< vertices for which normals are computed
    vector<SurfaceElement*> _ele; //!< list of elelments to draw
    string   _label;
};

#endif

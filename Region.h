#ifndef REGION_H
#define REGION_H

#include <stdio.h>
#include<FL/gl.h>
#include <vector>
#include "DrawingObjects.h"

class RRegion
{
  public:
    RRegion(int n, int l=0, bool b=true );
    RRegion(VolElement **, int, int n, int l );
    GLfloat* get_color( Object_t obj ){ return color[obj]; }
    void set_color(Object_t, float r, float g, float b, float a=1);
    inline bool visible(){return is_visible;}
    inline void visible(bool a){ is_visible=a; }
    bool show( Object_t obj ){ return showobj[obj]; }
    void show( Object_t obj, bool a ){ showobj[obj] = a; }
    void label(int l){ _label=l; }
    int  label(void)const {return _label;}
    vector<bool>* membership(){ return &_member; }
    vector<bool>& ele_membership(){ return _elemember; }
    inline bool member(int a){ return _member[a]; }
    inline void member(int a, bool b ){ _member[a]=b;}
    int  first(Object_t t){ return startind[t]; }
    void first(Object_t t, int a){ startind[t]=a; }
    int  size(Object_t t){ return _size[t]; }
    void size(Object_t t, int a){ _size[t]=a; }
    bool threeD(Object_t t){ return _3D[t]; }
    void threeD(Object_t t, int a){ _3D[t]=a; }
  private:
    GLfloat color[maxobject+2][4];
    int     startind[maxobject+2];
    int     endind[maxobject+2];
    bool    showobj[maxobject+2];
    bool    is_visible;
    int    _label;           //!< identifier in file for region
    vector<bool> _member;    //!< whether a node is part of region
    vector<bool> _elemember; //!< whether a element is part of region
    void    initialize( int, int, int );
    float   _size[maxobject+2];
    bool    _3D[maxobject+2];
};

int RRegion_sort( const void *a, const void *b );

#endif

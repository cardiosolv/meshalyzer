#ifndef REGION_H
#define REGION_H

#include <stdio.h>
#include<FL/gl.h>
#include <vector>
#include "DrawingObjects.h"

class Region
{
  public:
    Region(int n, int l=0, bool b=true );
    Region(VolElement **, int, int n, int l );
    GLfloat* get_color( Object_t obj ){ return color[obj]; }
    void set_color(Object_t, float r, float g, float b, float a=1);
    inline bool visible(){return is_visible;}
    inline void visible(bool a){ is_visible=a; }
    bool show( Object_t obj ){ return showobj[obj]; }
    void show( Object_t obj, bool a ){ showobj[obj] = a; }
    void label(int l){ _label=l; }
    int  label(void)const {return _label;}
    vector<bool>* membership(){ return &_member; }
    inline bool member(int a){ return _member[a]; }
    inline void member(int a, bool b ){ _member[a]=b;}
    int  first(Object_t t){ return startind[t]; }
    int  first(Object_t t, int a){ startind[t]=a; }
  private:
    GLfloat color[maxobject+2][4];
    int     startind[maxobject+2];
    int     endind[maxobject+2];
    bool    showobj[maxobject+2];
    bool    is_visible;
    int    _label;           //!< identifier in file for region
    vector<bool> _member;    //!< whether a node is part of region
    void    initialize( int, int );
};

int Region_sort( const void *a, const void *b );

#endif

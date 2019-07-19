#ifndef FRAME_H

#include <string>
#include "TBmeshWin.h"
#include "PNGwrite.h"
#ifdef OSMESA
#include <GL/osmesa.h>
#endif

class  Frame {
    public:
      Frame( TBmeshWin *t );
      ~Frame();
      Frame( TBmeshWin *t, int, int, string );
      int write( int, int, string, int );
      int write( int, int,  string, int, int,int stride=1 );
      void dump( int, int,  string );
    private:
     GLubyte* _buffer = NULL;
     int _w;
     int _h;
     TBmeshWin *_tbwm;
     void delete_objs();
#ifdef OSMESA
     OSMesaContext	_ctx;
#else
     GLuint _fb, _color_rb, _depth_rb;
#endif
};

#endif

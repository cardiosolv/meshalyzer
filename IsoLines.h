#ifndef ISOLINE_H
#define ISOLINE_H

#include "Model.h"
#include "DrawingObjects.h"
#include "CutSurfaces.h"
#include <vector>

class IsoLine {
    public:
        IsoLine( double v0, double v1, int n, int t ):_v0(v0),_v1(v1),_nl(n),
               _t(t) {_color[0]=_color[1]=_color[2]=0;_color[3]=1.;}
        ~IsoLine();
        int process( Surfaces *, DATA_TYPE * );
        int process( CutSurfaces *, DATA_TYPE * );
        int nl(){ return _nl; }
        int v0(){ return _v0; }
        int v1(){ return _v1; }
        int tm(){ return _t;  }
        void draw( Colourscale *, GLfloat );
		void color( const GLfloat* c ){memcpy(_color,c,4*sizeof(GLfloat));}
    private:
        vector<MultiPoint *> _polygon;
        GLfloat               _color[4];
        double                _v0, _v1;
        int                   _nl;
        int                   _t;
};

#endif


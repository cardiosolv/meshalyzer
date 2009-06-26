#ifndef ISOSURFACE_H
#define ISOSURFACE_H

#include "Model.h"
#include "DrawingObjects.h"
#include <vector>

class IsoSurface {
    public:
        IsoSurface( Model *, DATA_TYPE *, double, vector<bool>&, int );
        ~IsoSurface();
        void draw();
        GLfloat *color(){ return colour; }
        double isoval(){ return _val; } 
        void color(const GLfloat *c){memmove(colour,c, 4*sizeof(GLfloat));}
		int tm() const { return _tm; }
    private:
        vector<MultiPoint *> polygon;
        GLfloat              colour[4]; 
        double              _val;          // value
		int                 _tm;           // time when surface calculated
};

#endif


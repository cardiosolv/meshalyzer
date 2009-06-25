#ifndef ISOSURFACE_H
#define ISOSURFACE_H

#include "Model.h"
#include "DrawingObjects.h"
#include <vector>

class IsoSurface {
    public:
        IsoSurface( Model *m, DATA_TYPE *dat, double v, vector<bool>& );
        ~IsoSurface();
        void draw();
        GLfloat *color(){ return colour; }
        double isoval(){ return _val; } 
        void color(const GLfloat *c){memmove(colour,c, sizeof(colour) );}
    private:
        vector<MultiPoint *> polygon;
        GLfloat              colour[4];
        double              _val;
};

#endif

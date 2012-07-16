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
    private: // 
	    void determine_vert_norms(PPoint& pt);
	
    private: // member variables
      vector<SurfaceElement*> polygon;
	  GLfloat* _vertnorm;   //!< vertex normals
	  int*     _vert;       //!< vertices for which normals are computed
      GLfloat   colour[4];  //!< color of surface
      double   _val;        //!< data value of surface
      int      _tm;         //!< time when surface calculated
};

#endif


#ifndef ISOSURFACE_H
#define ISOSURFACE_H

#include "Model.h"
#include "DrawingObjects.h"
#include <vector>

class IsoSurface {
    public:
        IsoSurface( Model *, DATA_TYPE *, double, vector<bool>&, int, double *branch=NULL  );
        ~IsoSurface();
        void draw();
        GLfloat *color(){ return colour; }
        double isoval(){ return _val; } 
        void color(const GLfloat *c){memmove(colour,c, 4*sizeof(GLfloat));}
	    int tm() const { return _tm; }
        void saveAux( const char *fname );
    private: // 
	    void determine_vert_norms();
	
    private: // member variables
      vector<SurfaceElement*> polygon;
	  GLfloat* _vertnorm;   //!< vertex normals
      GLfloat   colour[4];  //!< color of surface
      double   _val;        //!< data value of surface
      int      _tm;         //!< time when surface calculated
      PPoint   _pts;        //!< points of surface
};

#endif


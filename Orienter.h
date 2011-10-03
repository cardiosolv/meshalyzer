#ifndef ORIENTER_H
#define ORIENTER_H

#include <FL/gl.h>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#include <FL/Fl_Double_Window.H>
#include "drawgl.h"

#include "ClipPlane.h"

class  Orienter : public Fl_Gl_Tb_Window 
{
    private:
        GLUquadricObj* gluobj;
        int           _cp; //current clipping plane
        Quaternion    _view;
        GLuint       _dl;
    public:
        Orienter(int x, int y, int w, int h, const char *l = 0);
        virtual ~Orienter(){}
        virtual void draw();
        virtual int handle( int );
        void    cp(int c);
        int     cp(){return _cp; }
        bool    setView(Quaternion v){
          if(v!=_view){_view=v;return true;}else return false;
        }
};



#endif

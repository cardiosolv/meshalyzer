#ifndef ORIENTER_H
#define ORIENTER_H

#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/Fl_Double_Window.H>
#include "drawgl.h"

#include "ClipPlane.h"

class  Orienter : public Fl_Gl_Tb_Window 
{
    private:
        GLUquadricObj* gluobj;
        int           _cp; //current clipping plane
        Quaternion    _view;
        GLuint        _dl;
        void           init_graphics();
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

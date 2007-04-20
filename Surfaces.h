#ifndef SURFACES_H
#define SURFACES_H

#include <stdio.h>
#include <stdlib.h>
#include "DrawingObjects.h"


class Surfaces {
	public:
		Surfaces(SurfaceElement **se=NULL, int st=-1, int end=0);
		GLfloat* fillcolor( ){ return _fillcolor; }
		void     fillcolor(float r, float g, float b, float a=1);
		GLfloat* outlinecolor( ){ return _outlinecolor; }
		void     outlinecolor(float r, float g, float b, float a=1);
		inline bool visible(){return is_visible;}
		inline void visible(bool a){ is_visible=a; } 
		inline bool filled( void ){  return _filled; }
		inline void filled( bool a ){ _filled=a; }
		inline bool outline( void ){  return _outline; }
		inline void outline( bool a ){ _outline=a; }
		int  start( ){ return startind; }
		void start( int a ){ startind = a; }
		int  end( ){ return endind; }
		void end( int a ) {endind = a; }
		void get_vert_norms( GLfloat *vn );
		void determine_vert_norms( Point & );
		SurfaceElement* ele( int a ){ return _ele[a]; }
	protected:
		GLfloat _fillcolor[4];
		GLfloat _outlinecolor[4];
		int startind;
		int endind;
		bool is_visible;
		bool _filled;         //!< draw filled
		bool _outline;        //!< draw the outline
		SurfaceElement** _ele; //!< list of elelments to draw
		GLfloat* _vertnorm;   //!< vertex normals
		int*     _vert;       //!< vertices for which normals are computed
};

#endif

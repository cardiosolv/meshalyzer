#include "trimesh.h"
#include <Fl/fl_file_chooser.h>
#include <fstream>

TBmeshWin ::TBmeshWin(int x, int y, int w, int h, const char *l = 0)
                : Fl_Gl_Tb_Window(x, y, w, h, l) 
{
	read_model();
	hipt = 0;
	hilight = 0;
	trackball.mouse.SetOglPosAndSize(-maxdim, maxdim, 2*maxdim, 2*maxdim );	
	trackball.size = maxdim;
}


void TBmeshWin :: highlight_tet(int a)
{
	hilight = a;
	redraw();
}


void TBmeshWin :: draw_tet( int tet )
{
	tet *= 4;
	GLfloat* a =  pts+3*tetra[tet++];
	GLfloat* b =  pts+3*tetra[tet++];
	GLfloat* c =  pts+3*tetra[tet++];
	GLfloat* d =  pts+3*tetra[tet];

	glVertex3fv( a );
	glVertex3fv( b );
	glVertex3fv( c );

	glVertex3fv( b );
	glVertex3fv( c );
	glVertex3fv( d );

	glVertex3fv( c );
	glVertex3fv( d );
	glVertex3fv( a );

	glVertex3fv( d );
	glVertex3fv( a );
	glVertex3fv( b );
}

	
void TBmeshWin :: draw()
{
	if (!valid()) {
        valid(1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPointSize( 10.0 );
		glEnable( GL_POINT_SMOOTH );
		glLineWidth( 1. );
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        //glEnable( GL_CULL_FACE );
        glEnable( GL_LINE_SMOOTH );
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glViewport(0,0,w(),h());
        glClearColor( 0, 0 , 0, 0 );
        glOrtho( -maxdim, maxdim, -maxdim, maxdim, -maxdim*10, maxdim*10 );
    	glClearColor( 0, 0 , 0, 0 );
	}
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
	glPushMatrix();
	trackball.DoTransform();
	glBegin(GL_TRIANGLES);
	glColor3f( 1, 1, 1 );
	for( int i=0; i<numtets; i++ )
		draw_tet(i); 

	/* draw_all_elements associated with a node */
	glColor3f( 0, 0, 1 );
	for( int i=0; i<4*numtets; i++ )
		if( tetra[i]==hipt )
			draw_tet( i/4 );

	/* draw highlighted element */
	glColor3f( 1, 0, 0 );
	glLineWidth(2);
	draw_tet(hilight);
	glLineWidth(1);

	glEnd();

	/* draw the highlighted point */
	if( hipt ) {
		glColor3f( 0, 1, 0 );
		glBegin( GL_POINTS );
		glVertex3fv( pts+3*hipt );
		glEnd();
	}
	glPopMatrix();
}


void TBmeshWin :: read_model()
{
	char *fn=NULL;
	while( fn == NULL )
		fn = fl_file_chooser( "Pick one", "*.pts", NULL );
	
	ifstream ifs( fn );
	ifs >> numpts;
	pts = new GLfloat[numpts*3+3];
	for( int i=3; i<=3*numpts; i+=3 ) ifs >> pts[i] >> pts[i+1] >> pts[i+2];
	ifs.close();
	sprintf( fn+strlen(fn)-3, "tetras" );
	ifstream itf(fn);
	itf >> numtets;
	int garbage;
	tetra = new int[4*numtets];
	for( int i=0; i<4*numtets; i+=4 ) 
		itf >> tetra[i] >> tetra[i+1] >> tetra[i+2] >> tetra[i+3] >> garbage;
	itf.close();

	// find maximum dimension
	maxdim = pts[0];
	for( int i=1; i<numpts*3; i++ )
		if( pts[i]>maxdim ) maxdim = pts[i];
}

int main( int argc, char *argv[] )
{
   	Fl::gl_visual(FL_RGB|FL_DOUBLE|FL_DEPTH);

 	meshwin win;
	win.winny->show();
	win.trackballwin->show();
	win.hiele_slide->show();
	Controls control;
	control.window->show();

	return Fl::run();
}

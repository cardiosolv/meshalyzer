#include "trimesh.h"
#include <string>
#include <sstream>
#include <libgen.h>

/* read in a CG (CoolGraphics) input file
 *
 * \param fin     file name 
 * \param w       display widget
 * \param control control widget
 */
void process_cg_format( char *fin, Meshwin *w, Controls* control )
{
  static int  bufsize=1024;
  char        buff[bufsize];
  
  string ptfile = fin;
  ptfile.erase( ptfile.size()-5, 5 );
  w->trackballwin->read_model(w->winny, ptfile.c_str(), true );

  // figure out the directory with the ".cg_in" file
  // all others are assumed to be in the same directory
  string dir(dirname(fin));
  dir += "/";

  // read in the surface files
  int         numtrif;
  gzFile in = gzopen( fin, "r" );
  gzgets(in, buff, bufsize); 
  sscanf( buff, "%d", &numtrif );
  gzgets(in, buff, bufsize); 			// read in the obligatory zero
  for( int i=0; i<numtrif; i++ ){
    gzgets(in, buff, bufsize );
	istringstream line( buff );
	string trifile;
	line >> trifile;
	trifile = dir+trifile+".tri";
	w->trackballwin->add_surface( trifile.c_str() );
  }
  gzclose( in );

  // compute surface normals
  w->trackballwin->model->_triele->compute_normals(0, 
		  							w->trackballwin->model->_triele->num()-1 );

  // read in the data
  string datafile=ptfile;
  datafile += "t0";
  w->trackballwin->get_data(datafile.c_str(), control->tmslider );
}


main( int argc, char *argv[] )
{
	Fl::gl_visual(FL_RGB|FL_DOUBLE|FL_DEPTH|FL_ALPHA);

	Meshwin win;
	Controls control;

	if( argc>1 && strstr( argv[1], ".cg_in" ) != NULL )
		process_cg_format( argv[1], &win, &control );
	else
	  win.trackballwin->read_model(win.winny, argc>=2?argv[1]:0);

	win.trackballwin->controlwin( &control );
	control.outputwin(win.trackballwin);
	bool vectordata=false;
	
	string dir = argc>=2? dirname(argv[1]) : ".";
	dir += "/";
	
	// deal with command line files specified
	for( int i=2; i<argc; i++ ) {
	  if( strstr( argv[i], ".tri" ) != NULL ){
		if( win.trackballwin->add_surface(argv[i])< 0 ) {
		  string altdir = dir;
		  altdir += argv[i];
		  win.trackballwin->add_surface(altdir.c_str());
		}
	  } else if( strstr( argv[i], ".dat" ) != NULL )
		win.trackballwin->get_data(argv[i]);
	  else if( strstr( argv[i], ".xfrm" ) != NULL )
		win.trackballwin->trackball.read( argv[i] );
	  else if( strstr( argv[i], ".mshz" ) != NULL )
		control.restore_state( argv[i] );
	  else if( strstr( argv[i], ".vpts" ) != NULL )
		vectordata = !win.trackballwin->getVecData(control.tmslider, argv[i]);
	  else
		win.trackballwin->get_data(argv[i], control.tmslider );
	}
	
	win.winny->show();
	win.trackballwin->show();
	for( int i=0; i<win.trackballwin->model->numSurf; i++ ) {
		char s[256];
		sprintf( s, "%d", i );
		control.currsurf->add( s, 0, NULL );
	}
	if( vectordata ) control.vectorgrp->activate();
	control.tethi->maximum( win.trackballwin->model->numVol()-1 );
	control.elehi->maximum( win.trackballwin->model->numVol()-1 );
	control.cabhi->maximum( win.trackballwin->model->_cable->num()-1 );
	control.verthi->maximum(win.trackballwin->model->pt.num()-1 );
	control.cnnxhi->maximum(win.trackballwin->model->_cnnx->num()-1 );
	control.mincolval->value(win.trackballwin->cs->min());
	control.maxcolval->value(win.trackballwin->cs->max());
	control.set_tet_region( win.trackballwin->model );
	control.window->show();
	if( vectordata ) control.vectorgrp->activate();

	return Fl::run();
}


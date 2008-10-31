#include "trimesh.h"
#include <string>
#include <sstream>
#include <libgen.h>
#include <getopt.h>

/** output usage 
 */
void
print_usage(void) 
{
  cout << "meshalyzer [options] model_base[.] [file.dat] [file.xfrm] [file.mshz] [file.vpts]"<<endl;
  cout << "with options: " << endl;
  cout << "--iconifycontrols|-i  iconify controls on startup" << endl;
  cout << "--no_elem|-n          do not read eleemnt info" << endl;
  cout << "--help|-h             print this message" << endl;
  exit(0);
}


/* read in a CG (CoolGraphics) input file
 *
 * \param fin     file name
 * \param w       display widget
 * \param control control widget
 */
void process_cg_format(char *fin, Meshwin *w, Controls* control, bool no_elems)
{
  static int  bufsize=1024;
  char        buff[bufsize];

  string ptfile = fin;
  ptfile.erase( ptfile.size()-5, 5 );
  w->trackballwin->read_model(w->winny, ptfile.c_str(), no_elems, true );

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
  for ( int i=0; i<numtrif; i++ ) {
    gzgets(in, buff, bufsize );
    istringstream line( buff );
    string trifile;
    line >> trifile;
    trifile = dir+trifile+".tri";
    w->trackballwin->add_surface( trifile.c_str() );
  }
  gzclose( in );

  // compute surface normals
  //w->trackballwin->model->_triele->compute_normals(0,
  //w->trackballwin->model->_triele->num()-1 );

  // read in the data
  string datafile=ptfile;
  datafile += "t0";
  w->trackballwin->get_data(datafile.c_str(), control->tmslider );
}


static struct option longopts[] = {
  { "iconifycontrols", no_argument, NULL, 'i' },
  { "no_elem"        , no_argument, NULL, 'n' },
  { "help"           , no_argument, NULL, 'h' },
  { NULL             , 0          , NULL, 0   }
};


main( int argc, char *argv[] )
{
  Fl::gl_visual(FL_RGB|FL_DOUBLE|FL_DEPTH|FL_ALPHA);

  Meshwin win;
  Controls control;
  bool iconcontrols = false;
  bool no_elems  = false;

  int ch;
  while( (ch=getopt_long(argc, argv, "inh", longopts, NULL)) != -1 )
	switch(ch) {
		case 'i':
			iconcontrols = true;
			break;
        case 'n':
			no_elems = true;
			break;
		case 'h':
			print_usage();
			break;
		default:
			break;
	}

  int model_index=optind;
  while( model_index<argc && argv[model_index][0]=='-' )
	model_index++;

  if (  model_index<argc && strstr( argv[model_index], ".cg_in" ) != NULL )
    process_cg_format( argv[1], &win, &control, no_elems );
  else
    win.trackballwin->read_model( win.winny,
			model_index<argc?argv[model_index]:0, no_elems );

  win.trackballwin->controlwin( &control );
  control.outputwin(win.trackballwin);
  bool vectordata=false;

  string dir = argc>=2? dirname(argv[1]) : ".";
  dir += "/";

  // deal with command line files specified
  for ( int i=model_index+1; i<argc; i++ ) {
    if ( argv[i][0] = '-' ) 
      continue;
    if ( strstr( argv[i], ".tri" ) != NULL ) {
      if ( win.trackballwin->add_surface(argv[i])< 0 ) {
        string altdir = dir;
        altdir += argv[i];
        win.trackballwin->add_surface(altdir.c_str());
      }
    } else if ( strstr( argv[i], ".dat" ) != NULL )
      win.trackballwin->get_data(argv[i]);
    else if ( strstr( argv[i], ".xfrm" ) != NULL )
      win.trackballwin->trackball.read( argv[i] );
    else if ( strstr( argv[i], ".mshz" ) != NULL )
      control.restore_state( argv[i] );
    else if ( strstr( argv[i], ".vpts" ) != NULL )
      vectordata = !win.trackballwin->getVecData(control.tmslider, argv[i]);
    else
      win.trackballwin->get_data(argv[i], control.tmslider );
  }

  win.winny->show();
  win.trackballwin->show();
  for ( int i=0; i<win.trackballwin->model->numSurf(); i++ ) {
    char s[256];
    sprintf( s, "%d", i );
    control.surflist->add( s, 1 );
  }
  if ( vectordata ) control.vectorgrp->activate();
  control.tethi->maximum( win.trackballwin->model->numVol()-1 );
  control.elehi->maximum( win.trackballwin->model->numVol()-1 );
  control.cabhi->maximum( win.trackballwin->model->_cable->num()-1 );
  control.verthi->maximum(win.trackballwin->model->pt.num()-1 );
  control.cnnxhi->maximum(win.trackballwin->model->_cnnx->num()-1 );
  control.mincolval->value(win.trackballwin->cs->min());
  control.maxcolval->value(win.trackballwin->cs->max());
  control.set_tet_region( win.trackballwin->model );
  control.window->show();
  if ( vectordata ) control.vectorgrp->activate();
  if ( iconcontrols )
    control.window->iconize();
  win.winny->position(1,1);

  return Fl::run();
}


#include <GLee.h>
#include "trimesh.h"
#include "isosurf.h"
#include <string>
#include <sstream>
#include <libgen.h>
#include <getopt.h>
#include <FL/Fl_Text_Display.H>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef USE_HDF5
#include <hdf5.h>
#include <ch5/ch5.h>
#endif

static Controls *ctrl_ptr;
static Meshwin  *win_ptr;

sem_t *meshProcSem;               // global semaphore for temporal linking
sem_t *linkingProcSem;            // global semaphore for process linking

#define SET_HI( A, B ) \
    if( win.trackballwin->model->number(B) ) \
        control.A##hi->maximum( win.trackballwin->model->number(B)-1 ); \
    else \
        control.A##hi->deactivate();

/** animate in response to a signal received: SIGUSR1 for forward, 
 *  SIGUSR2 for backward
 *
 * \param sig the signal
 */
void animate_signal( int sig, siginfo_t *si, void *v ) 
{
  int fs = ctrl_ptr->frameskip->value();
  fs *= sig==SIGUSR1 ? 1 : -1;

  int newtm = ctrl_ptr->tmslider->value() + fs;

  if( newtm<0 )
    newtm = ctrl_ptr->tmslider->maximum();
  if( newtm> ctrl_ptr->tmslider->maximum() )
    newtm = 0;

  win_ptr->trackballwin->set_time( newtm );
  ctrl_ptr->tmslider->value(newtm);

  sem_post( meshProcSem );
}

/** animate in response to a signal received: SIGUSR1 for forward, 
 *  SIGUSR2 for backward
 *
 * \param sig the signal
 */
void process_linkage_signal( int sig, siginfo_t *si, void *v ) 
{
  // if the signal is SIGALRM, it a new message queue is connected
  // to this process, create bi-directional linking

  // 1. call createBiDirectionalMessageQueue
  if (sig != SIGALRM){
    return;
  }

  win_ptr->trackballwin->CheckMessageQueue();  

  // receive msg
  sem_post( linkingProcSem );
}

/** read in the version and license information
 *
 * \param text text widget
 */
void
read_version_info( Fl_Text_Display *txt )
{
  txt->buffer( new Fl_Text_Buffer() );
  ifstream ifs( "version.txt" );
  string line;
  txt->insert_position(0);
  while( getline( ifs, line ) ){
    txt->insert( line.c_str() );
    txt->insert( "\n" );
  }
}


/* find out where model is located
 *
 * \param fn specified file name
 *
 * \return path to the model
 */
string
find_model_dir( string fn )
{
  if( fn.empty() ) return "";

  if( ifstream(fn.c_str()) ) return fn;
  if( ifstream((fn+"pts").c_str()) ) return fn;
  if( ifstream((fn+"pts.gz").c_str()) ) return fn;
  if( ifstream((fn+".pts").c_str()) ) return fn;
  if( ifstream((fn+".pts.gz").c_str()) ) return fn;
  if( fn.at(0) == '/' ) return "";

  char *path_var = strdup(getenv("MESHALYZER_MODEL_DIR"));
  char *ptr = strtok( path_var, ":" );
  while( ptr ) {
    string filename = ptr;
    filename += "/" + fn;
    if( ifstream(filename.c_str()) ) return filename;
    if( ifstream((filename+"pts").c_str()) ) return filename;
    if( ifstream((filename+"pts.gz").c_str()) ) return filename;
    if( ifstream((filename+".pts").c_str()) ) return filename;
    if( ifstream((filename+".pts.gz").c_str()) ) return filename;
  }
  return "";
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

/* Reads in an HDF5 input file.
 *
 * \param fin      file name
 * \param w        display widget
 * \param control  control widget
 */
void process_h5_format(char *fin, Meshwin *w, Controls *control, bool no_elems)
{
#ifdef USE_HDF5
  hid_t file;
  
  if (ch5_open(fin, &file)) {
    cerr << "Invalid HDF5 file or not found" << endl;
    exit(1);
  }
  w->trackballwin->read_model(w->winny, file, no_elems, false);
  ch5_close(file);
#else
  assert(0);
#endif
}


/** output usage 
 */
void
print_usage(void) 
{
  cout << "meshalyzer [options] model_base[.] [file.dat] [file.xfrm] [file.mshz] [file.vpts]"<<endl;
  cout << "with options: " << endl;
  cout << "--iconifycontrols|-i  iconify controls on startup" << endl;
  cout << "--no_elem|-n          do not read element info" << endl;
  cout << "--help|-h             print this message" << endl;
  cout << "--thrdRdr|-t          force threaded data reading" << endl;
  cout << "--groupID=GID|-gGID   meshalyzer group" << endl;
  cout << "--PNG=file|-pfile     output as PNG to file and exit" << endl;
  exit(0);
}


static struct option longopts[] = {
  { "iconifycontrols", no_argument, NULL, 'i' },
  { "no_elem"        , no_argument, NULL, 'n' },
  { "help"           , no_argument, NULL, 'h' },
  { "gpoupID"        , 1          , NULL, 'g' },
  { "PNGdump"        , 1          , NULL, 'p' },
  { "thrdRdr"        , no_argument, NULL, 't' },
  { NULL             , 0          , NULL, 0   }
};


main( int argc, char *argv[] )
{
  Fl::gl_visual(FL_RGB|FL_DOUBLE|FL_DEPTH|FL_ALPHA);
#ifdef USE_HDF5
  H5Eset_auto1(NULL, NULL);// silence HDF errors
#endif


  bool iconcontrols   = false;
  bool no_elems       = false;
  bool threadedReader = false;
  char *PNGfile       = NULL;
  const char *grpID   = "0";

  int ch;
  while( (ch=getopt_long(argc, argv, "inhg:", longopts, NULL)) != -1 )
	switch(ch) {
		case 'i':
			iconcontrols = true;
			break;
        case 'n':
			no_elems = true;
			break;
        case 'g':
            grpID = strdup(optarg);
            break;
		case 'h':
			print_usage();
			break;
        case 'p':
            PNGfile = strdup(optarg);
            cerr << "Womp womp wommp : not working yet! " << endl;
            break;
        case 't':
            threadedReader = true;
            break;
		default:
			break;
	}

  Controls control;
  ctrl_ptr = &control;
  Meshwin win;
  win_ptr = &win;

  int model_index=optind;
  while( model_index<argc && argv[model_index][0]=='-' )
	model_index++;

  string model_path = find_model_dir( model_index<argc?argv[model_index]:"" );

  if ( model_path.length()>5 && model_path.rfind(".cg_in" ) == model_path.length()-6 )
    process_cg_format( argv[1], &win, &control, no_elems );
  else if ( model_path.length()>5 && model_path.rfind(".modH5" ) == model_path.length()-6)
    process_h5_format(argv[1], &win, &control, no_elems);
  else
    win.trackballwin->read_model( win.winny, model_path.c_str(), no_elems );

  SET_HI( tet,  VolEle  );
  SET_HI( ele,  SurfEle );
  SET_HI( cab,  Cable   );
  SET_HI( vert, Vertex  );
  SET_HI( cnnx, Cnnx    );

  ProgInfo info;
  read_version_info( info.infotxt );
  control.proginfo = info.proginfo;

  win.trackballwin->controlwin( &control );
  control.outputwin(win.trackballwin);
  bool vectordata=false;

  string dir = argc>=2? dirname(argv[1]) : ".";
  dir += "/";

  // look for default state
  string defstate = getenv("HOME");
  defstate += "/.default.mshz";
  struct stat buf;
  if( !stat( defstate.c_str(), &buf) )
    control.restore_state( defstate.c_str() );

  //if( !PNGfile )
  win.winny->show();
  win.trackballwin->forceThreadData( threadedReader );

  // deal with command line files specified
  for ( int i=model_index+1; i<argc; i++ ) {
    if ( argv[i][0] == '-' ) 
      continue;
    if ( strstr( argv[i], ".tri" )  != NULL  ||
         strstr( argv[i], ".surf" ) != NULL ) {
      if ( win.trackballwin->add_surface(argv[i])< 0 ) {
        string altdir = dir;
        altdir += argv[i];
        win.trackballwin->add_surface(altdir.c_str());
      }
    } else if ( strstr( argv[i], ".datH5:nodal/" ) != NULL )
      win.trackballwin->get_data(argv[i], control.tmslider );
    else if ( strstr( argv[i], ".dat" )    != NULL )
      win.trackballwin->get_data(argv[i]);
    else if ( strstr( argv[i], ".xfrm" ) != NULL )
      win.trackballwin->trackball.read( argv[i] );
    else if ( strstr( argv[i], ".mshz" ) != NULL )
      control.restore_state( argv[i] );
    else if ( strstr( argv[i], ".vpts" )    != NULL ||
              strstr( argv[i], ":vector/" ) != NULL )
      vectordata = !win.trackballwin->getVecData(control.tmslider, argv[i]);
    else if ( strstr( argv[i], ".pts_t" )    != NULL  ||
              strstr( argv[i], ":auxGrid/" ) != NULL )
      win.trackballwin->readAuxGrid( control.tmslider, argv[i]);
    else if ( strstr( argv[i], ".dynpt" )  != NULL )
      win.trackballwin->read_dynamic_pts( argv[i], control.tmslider );
    else
      win.trackballwin->get_data(argv[i], control.tmslider );
  }

  win.trackballwin->cplane->calc_intercepts();
  win.trackballwin->show();

  for ( int i=0; i<win.trackballwin->model->numSurf(); i++ ) {
    control.surflist->add( win.trackballwin->model->surface(i)->label().c_str(),
                                                                        1 );
  }
  if ( vectordata ) control.vectorgrp->activate();
  control.mincolval->value(win.trackballwin->cs->min());
  control.maxcolval->value(win.trackballwin->cs->max());
  control.set_tet_region( win.trackballwin->model );
  if( !PNGfile )
    control.window->show();
  if ( win.trackballwin->auxGrid ) control.auxgridgrp->activate();
  if ( iconcontrols )
    control.window->iconize();
  win.winny->position(1,1);

  // set up named semaphore for meshProcSem
  string semstr = "/mshz";
  semstr += grpID;
  meshProcSem = sem_open( semstr.c_str(), O_CREAT, S_IRWXU, 0 );
  if( meshProcSem==SEM_FAILED )
    cerr << "Temporal linking not possible" << endl;

  // set up signal handling
  struct sigaction sigact;
  sigact.sa_sigaction = animate_signal;
  sigact.sa_flags     = SA_SIGINFO;
  sigfillset( &sigact.sa_mask );
  // setup the signal handling
  struct sigaction sigLinkAct;
  sigLinkAct.sa_sigaction = process_linkage_signal;
  sigLinkAct.sa_flags = SA_SIGINFO;
  sigfillset( &sigLinkAct.sa_mask );

  sigaction( SIGUSR1, &sigact, NULL );
  sigaction( SIGUSR2, &sigact, NULL );
  sigaction( SIGALRM, &sigLinkAct, NULL );

  // set up named semphore for linkingProcSem
  string linkageStr = "/linkage";
  linkageStr += grpID;
  linkingProcSem = sem_open( linkageStr.c_str(), O_CREAT, S_IRWXU, 0 );
  if (linkingProcSem == SEM_FAILED)
    cerr << "Message Queue inter-process communication not possible"
	 << endl;

  void write_frame( string fname, int w, int h, TBmeshWin *tbwm );
  if( PNGfile ) {
    //Fl::wait();
    GLeeInit();
    GLeeForceLink("GL_EXT_framebuffer_object");
    write_frame( PNGfile, 455, 455, win.trackballwin );
    //write_frame( PNGfile, 455, 455, win.trackballwin );
    exit(0);
  }

  Fl::run();

  return 0;
}


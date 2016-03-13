//#include <GLee.h>
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
#include <sys/types.h>
#include <errno.h>
#include "Frame.h"
#include <unistd.h>
#ifdef USE_HDF5
#include <hdf5.h>
#include <ch5/ch5.h>
#endif

#ifdef OSMESA
#include <sys/mman.h>
pid_t  master;
#else
#include <FL/glut.H>
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


/** do clean up if ctrl+C pressed
 */
void do_cleanup( int sig, siginfo_t *si, void *v )
{
#ifndef OSMESA
    stringstream nw_name("/nw_");
    nw_name << getpid();
    stringstream fini_name("/fini_");
    fini_name << getpid();
    sem_unlink( fini_name.str().c_str() );
    sem_unlink( nw_name.str().c_str() );
#endif
  exit(0);
}


/** compute the surfaces and output them 
 *  the program should not exit if the filename
 *  begins with "+". If the filename=="(+)?/dev/null",
 *  do not write the file
 *
 * \retval true exit after call
 * \retval false do not exit
 */
bool
compute_write_surfaces( Model *model, string sf )
{
  int ns  = model->numSurf();
  model->add_region_surfaces();
  model->add_surface_from_elem( model->file().c_str() );

  bool firstplus=false;
  if(sf[0] == '+') {
    firstplus = true;
    sf.erase(0,1);
  }

  if( sf.compare("/dev/null") ) {
    if(sf[sf.length()-1] != '.')
      sf += ".";
    sf += "surf";
    ofstream of(sf);
    for( int i=ns; i<model->numSurf(); i++ )
      model->surface(i)->to_file(of);
    cout << "Finished writing " << sf << endl;
  }

  if( firstplus ) {
    for( int s=ns; s<model->numSurf(); s++ ) 
      ctrl_ptr->surflist->add( model->surface(s)->label().c_str(),1);
  }

  return firstplus;
}


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


/** output a sequence of PNG images offscreen
 *
 * \param filename file or directory (numf>1) name
 * \param f0       first frame number, -1->do not use it
 * \param numf     number of frames
 * \param tbwm     rendering window
 * \param size     image width/height
*/
void
os_png_seq( string filename, int f0, int numf, TBmeshWin *tbwm, int size, int nproc )
{
#ifndef OSMESA
  // need to create an OpenGL context
  int   argc = 1;
  char *argv = strdup("-iconic");
  glutInit(&argc, &argv);
  glutInitWindowSize(1,1);
  glutCreateWindow("Take it eaze");
#endif

  // determine first and last frame numbers
  if( f0<0 )
    f0 = tbwm->time();
  if( numf==-1 )
    numf = tbwm->max_time()-f0+1;
  int f1 = f0+numf-1;

  // strip ".png" from file name if present
  if( filename.length()>4 && (filename.substr(filename.length()-4)==".png") )
    filename.erase(filename.length()-4);
  
  // output sequence into a directory which we may need to create
  if( numf>1 ) {
    tbwm->transBgd(false);
    if( mkdir( filename.c_str(), 0744 )==-1 && errno!=EEXIST ) {
      cerr << "Exiting: Cannot create directory "<<filename<<endl;
      exit(1);
    }
    filename += "/frame"; // base name for images
  }
  
  tbwm->resize(0,0,size,size);
  Frame frame( tbwm );

  if( numf==1 ) {
    frame.write( size, size, filename, f0 );
  } else {
#ifdef OSMESA
    // create 2 semaphores and shared memory needed for interprocess communication
    stringstream nw_name("/nw");
    master = getpid();
    nw_name << master;
    sem_t *nw_cnt   = sem_open( nw_name.str().c_str(), O_CREAT, S_IRWXU, 1 );
    stringstream fini_name("/fini");
    fini_name << master;
    sem_t *finished = sem_open( fini_name.str().c_str(), O_CREAT, S_IRWXU, 0 );
    stringstream cnt_name("/cnt");
    cnt_name << master;
    int shm = shm_open(cnt_name.str().c_str(), O_CREAT|O_RDWR, 0666);
    ftruncate(shm, sizeof(int));
    int *nwr = (int*)mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0 ); 

    // fork since it is too difficult to copy tbwm with all of its dynamic allocations
    for( int i=1; i<nproc;i++ ) {
      if( !fork() ) { 
        f0 += i;
        break;
      }
    }
    
    int nw = frame.write( size, size, filename, f0, f1, nproc );

    sem_wait( nw_cnt );
    *nwr = *nwr + nw;
    // wait for everyone to finish and count the number of frames written
    sem_post( finished );
    int nfin;
    sem_getvalue( finished, &nfin );
    if( nfin==nproc ) {
      cerr << "Wrote " << *nwr << " frames\n" << endl;
      sem_unlink(nw_name.str().c_str() );
      sem_unlink(fini_name.str().c_str() );
      shm_unlink(cnt_name.str().c_str() );
    }
    sem_post( nw_cnt );

#else
    int nw = frame.write( size, size, filename, f0, f1 );
    if( nw != numf ) 
      cerr << "Only " << nw << " of " << numf << " frames written" << endl;
#endif
  }
  exit(0);
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

  char *path_var = NULL;
  char *mod_dir_env = getenv("MESHALYZER_MODEL_DIR");
  if( mod_dir_env ) 
    path_var = strdup(mod_dir_env);
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
  cout << "--PNGfile=file        output PNGs and exit" << endl;
  cout << "--frame=file          first frame for PNG dump (-1=do not set)" << endl;
  cout << "--numframe=num        number of frames to output (default=1)" << endl;
  cout << "--size=num            output size of PNG in pixels (default=512)" << endl;
  cout << "--nproc=num           #parallel procs for PNG sequences" << endl;
  cout << "--compSurf=(+)?[file] compute surfaces, +=do not exit after" << endl;
  exit(0);
}


static struct option longopts[] = {
  { "iconifycontrols", no_argument, NULL, 'i' },
  { "no_elem"        , no_argument, NULL, 'n' },
  { "help"           , no_argument, NULL, 'h' },
  { "thrdRdr"        , no_argument, NULL, 't' },
  { "gpoupID"        , 1          , NULL, 'g' },
  { "PNGfile"        , 1          , NULL, 'P' },
  { "frame"          , 1          , NULL, 'f' },
  { "numframe"       , 1          , NULL, 'N' },
  { "size"           , 1          , NULL, 's' },
  { "nproc"          , 1          , NULL, 'p' },
  { "compSurf"       , 2          , NULL, 'S' },
  { NULL             , 0          , NULL, 0   }
};


int
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
  int   pngsize        = 512;
  const char *grpID   = "0";
  int    frame0   = -1,
         numframe =  1;
  int    nproc    =  1;
  char *surfFile  = NULL;

  int ch;
  while( (ch=getopt_long(argc, argv, "ing:hP:N:f:p:ts:S::", longopts, NULL)) != -1 )
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
        case 'P':
            PNGfile = strdup(optarg);
            break;
		case 'N':
			numframe = atoi(optarg);
			break;
		case 'f':
			frame0 = atoi(optarg);
			break;
		case 'p':
			nproc = atoi(optarg);
			break;
        case 't':
            threadedReader = true;
            break;
        case 's':
            pngsize = atoi(optarg);
			break;
        case 'S' : 
            surfFile = strdup(optarg?optarg:"");
            break;
        case '?':
            cerr << "Unrecognized option --- bailing" << endl;
            exit(1);
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

  if( !PNGfile )
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
  // clean up if killed
  struct sigaction sigCleanup;
  sigCleanup.sa_sigaction = do_cleanup;
  sigCleanup.sa_flags = SA_SIGINFO;
  sigfillset( &sigCleanup.sa_mask );

  sigaction( SIGUSR1, &sigact,     NULL );
  sigaction( SIGUSR2, &sigact,     NULL );
  sigaction( SIGALRM, &sigLinkAct, NULL );

  // set up named semphore for linkingProcSem
  string linkageStr = "/linkage";
  linkageStr += grpID;
  linkingProcSem = sem_open( linkageStr.c_str(), O_CREAT, S_IRWXU, 0 );
  if (linkingProcSem == SEM_FAILED)
    cerr << "Message Queue inter-process communication not possible"
	 << endl;

  // just output images, no interaction
  if( PNGfile ) 
    os_png_seq( PNGfile, frame0, numframe, win.trackballwin, pngsize, nproc );

  if( surfFile ) {
    if( *surfFile=='\0' ) 
        surfFile = strdup(win.trackballwin->model->file().c_str());
    if(!compute_write_surfaces( win.trackballwin->model, surfFile ) )
      exit(0);
  }
  Fl::run();

  return 0;
}


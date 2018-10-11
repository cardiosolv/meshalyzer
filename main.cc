//#include <GLee.h>
#include "trimesh.h"
#include "isosurf.h"
#include <string>
#include <cstring>
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

pid_t master;
Controls *ctrl_ptr;
static Meshwin  *win_ptr;

#ifdef OSMESA
#include <sys/mman.h>
sem_t *new_cnt;

sem_t * make_sem( string base, int val=0 ) {
      base += to_string(master);
      return sem_open( base.c_str(), O_CREAT, S_IRWXU, val );
}
#else
#include <FL/glut.H>
#endif

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
#ifdef OSMESA
    cerr << "Cleaning up semaphores and shm" << endl;
    stringstream nw_name("/nw_");
    nw_name << master;
    stringstream cnt_name("/cnt_");
    cnt_name << master;
    sem_unlink( nw_name.str().c_str() );
    shm_unlink( cnt_name.str().c_str() );
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
  // if the signal is SIGALRM, a new message queue is connected
  // to this process, create bi-directional linking

  // 1. call createBiDirectionalMessageQueue
  if (sig != SIGALRM){
    return;
  }

  // temporarily block signals until this action is completed
  sigset_t intmask;
  sigemptyset(&intmask);
  sigaddset(&intmask, SIGALRM);
  sigprocmask(SIG_BLOCK, &intmask, NULL); 

  sem_post( linkingProcSem );        // signal msg received
  win_ptr->trackballwin->CheckMessageQueue();  

  sigprocmask(SIG_UNBLOCK, &intmask, NULL);
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
 * \param f0       first frame number
 * \param numf     number of frames
 * \param tbwm     rendering window
 * \param size     image width/height
 * \param nproc    number of processes concurrently writing
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
    // create shared memory needed for interprocess communication
    string cnt_name("/cnt_");
    cnt_name += to_string(master);
    int shm = shm_open(cnt_name.c_str(), O_CREAT|O_RDWR, 0666);
    ftruncate(shm, 2*sizeof(int));
    int *nwr = (int*)mmap(NULL, 2*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0 ); 
    
    int nw = frame.write( size, size, filename, f0, f1, nproc );

    // count the number of frames written on the way out
    sem_wait( new_cnt );
    nwr[0] += nw;
    if( ++nwr[1]==nproc ) {
      cerr << "Wrote " << nwr[0] << " frames\n" << endl;
      string new_name = "/nw";
      new_name += to_string(master);
      sem_unlink(new_name.c_str() );
      shm_unlink(cnt_name.c_str() );
    } else
      sem_post( new_cnt );

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
    ptr = strtok( NULL, ":"  );
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
  cout << "meshalyzer [options] model_base[.[pts]] [file.igb|file.dat|file.datH5:nodal/#] [file.xfrm] [file.mshz] "
          "[file.vpts]"<<endl;
  cout << "with options: " << endl;
  cout << "--iconifycontrols|-i  iconify controls on startup" << endl;
  cout << "--no_elem        |-n  do not read element info" << endl;
  cout << "--help           |-h  print this message" << endl;
  cout << "--thrdRdr        |-t  force threaded data reading" << endl;
  cout << "--groupID=GID    |-g  meshalyzer group" << endl;
  cout << "--PNGfile=file   |-P  output PNGs and exit" << endl;
  cout << "--frame=file     |-f  first frame for PNG dump (-1=do not set)" << endl;
  cout << "--numframe=num   |-N  number of frames to output (default=1)" << endl;
  cout << "--size=num       |-s  output size of PNG in pixels (default=512)" << endl;
  cout << "--nproc=num      |-p  #parallel procs for PNG sequences" << endl;
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
  
  // fork since it is too difficult to copy tbwm with all of its dynamic allocations
  int proc=0;
  master = getpid();
#ifdef OSMESA
  new_cnt = make_sem("/nw",1);
  for( int i=1; i<nproc;i++ ) {
    if( !fork() ) { 
      proc = i;
      new_cnt = make_sem("/nw");
      sem_wait( new_cnt );
      sem_post( new_cnt );
      break;
    }
  }
  if( !proc ) sem_post( new_cnt );
#endif

  Fl::gl_visual(FL_RGB|FL_DOUBLE|FL_DEPTH|FL_ALPHA);
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

  if( !PNGfile ) {
#ifdef OSMESA
    cerr << "PNGfile must be specified with mesalyzer!" << endl;
    exit(1);
#else
    win.winny->show();
#endif
  }
  win.trackballwin->forceThreadData( threadedReader );
  if( threadedReader )
    control.read_threaded->set( );
  else
    control.read_threaded->clear( );

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
              strstr( argv[i], ":auxGrid/" ) != NULL ) {
      if( !win.trackballwin->readAuxGrid( control.tmslider, argv[i]) )
        control.auxgridgrp->activate();
    } else if ( strstr( argv[i], ".dynpt" )  != NULL )
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
  string semstr = "/mshz_";
  semstr += to_string(getuid());
  semstr += "_";
  semstr += grpID;
  meshProcSem = sem_open( semstr.c_str(), O_CREAT, S_IRWXU, 0 );
  if( meshProcSem==SEM_FAILED )
    perror("Temporal linking not possible");

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
  string linkageStr = "/linkage_";
  linkageStr += to_string(getuid());
  linkageStr += "_";
  linkageStr += grpID;
  linkingProcSem = sem_open( linkageStr.c_str(), O_CREAT, S_IRWXU, 0 );
  if (linkingProcSem == SEM_FAILED)
    perror("Message Queue inter-process communication not possible");

  if( surfFile ) {
    if( *surfFile=='\0' ) 
        surfFile = strdup(win.trackballwin->model->file().c_str());
    if(!compute_write_surfaces( win.trackballwin->model, surfFile ) )
      exit(0);
  }

  // just output images, no interaction
  if( PNGfile )  {
    if( frame0<0 )   frame0   = win.trackballwin->time();
    if( numframe<0 ) numframe = win.trackballwin->max_time()-frame0+1;
    os_png_seq( PNGfile, frame0+proc, numframe-proc, win.trackballwin, pngsize, nproc );
  }

  Fl::run();

  return 0;
}


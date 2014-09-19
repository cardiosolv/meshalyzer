#include "DataClass.h"
#include "zlib.h"
#include "IGBheader.h"
#include <string>
#include <map>
#include <sys/types.h>
#include <regex.h>
#include <dirent.h>

#ifdef _SC_AVPHYS_PAGES
#elif defined(__APPLE__)
#include <unistd.h>
#include <mach/mach.h>
#else
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/vmmeter.h>
#endif

#ifdef USE_HDF5
#include "ch5/ch5.h"
#endif

using namespace std;


/** determine the file type
 *
 *  - FTascii: one acsii number per line
 *  - FTIGB: IGB data
 *  - FTDynPt : Dynamic point data, stored in IGB_Vec3_f format
 *  - FTfileSeqCG: Cool Graphics format of one ascii file per time, with the
 *         first line of the form <TT> t = [0-9]+</TT>. Files are named
 *         <TT>basename.t[0-9]+</TT>
 *  - FThdf5 : HDF5 data format
 *
 *  \param fn   file name
 *
 *  \return if everything fails, FTascii
 */
fileType FileTypeFinder ( const char *fn )
{
  string fname = fn;

  // Open the file:
  gzFile in;
  if ( (in=gzopen( fname.c_str(), "r")) == NULL ) {
    fname += ".gz";
    in=gzopen( fname.c_str(), "r");
  }

  if ( strstr( fn, ".igb" ) != NULL ) {

    if( in == NULL ) 
      throw(1);

    IGBheader* head;
    try {
      head = new IGBheader( in, true );
    } catch(...) {
      gzclose( in );
      return FTascii;
    }

    return FTIGB;

  } else if ( strstr( fn, ".dynpt" ) != NULL ) {

    if( in == NULL ) 
      throw(1);

    IGBheader* head = new IGBheader( in );
    int res = head->read();
    gzclose( in );

    if ( res || head->type()!=IGB_VEC3_f ) {
      return FTascii;
    } else {
      delete head;
      return FTDynPt;
    }
#ifdef USE_HDF5
  } else if ( strstr( fn, ".datH5" ) != NULL ) {
    hid_t hin;
    string filename = fn;
    filename = filename.substr(0,filename.find_last_of(":"));
    if( ch5_open( filename.c_str(), &hin ) ) 
      return FTascii;

    ch5_close( hin );
    return FThdf5;
#endif

  } else {			// might be a CG file sequence

    if( in == NULL ) 
      throw(1);

    const char* tpos = strrchr( fn, 't' ); // look for last "t" in the file name

    if ( tpos ==  NULL ) // no "t" found, therefore not CG file sequence
      return FTascii;

    // make sure all remaining characters are numbers
    while ( ++tpos<fn+strlen(fn) && *tpos>='0' && *tpos<='9' );
    if ( tpos<fn+strlen(fn) )
      return FTascii;

    // see if the first line of file is of proper format: t = number
    char buff[1024];
    int tm, numread = sscanf( gzgets( in, buff, 1024 ), "t = %d", &tm );
    gzrewind( in );
    if ( numread==1 )
      return FTfileSeqCG;
    else
      return FTascii;
  }
}


/** determine a list of files names in increasing time order for a CG file
 *  sequence
 *
 *  We will extract the time from the file name and
 *  use the STL map container to automatcially sort the list of file names
 *
 *  \param filelist container for names
 *  \param fn      the name of a file in the sequence
 *
 *  \pre \p fn is a valid file and the minimum time is zero
 *  \post \p filelist will have added to it the filenames with the times as
 *        keys
 */
void CG_file_list( map<int,string>& filelist, const char *fn )
{
  // get the directory and base name for the files
  string dirname, base = fn;
  string::size_type  slashpos = base.rfind("/");
  if ( slashpos == string::npos ) {
    dirname = ".";
  } else {
    dirname  = base.substr( 0, slashpos );
    base = base.substr( slashpos+1 );
  }
  base = base.substr( 0, base.rfind(".t")+2 );

  // set up the regular expression to identify for the file names
  regex_t preg;
  string  regex = base.substr(0,base.size()-2) + "[.]t[0-9]+";
  regcomp( &preg, regex.c_str(), REG_NOSUB|REG_EXTENDED );

  // open the directory and find all matching file names
  DIR*   dir = opendir( dirname.c_str() );
  struct dirent* direntry;
  if ( dir==NULL ) return;
  while ( (direntry=readdir(dir)) != NULL )
    if ( !regexec( &preg, direntry->d_name, 0, NULL, 0 ) ) {
      //extract the time as an int
      string timestr = direntry->d_name;
      timestr = timestr.substr( timestr.rfind(".t")+2 );
      int tm;
      sscanf( timestr.c_str(), "%d", &tm );
      filelist[tm] =  string(dirname+"/"+base+timestr);
    }
  closedir( dir );
  regfree( &preg );
}


/** return the number of time instances for a file
 *
 *  \param fn file name
 *
 *  return the number of time instances
 */
long getNumberTimes( const char* fn )
{
  map<int,string> flist;
  int retval;

  switch ( FileTypeFinder( fn ) ) {
    case FTascii:
      return 1;
    case FTfileSeqCG:
      CG_file_list( flist, fn );
      return flist.size();
    case FTIGB:
      gzFile in;
      string fname = fn;
      if ( (in=gzopen( fname.c_str(), "r")) == NULL ) {
        fname += ".gz";
        in=gzopen( fname.c_str(), "r");
      }
      IGBheader head(in,true);
      gzclose( in );
      return head.t();
  }
}


/** determine the number of free pages of memory
 *
 *  \return the number of free pages of memory
 */
long long getFreePages( )
{
  long long memoryAvail;

#ifdef _SC_AVPHYS_PAGES

  memoryAvail = sysconf(_SC_AVPHYS_PAGES);

#elif defined(__APPLE__)       // not in man pages but I got it from vm_stat.c

  vm_statistics_data_t	 vm_stat;
  mach_msg_type_number_t count  = HOST_VM_INFO_COUNT;
  mach_port_t            myhost = mach_host_self();

  /* Don't blame me for the following cast. host_statistics() actually requires
  * an array of integers but a structure of integers is passed. This
  * was taken from vm_stat.c
  */
  if (host_statistics( myhost, HOST_VM_INFO, (int *)(&vm_stat),
                       &count) != KERN_SUCCESS)
    return -1;

  memoryAvail = vm_stat.free_count;

#else    // APPLE should fix sysctl() so this works

  int vmname[] = { CTL_VM, VM_METER };
  struct vmtotal vmstat;
  size_t sz = sizeof(vmstat);

  sysctl( vmname, 2, &vmstat, &sz, NULL, 0 );

  memoryAvail = vmstat.t_free;

#endif

  return memoryAvail;
}


/** determine the grid index and grid type from a HDF5 grid name
 *
 *  \param filename[in]  grid name
 *  \param gridtype[out] grid type
 *  \param index[out]    index of grid
 *
 *  \note Grid names are of the form <TT>file:grid_type/index</TT>
 *
 *  \return 0 on success, 1 o.w.
 */
int
parse_HDF5_grid( const char *filename, string& type, unsigned int& index )
{
  string fn = filename;

  int ip = fn.find_last_of( "/" );
  if( ip == string::npos )
    return 1;
  index = atoi( fn.substr(ip+1).c_str() );
  int gt = fn.find_last_of( ":" );
  if( ip < gt+2 )
    return 1;
  type = fn.substr( gt+1, ip-gt-1 );
  return 0;
}


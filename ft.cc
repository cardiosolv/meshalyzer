#include "DataClass.h"
#include "zlib.h"
#include "IGBheader.h"
#include <string>
#include <map>
#include <sys/types.h>
#include <regex.h>
#include <dirent.h>
using namespace std;


/** determine the file type
 *
 *  - FTascii: one acsii number per line
 *  - FTIGB: IGB data
 *  - FTfileSeqCG: Cool Graphics format of one ascii file per time, with the
 *         first line of the form <TT> t = [0-9]+</TT>. Files are named
 *         <TT>basename.t[0-9]+</TT>
 *
 *  \param fn   file name
 *  \param in   stream pointer
 *  \param head IGB object pointer
 *  
 *  \return if everything fails, FTascii
 */
FileType FileTypeFinder ( const char *fn ) 
{
  string fname = fn;
				 
  // Open the file:
  gzFile in;
  if ( (in=gzopen( fname.c_str(), "r")) == NULL ){
    fname += ".gz";
    if ( (in=gzopen( fname.c_str(), "r")) == NULL )
      throw( 1 );
  }

  if( strstr( fn, ".igb" ) != NULL ) {

	IGBheader* head = new IGBheader( in );
	int res = head->read();

	gzclose( in );

	if( res ) {
	  return FTascii;
	} else { 
	  delete head;
	  return FTIGB;
	}

  } else {			// might be a CG file sequence

	char* tpos = strrchr( fn, 't' ); // look for last "t" in the file name

	if( tpos ==  NULL ) // no "t" found, therefore not CG file sequence
	  return FTascii;

	// make sure all remaining characters are numbers
	while( ++tpos<fn+strlen(fn) && *tpos>='0' && *tpos<='9' );
	if( tpos<fn+strlen(fn) )
	  return FTascii;

	// see if the first line of file is of proper format: t = number
	char buff[1024];
	int tm, numread = sscanf( gzgets( in, buff, 1024 ), "t = %d", &tm );
	gzrewind( in );
	if( numread==1 ) 
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
  if( slashpos == string::npos ) {
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
  if( dir==NULL ) return;
  while( (direntry=readdir(dir)) != NULL )
	if( !regexec( &preg, direntry->d_name, 0, NULL, 0 ) ){
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
		  
  switch( FileTypeFinder( fn ) ) {
	  case FTascii:
		  return 1;
	  case FTfileSeqCG:
		  CG_file_list( flist, fn );
		  return flist.size();
	  case FTIGB:
		  gzFile in;
		  string fname = fn;
		  if ( (in=gzopen( fname.c_str(), "r")) == NULL ){
			fname += ".gz";
			in=gzopen( fname.c_str(), "r");
		  }
		  IGBheader head(in);
		  head.read();
		  gzclose( in );
		  return head.t();
  }
}

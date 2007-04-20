#ifndef FILESEQCGREADER_H
#define FILESEQCGREADER_H

#include "DataReader.h"

#define SCAN_STR_START "%*f %*f "

template<class T>
class FileSeqCGreader : public DataReader<T> {

 public:
  FileSeqCGreader( Master<T>* _mthread, Slave<T>* _sthread, 
		  			Maxmin<T>* _maxmin_ptr, map<int,string>& filelist );
  ~FileSeqCGreader();
  virtual void reader();
  virtual void local_maxmin();
  virtual void abs_maxmin();
  virtual void tmsr();   
  virtual void find_maxtm();
  
 private:
  char*            buff;
  static const int bufsize = 1024;
  int              block_size;
  map<int,string>  files;
  string           scanner; 
  using DataReader<T>:: mthread;
  using DataReader<T>:: sthread;
  using DataReader<T>:: maxmin_ptr;
  using DataReader<T>:: data;
  using DataReader<T>:: in;
};

template<class T>
FileSeqCGreader<T>::FileSeqCGreader ( Master<T>* _mthread, Slave<T>* _sthread, 
		Maxmin<T>* _maxmin_ptr, map<int,string>& filelist ) : 
		buff(new char[bufsize]), files(filelist)
{ 
  mthread    = _mthread;
  sthread    = _sthread;
  maxmin_ptr = _maxmin_ptr;
  block_size = mthread->slsz/BLOCK_SLSZ;
  data       = sthread->data;
  scanner    = SCAN_STR_START;
  scanner   += mthread->scanstr;
}


// Destructor
template<class T>
FileSeqCGreader<T>::~FileSeqCGreader() 
{
  gzclose( in );
  delete [] buff;
  files.clear();
}


/** read in a time slice
 *
 *  \note The file contains one less points than the number requested. We
 *        duplicate the very first point.
 *  \note The first line of the file is thrown away.
 */
template<class T>
void FileSeqCGreader<T>::reader() 
{
  if( sthread->rdtm<0 || sthread->rdtm>mthread->maxtm ) {
	sthread->rdtm = -1000;
    sthread->v_bit = false;
	return;
  }

  map<int,string>::iterator CGp = files.begin();
  for( int i=0; i<sthread->rdtm; i++ )
    CGp++;
 
  if( in!=NULL )gzclose( in );
  in = gzopen( CGp->second.c_str(), "r" );
  
  gzgets(in, buff, bufsize) == Z_NULL; //throw away first line

  if( gzgets(in, buff, bufsize) == Z_NULL );
  if( sscanf(buff, scanner.c_str(), data) != 1) return;
  data[1] = data[0];		// repeat since we added bogus first node

  for (int i=2; i<mthread->slsz; i++){
    if( gzgets(in, buff, bufsize) == Z_NULL ) break;
    if( sscanf(buff, scanner.c_str(), (data+i)) != 1) break;
  }

  sthread->data = data;
  sthread->v_bit = true;
}


/** Get the entire time series for a point.
 *
 *  \note The first point must be duplicated so we must take this into
 *        account with the offset. The offset to the 100th point, for
 *        example, actually points to the point lavbelled 101.
 */
template<class T>
void FileSeqCGreader<T>::tmsr() 
{
  if( sthread->unlock<0||sthread->unlock>=mthread->slsz ) return;
  
  map<int,string>::iterator CGp = files.begin();
  
  for( int t=0; t<=mthread->maxtm; t++ ) {

	gzFile in = gzopen( (CGp++)->second.c_str(), "r" );

	int block, offset;
	// account for first point actually being point 1 (point 0 = point 1 )
	if( sthread->unlock ) {
	  block  = (sthread->unlock-1)/block_size;
	  offset = (sthread->unlock-1)%block_size;
	} else {
	  block = 0;
	  offset = 0;
	}

	gzseek( in, maxmin_ptr->sl_ptr[t][block], SEEK_SET );

	for( int i=0; i<=offset; i++  )
      if( gzgets(in, buff, bufsize) == Z_NULL ) break;

    if( sscanf(buff, scanner.c_str(), sthread->data+t) != 1 ) throw(1);

	gzclose(in);
  }
}


template<class T>
void FileSeqCGreader<T>::abs_maxmin() 
{
  maxmin_ptr->abs_max = maxmin_ptr->lmax[0];
  maxmin_ptr->abs_min = maxmin_ptr->lmin[0];

  for( int t=1; t<=mthread->maxtm; t++ ) {
	if( maxmin_ptr->lmin[t] < maxmin_ptr->abs_min )
      maxmin_ptr->abs_min = maxmin_ptr->lmin[t];
	if( maxmin_ptr->lmax[t] > maxmin_ptr->abs_max )
      maxmin_ptr->abs_max = maxmin_ptr->lmax[t];
  }
}


/** determine local minim and maxima for each slice and build a list of
 *  offsets into various blocks of the file
 *
 *  Each time slice is divided into <TT>BLOCK_SLSZ</TT> blocks to
 *  facilitate later picking out one point from the slice. This way, if
 *  we need the thousandth point in the slice, we do not have to read in
 *  the preceding 999. We only need to start counting from the closest
 *  previous stored offset. This is helpful since ASCII files may contain 
 *  different numbers of bytes per line. For example, a time slice with 
 *  2500 points and 5 blocks will have offsets to entries 0, 500,
 *  1000, 1500 and 2000.
 *
 *  \post <TT>maxmin_ptr->lmin</TT> contains a list of minima for each
 *        time \n
 *  \post <TT>maxmin_ptr->lmax</TT> contains a list of maxima for each
 *        time
 *  \post <TT>maxmin_ptr->slptr</TT> contains a list for each time slice
 *        of the byte offsets to the beginning of each block
 *
 */
template<class T>
void FileSeqCGreader<T>::local_maxmin() 
{
  map<int,string>::iterator CGp;

  maxmin_ptr->sl_ptr  = new long int *[mthread->maxtm+1];
  maxmin_ptr->num_slc = mthread->maxtm+1;
  
  int t=0;
  for( CGp=files.begin(); CGp!=files.end(); CGp++, t++ ) {
    T   temp;

    maxmin_ptr->sl_ptr[t] = new long int[BLOCK_SLSZ];
	
	gzFile in = gzopen( CGp->second.c_str(), "r" );
	if( gzgets(in, buff, bufsize) == Z_NULL ) break; //throw away first line
	
	maxmin_ptr->sl_ptr[t][0] = gztell( in );
	if( gzgets(in, buff, bufsize) == Z_NULL ) break;
	if( sscanf(buff, scanner.c_str(), &temp) != 1 ) break;

	maxmin_ptr->lmax[t] = maxmin_ptr->lmin[t] = temp;
	
	for( int i=1; i<mthread->slsz-1; i++ ){  

	  if( !(i%block_size) )
		maxmin_ptr->sl_ptr[t][i/block_size] = gztell( in );

	  if( gzgets(in, buff, bufsize) == Z_NULL ) break;
	  if( sscanf(buff, scanner.c_str(), &temp) != 1) break;
	  
	  if(maxmin_ptr->lmax[t] < temp)
		maxmin_ptr->lmax[t] = temp;
	  if(maxmin_ptr->lmin[t] > temp)
		maxmin_ptr->lmin[t] = temp;
	}
	maxmin_ptr->lv_bit[t] = true;
	gzclose( in );
  }
  maxmin_ptr->read = true;
}


template<class T>
void FileSeqCGreader<T>::find_maxtm() 
{
  mthread->maxtm = files.size()-1;
}
  
#endif

#ifndef ASCIIREADER_H
#define ASCIIREADER_H

#include "DataReader.h"
#include "stdlib.h"
#include <cstdio>

template<class T>
class asciireader : public DataReader<T>
{

  public:
    asciireader(Master<T>* _mthread, Slave<T>* _sthread, Maxmin<T>* _maxmin_ptr);
    ~asciireader();
    virtual void reader();
    virtual void local_maxmin();
    virtual void tmsr();
    virtual void find_maxtm();

  private:
    char*            buff;
    static const int bufsize = 1024;
    int              blk_size;
    using DataReader<T>:: mthread;
    using DataReader<T>:: sthread;
    using DataReader<T>:: maxmin_ptr;
    using DataReader<T>:: data;
    using DataReader<T>:: in;
};

template<class T>
asciireader<T>::asciireader( Master<T>* _mthread, Slave<T>* _sthread,
                             Maxmin<T>* _maxmin_ptr ): buff(NULL)
{
  mthread    = _mthread;
  sthread    = _sthread;
  maxmin_ptr = _maxmin_ptr;

  // Open the file
  if ((in = gzopen((mthread->fname).c_str(), "rb")) == NULL) {
    mthread->fname += ".gz";
    if ( (in = gzopen((mthread->fname).c_str(), "rb")) == NULL)
      throw(1);
  }
  data = sthread->data;
  buff = new char[bufsize];
  blk_size = mthread->slsz/BLOCK_SLSZ;
}

//
// Destructor
template<class T>
asciireader<T>::~asciireader()
{

  gzclose( in );
  delete [] buff;
  delete [] data;
}


template<class T>
void asciireader<T>::reader()
{
  if ( sthread->rdtm<0 || sthread->rdtm>mthread->maxtm ) {
    sthread->v_bit = false;
    return;
  }

  while ( !maxmin_ptr->lv_bit[sthread->rdtm] ) {}

  gzseek(in, maxmin_ptr->sl_ptr[sthread->rdtm][0], SEEK_SET);

  for (int i=0; i<mthread->slsz; i++) {
    if ( gzgets(in, buff, bufsize) == Z_NULL ) break;
    if ( sscanf(buff, mthread->scanstr, data+i) != 1) break;
  }
  sthread->data  = data;
  sthread->v_bit = true;
}


//! extract the time series
template<class T>
void asciireader<T>::tmsr()
{
  int block  = sthread->unlock/blk_size;
  int offset = sthread->unlock%blk_size;

  for ( int t=0; t<=mthread->maxtm; t++ ) {

    gzseek( in, maxmin_ptr->sl_ptr[t][block], SEEK_SET);

    for ( int m=0; m<=offset; m++ )
      if ( gzgets(in, buff, bufsize) == Z_NULL ) break;
    if ( sscanf(buff, mthread->scanstr, sthread->data+t) != 1 ) throw(1);
  }
  gzrewind( in );
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
void asciireader<T>::local_maxmin()
{

  maxmin_ptr->sl_ptr  = new z_off_t *[mthread->maxtm+1];
  maxmin_ptr->num_slc = mthread->maxtm+1;

  for ( int t=0; t<=mthread->maxtm; t++ ) {
    T   temp;

    maxmin_ptr->sl_ptr[t]    = new z_off_t[BLOCK_SLSZ];
    maxmin_ptr->sl_ptr[t][0] = gztell( in );

    if ( gzgets(in, buff, bufsize) == Z_NULL ) break;
    if ( sscanf(buff, mthread->scanstr, &temp) != 1 ) break;
    maxmin_ptr->lmax[t] = maxmin_ptr->lmin[t] = temp;

    for ( int i=1; i<mthread->slsz; i++ ) {

      if ( !(i%blk_size) )
        maxmin_ptr->sl_ptr[t][i/blk_size] = gztell( in );

      if ( gzgets(in, buff, bufsize) == Z_NULL ) break;
      if ( sscanf(buff, mthread->scanstr, &temp) != 1) break;

      if (maxmin_ptr->lmax[t] < temp)
        maxmin_ptr->lmax[t] = temp;
      if (maxmin_ptr->lmin[t] > temp)
        maxmin_ptr->lmin[t] = temp;

    }
    maxmin_ptr->lv_bit[t] = true;
  }
  maxmin_ptr->read = true;
}


template<class T>
void asciireader<T>::find_maxtm()
{
  int line=0;
  while ( gzgets( in, buff, bufsize ) != Z_NULL ) { line++; }

  mthread->maxtm = (line/mthread->slsz)-1;
  gzrewind( in );
}

#endif

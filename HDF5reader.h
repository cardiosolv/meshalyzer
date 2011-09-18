#ifndef HDFREADER_H
#define HDFREADER_H

#include "DataReader.h"

template<class T>
class HDF5reader : public DataReader<T>
{
  public:
    HDF5reader(Master<T>* _mthread, Slave<T>* _sthread, Maxmin<T>* _maxmin_ptr);
    ~HDF5reader();
    virtual void reader();
    virtual void local_maxmin();
    virtual void tmsr();
    virtual void find_maxtm();

  private:
    using DataReader<T>::  mthread;
    using DataReader<T>::  sthread;
    using DataReader<T>::  maxmin_ptr;
    using DataReader<T>::  data;
    using DataReader<T>::  in;
    char*                  buf;
    int                    slsz;		//!< size of slice in bytes
#ifdef USE_HDF5
    hid_t                  hin;
#endif
    unsigned int           _index;
};

template<class T>
HDF5reader<T>::HDF5reader( Master<T>* _mthread, Slave<T>* _sthread,
                                                        Maxmin<T>* _maxmin_ptr ):
    DataReader<T>(_mthread,_sthread,_maxmin_ptr), buf(NULL)
{
#ifdef USE_HDF5
  string &fn = mthread->fname;

  if( ch5_open( fn.substr(0, fn.find_last_of(":")).c_str(), &hin) ) {
    throw(1);
  }
  
  string gtype;
  if( parse_HDF5_grid( fn.c_str(), gtype, _index ) || gtype != "nodal" )
    throw(1);

  struct ch5s_nodal_grid info;
  ch5s_nodal_grid_info( hin, _index, &info );

  slsz = info.num_nodes*info.node_width*sizeof(float);
  buf  = new char[slsz];
  data = sthread->data;
#endif
}


template<class T>
HDF5reader<T>::~HDF5reader()
{
#ifdef USE_HDF5
  ch5_close( hin );
#endif
  delete [] buf;
}


/** read in a particular time which is specified in the slave thread */
template<class T>
void HDF5reader<T>::reader()
{
#ifdef USE_HDF5
  ch5s_nodal_read(  hin, _index, sthread->rdtm, sthread->rdtm, data );
#endif
  sthread->data = data;
  sthread->v_bit = true;
}

/** find the minimum and maximum for each slice */
template<class T>
void HDF5reader<T>::local_maxmin()
{
  maxmin_ptr->read = true;
  int slice_size = mthread->slsz;

  // Read in time slices then check for max and min
  for (int i=0; i<=mthread->maxtm; i++) {

#ifdef USE_HDF5
    ch5s_nodal_read(  hin, _index, i, i, data );
#endif

    maxmin_ptr->lmin[i] = data[0];
    maxmin_ptr->lmax[i] = data[0];

    for (int j=1; j<slice_size; j++) {
      if ( data[j] > maxmin_ptr->lmax[i] )
        maxmin_ptr->lmax[i] = data[j];
      if ( data[j] < maxmin_ptr->lmin[i] )
        maxmin_ptr->lmin[i] = data[j];
    }
    maxmin_ptr->lv_bit[i] = true;
  }
}


/** get the time series for a point */
template<class T>
void HDF5reader<T>::tmsr()
{
#ifdef USE_HDF5
  ch5s_nodal_read_time_series( hin, _index, sthread->unlock, sthread->data );
#endif
}


/**  Find the maximum time (one less than the number of frames */
template <class T>
void HDF5reader<T>::find_maxtm()
{
#ifdef USE_HDF5
  struct ch5s_nodal_grid info;
  ch5s_nodal_grid_info( hin, _index, &info );
  mthread->maxtm = info.time_steps-1;
#endif

  if ( !mthread->maxtm  ) {
    free( data );
    throw( 1 );
  }
}

#endif

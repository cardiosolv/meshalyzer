#ifndef THREADED_DATA_H
#define THREADED_DATA_H
#ifndef _MULTI_THREADED
#define _MULTI_THREADED
#endif

#include "DataClass.h"
#include "IGBheader.h"
#include <zlib.h>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <time.h>


#define READ_AHEAD  2            //!< Number of time slice to read ahead(>=1)
#define READ_BEHIND 2            //!< Number of time slice to read behind(>=0)

#define THREADS (READ_AHEAD+READ_BEHIND+1)    //!< Number of threads

const struct timespec sleepwait = {0, 10000};

//! class to hold maxima and minima
template<class T>
class Maxmin
{
  public:
    Maxmin():sl_ptr(NULL), lmax(NULL), lmin(NULL), lv_bit(NULL), read(false),
        v_bit_local(false), v_bit_abs(false), num_slc(0){}
    ~Maxmin();
    bool        read;		  //!< offsets calculated
    z_off_t**   sl_ptr;       //!< Slice pointer (byte offset)
    int         num_slc;	  //!< number of time slices
    T*          lmax;         //!< Local maxima
    T*          lmin;         //!< Local minima
    bool*       lv_bit;       //!< Array of local valid bit, each element correspond to the same element on the lmax and lmin
    bool        v_bit_local;  //!< Valid bit for the entire lmax and lmin arrays
    T           abs_max;      //!< Absolute maxmimum
    T           abs_min;      //!< Absolute minimum
    bool        v_bit_abs;    //!< Valid bit for abs_max and abs_min
    void size(int n) {
      lmax=new T[n]; lmin=new T[n]; lv_bit=new bool[n];
      memset( lv_bit, 0, sizeof(bool)*n );
    }
};


template<class T>
Maxmin<T>::~Maxmin()
{
  delete[] lmax;
  delete[] lmin;
  delete[] lv_bit;
  if ( num_slc ) {
    for ( int i=0; i<num_slc; i++ ) delete[] sl_ptr[i];
    delete[] sl_ptr;
  }
}


//! class to hold data common to all threads
template<class T>
class Master
{
  public:
    Master(const char *fn,int s):slsz(s),fname(fn),maxtm(0),maxmin(NULL){}
    fileType    ftype;      //!< What type of file is it?
    string      fname;      //!< Argument sent to the class
    const char* scanstr;    //!< Scan string
    int         slsz;       //!< \#points in a time slice (not bytes)
    int         maxtm;      //!< Max time (=\#slices-1)
    Maxmin<T>*  maxmin;     //!< Pointer to maxmin structure
};


// need this line because of circular references
template<class T>
class DataReader;


//! class to hold thread specific data
template<class T>
class Slave
{
  public:
    Slave(Master<T>*m=NULL, int datasize=0);
    ~Slave(){ pthread_cancel( threadID ); delete datareader; delete[] data; }
    Master<T>*       mthread;        // PPointer to master
    int              rdtm;           // Read time
    T*               data;           // thread data buffer
    int              size;           // size of data buffer
    bool             v_bit;          // Valid bit for data
    pthread_t        threadID;       // Thread ID
    sem_t            start;          // Semaphore to start slave task
    sem_t            done;           // Semaphore to signal slave done
    int              unlock;         // Indicates slave to post a done semaphore when finished
    pthread_mutex_t  mutex_slave;    // Mutex to slave
    DataReader<T>*   datareader;     // pointer to derived DataReader class
    void resize( int s ){if (size)delete data; data = new T[s];size=s;}
    void master(Master<T>*m,int s){mthread=m;resize(s);}
};

template<class T>
Slave<T>::Slave( Master<T>*_mthread, int datasize ): mthread(_mthread),
    size(datasize), rdtm(-10000), v_bit(false), unlock(0), data(NULL)
{
  sem_init( &start, 0, 0 );
  sem_init( &done, 0, 0 );
  pthread_mutex_init ( &mutex_slave, NULL );
  if ( size ) data = new T[size];
}


#include "DataReader.h"
#include "IGBreader.h"
#include "asciireader.h"
#include "FileSeqCGreader.h"

// class to control threaded data reading
template<class T>
class ThreadedData : public DataClass<T>
{
    using DataClass<T> :: data;
    using DataClass<T> :: maxtm;
    using DataClass<T> :: last_tm;
    using DataClass<T> :: slice_size;
    using DataClass<T> :: filename;
    using DataClass<T> :: _dt;
    using DataClass<T> :: _t0;
  public:
    ThreadedData( const char *fn, int slsz, bool tm_anal=true );
    ~ThreadedData( );
    static    void*   ThreadCaller( void* _sthread );  //!< read in a time slice
    static    void*   minimax( void* _sthread );       //!< read max & min times
    static    void*   tmsrCollector( void* _sthread ); //!< read in time series
    virtual   T       max(int);	        //!< Maximum value at a slice of time
    virtual   T       max();	        //!< Maximum value of the entire series
    virtual   T       min(int);         //!< Minimum value at a slice of time
    virtual   T       min();            //!< Minimum value of the entire series
    virtual   T*      slice(int);       //!< PPointer to slice of data at a time
    virtual   void    time_series( int, T* );    //!< PPointer to time series
    virtual   void    increment(int increment);  //!< Sets increment
  private:
    Master<T>*        mthread;              //!< master thread
    Slave<T>*         sthread;              //!< slice reading threads
    Slave<T>*         stmsr;                //!< thread to read time series
    Maxmin<T>*        maxmin;               //!< Pointer to maxmin
    int               incrementation;       //!< Increment value
    pthread_mutex_t   mutex_incrementation; //!< Mutex to incrementation
    int               element;              //!< Value to decide the next thread
    gzFile            in;                    //!< file to read
    bool              replaceable( Slave<T>*, int );
};


/**Constructor
 *
 * \param fn      filename
 * \param slsz    number of points in a slice
 * \param tm_anal do min/max analysis of time series?
 */
template<class T>
ThreadedData<T>::ThreadedData( const char *fn, int slsz, bool tm_anal ):
    mthread( new Master<T>(fn,slsz)),incrementation(1),element(0)
{

  pthread_mutex_init( &mutex_incrementation, NULL );
  slice_size = slsz;
  _dt         = 1;
  _t0         = 1;

  mthread->ftype = FileTypeFinder( fn );
  mthread->maxmin = maxmin = new Maxmin<T>;

  // ugly but I don't know what else to do besides specialization which is ugly
  if     ( typeid(T) == typeid(double) )  mthread->scanstr = "%lf";
  else if ( typeid(T) == typeid(float) )  mthread->scanstr = "%f";
  else if ( typeid(T) == typeid(int) )    mthread->scanstr = "%d";
  else if ( typeid(T) == typeid(short) )  mthread->scanstr = "%hd";
  else if ( typeid(T) == typeid(long) )   mthread->scanstr = "%ld";
  else exit(1);

  // Allocation of slaves
  sthread = new Slave<T>[THREADS];
  for ( int k=0; k<THREADS; k++ )
    sthread[k].master( mthread, mthread->slsz );

  Slave<T>* sabs   = new Slave<T>( mthread );
  Slave<T>* slocal = NULL;
  if( tm_anal ) {
    stmsr  = new Slave<T>( mthread );
    slocal = new Slave<T>( mthread, mthread->slsz );
  }

///////////////////////////////////////////////////////////////////////////////
  // To add a new file type, add a case to the following switch statement.
  // Following modification must be made to accomodate a new file type.
  // 1. DataClass.h must be changed to accomodate the new file type.
  //    The new type must be included in the enumeration.
  // 3. New reader derived class.
  // 4. Add a #include filename after line 94 or after #include "asciireader.h"
  /////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  switch ( mthread->ftype ) {
    case FTIGB:
      sabs->datareader   = new IGBreader<T>(mthread, sabs, maxmin);
      slocal->datareader = new IGBreader<T>(mthread, slocal, maxmin);
      stmsr->datareader  = new IGBreader<T>(mthread, stmsr, maxmin);
      for ( int k=0; k<THREADS; k++ )
        sthread[k].datareader = new IGBreader<T>( mthread,
                                sthread+k, maxmin);
      _dt = ((IGBreader<T>*)(slocal->datareader))->dt();
      _t0 = ((IGBreader<T>*)(slocal->datareader))->org_t();
      break;
    case FTDynPt:
      sabs->datareader   = new IGBreader<T>(mthread, sabs, maxmin);
      for ( int k=0; k<THREADS; k++ )
        sthread[k].datareader = new IGBreader<T>( mthread,
                                sthread+k, maxmin);
      _dt = ((IGBreader<T>*)(sabs->datareader))->dt();
      _t0 = ((IGBreader<T>*)(sabs->datareader))->org_t();
      break;
    case FTascii:
      sabs->datareader   = new asciireader<T>(mthread, sabs, maxmin);
      slocal->datareader = new asciireader<T>(mthread, slocal, maxmin);
      stmsr->datareader  = new asciireader<T>(mthread, stmsr, maxmin);
      for ( int k=0; k<THREADS; k++ )
        sthread[k].datareader = new asciireader<T>(mthread,
                                sthread+k, maxmin);
      break;
    case FTfileSeqCG: {
      map<int,string>   CGfiles;
      CG_file_list( CGfiles, fn );
      sabs->datareader =   new FileSeqCGreader<T>( mthread,
                           sabs, maxmin, CGfiles );
      slocal->datareader = new FileSeqCGreader<T>( mthread, slocal,
                           maxmin, CGfiles);
      stmsr->datareader =  new FileSeqCGreader<T>( mthread, stmsr,
                           maxmin, CGfiles);
      for ( int k=0; k<THREADS; k++ )
        sthread[k].datareader = new FileSeqCGreader<T>( mthread,
                                sthread+k, maxmin, CGfiles);
    }
    break;
    default:
      throw(1);
  }

  filename = fn;

  // Finds and sets maxtm
  sabs->datareader->find_maxtm();
  maxtm = mthread->maxtm;
  maxmin->size(maxtm+1);

  if( tm_anal ) {
    // Call a thread to read in each time slice and get max&min times
    if (pthread_create(&(slocal->threadID), NULL, minimax, (void*)slocal) )
      throw(1);
    // Create a thread to read in time series
    if ( pthread_create( &stmsr->threadID, NULL, tmsrCollector, (void*)stmsr) )
      throw(1);
  }

  for ( int i=0; i<THREADS; i++ ) {
    if ( pthread_create(&sthread[i].threadID, NULL,
                ThreadCaller, (void*)&sthread[i]) )
      throw(1);
  }
}

/// Destructor
template<class T>
ThreadedData<T>::~ThreadedData()
{
  delete maxmin;
  delete [] sthread;
  delete stmsr;
}


/** static function to find the extrema over all times
 *
 * \param _sthread slave thread finding extrema
 */
template<class T>
void* ThreadedData<T>::minimax( void* _sthread )
{
  Slave<T>*  sthr = (Slave<T>*)_sthread;
  sthr->datareader->local_maxmin();

  sthr->mthread->maxmin->v_bit_local = true;

  // now determine absolute max and min
  sthr->mthread->maxmin->abs_max = sthr->mthread->maxmin->lmax[0];
  sthr->mthread->maxmin->abs_min = sthr->mthread->maxmin->lmin[0];

  for ( int t=1; t<=sthr->mthread->maxtm; t++ ) {
    if ( sthr->mthread->maxmin->lmin[t] < sthr->mthread->maxmin->abs_min )
      sthr->mthread->maxmin->abs_min = sthr->mthread->maxmin->lmin[t];
    if ( sthr->mthread->maxmin->lmax[t] > sthr->mthread->maxmin->abs_max )
      sthr->mthread->maxmin->abs_max = sthr->mthread->maxmin->lmax[t];
  }
  sthr->mthread->maxmin->v_bit_abs = true;
  delete sthr;
}


//! static function to extract the time series for a point
template<class T>
void* ThreadedData<T>::tmsrCollector( void* _sthread )
{
  Slave<T>* sthr = (Slave<T>*)_sthread;
  sthr->resize(sthr->mthread->maxtm+1);

  while (1) {
    sem_wait( &sthr->start );
    sthr->datareader->tmsr();
    sem_post( &sthr->done );
  }
}


// get the maximum for a particular time
template<class T>
T ThreadedData<T>::max( int tm )
{

  // If tm exceeds maxtm, error
  if ( tm>maxtm && maxtm != -1 )
    return 0;

  while ( !maxmin->lv_bit[tm] ) { nanosleep( &sleepwait, NULL); }
  return maxmin->lmax[tm];
}


// global maximum
template<class T>
T ThreadedData<T>::max()
{
  while ( !maxmin->v_bit_abs ) {nanosleep( &sleepwait, NULL);}
  return maxmin->abs_max;
}


/** minimum
 *
 * \param time slice to find minimum for
 */
template<class T>
T ThreadedData<T>::min( int tm )
{
  // If tm exceeds maxtm, error
  if ( tm>maxtm && maxtm != -1 )
    return 0;
  // When lv_bit set, send back local min
  while ( !maxmin->lv_bit[tm] ) { nanosleep( &sleepwait, NULL); }
  return maxmin->lmin[tm];
}


// global minimum
template<class T>
T ThreadedData<T>::min()
{
  while ( !maxmin->v_bit_abs ) { nanosleep(&sleepwait,NULL); }
  return maxmin->abs_min;
}


/** get the data for a particular time
 *
 *  This routine also reads slices ahead and behind into
 *  buffers to anticipate the next slice requested
 *
 *  \param tm time
 *
 *  \return a pointer to an array with the data
 */
template<class T>
T* ThreadedData<T>::slice( int tm )
{
  if ( tm>maxtm && maxtm != -1 ) return 0;

  // Locks incrementation, copies incrementation and unlock it
  //pthread_mutex_lock( &mutex_incrementation );
  //pthread_mutex_unlock( &mutex_incrementation );

  // Searches cache for tm
  T* retval;
  bool not_found = true;
  for ( int i=0; i<THREADS; i++ ) {
    if ( sthread[i].rdtm == tm ) {
      not_found = false;
      while ( !sthread[i].v_bit ) { nanosleep( &sleepwait, NULL ); }
      retval = sthread[i].data;
      break;
    }
  }

  while ( not_found ) {
    if ( !pthread_mutex_trylock(&sthread[element].mutex_slave) ) {
      if ( replaceable( sthread+element, tm ) ) {
        sthread[element].rdtm   = tm;
        sthread[element].v_bit  = false;
        sthread[element].unlock = 1;
        sem_post( &sthread[element].start );
        sem_wait( &sthread[element].done );
        retval = sthread[element].data;
        element = (element+1)%THREADS;
        break;
      }
      pthread_mutex_unlock( &sthread[element].mutex_slave );
    }
    element = (element+1)%THREADS;
  }

  // fill in the look ahead locations
  for ( int j=READ_AHEAD; j>=-READ_BEHIND; j-- ) {

    int read_time;
    if ( j<0 )
      read_time = tm + j*incrementation;
    else
      read_time = tm + ((READ_AHEAD)-j+1)*incrementation;

    if ( !j || read_time<0 || read_time>maxtm ) continue;

    not_found = true;
    for ( int k=0; k<THREADS; k++ )
      if ( sthread[k].rdtm == read_time ) {
        not_found = false;
        break;
      }

    while ( not_found ) {
      if ( !pthread_mutex_trylock(&sthread[element].mutex_slave) ) {
        if ( replaceable( sthread+element, tm ) ) {
          sthread[element].rdtm = read_time;
          sthread[element].v_bit = false;
          sem_post( &sthread[element].start );
          element = (element+1)%THREADS;
          break;
        }
        pthread_mutex_unlock( &sthread[element].mutex_slave );
      }
      element= (element+1)%THREADS;
    }
  }
  return retval;
}


//! Slave thread used to obtain time slice
template<class T>
void* ThreadedData<T>::ThreadCaller( void* _sthread )
{
  Slave<T>* sthr = (Slave<T>*)_sthread;

  while (1) {
    sem_wait( &(sthr->start) );
    sthr->datareader->reader();

    if ( sthr->unlock ) {
      sthr->unlock = 0;
      sem_post( &(sthr->done) );
    }
    pthread_mutex_unlock( &sthr->mutex_slave );
  }
}


// Find the time series for a point
template<class T>
void ThreadedData<T>::time_series( int offset, T* buffer )
{
  while ( !maxmin->read ) {nanosleep( &sleepwait, NULL);}
  stmsr->unlock = offset;
  stmsr->data = buffer;
  sem_post( &stmsr->start );
  sem_wait( &stmsr->done );
}


template<class T>
void ThreadedData<T>::increment(int increment)
{
  // Lock, set incrementation, unlock
  pthread_mutex_lock( &mutex_incrementation );
  incrementation = increment;
  pthread_mutex_unlock( &mutex_incrementation );
}


/** determine if a thread contains data that can be overwritten
 *
 *  Data is replaceable if any of the following conditions are met:
 *   - the data is not valid
 *   - the time of the data is outside the look ahead (or behind range)
 *   - the time of the data isn't offset a multiple of the increment from
 *     the current time
 *
 *  \return true if the data meets any of the conditions, false o.w.
 */
template<class T>
bool ThreadedData<T>::replaceable(Slave<T> *s, int t)
{
  if ( incrementation>0 )
    return !s->v_bit || s->rdtm>t+READ_AHEAD*incrementation  ||
           s->rdtm<t-READ_BEHIND*incrementation || (s->rdtm-t)%incrementation;
  else
    return !s->v_bit || s->rdtm<t+READ_AHEAD*incrementation  ||
           s->rdtm>t-READ_BEHIND*incrementation || (s->rdtm-t)%incrementation;
}
#endif

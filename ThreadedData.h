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


#define READ_AHEAD  2            //!< Number of time slice to read ahead(>=1)
#define READ_BEHIND 2            //!< Number of time slice to read behind(>=0)

#define THREADS (READ_AHEAD+READ_BEHIND+1)    //!< Number of threads 

//! class to hold maxima and minima
template<class T>
class Maxmin{
  public:
  Maxmin():sl_ptr(NULL), lmax(NULL), lmin(NULL), lv_bit(NULL), read(false),
		   			v_bit_local(false), v_bit_abs(false), num_slc(0){}
  ~Maxmin();
  bool        read;			//!< offsets calculated
  long int**  sl_ptr;       //!< Slice pointer (byte offset)
  int         num_slc;		//!< number of time slices
  T*          lmax;         //!< Local maxima
  T*          lmin;         //!< Local minima
  bool*       lv_bit;       //!< Array of local valid bit, each element correspond to the same element on the lmax and lmin
  bool        v_bit_local;  //!< Valid bit for the entire lmax and lmin arrays
  T           abs_max;      //!< Absolute maxmimum
  T           abs_min;      //!< Absolute minimum
  bool        v_bit_abs;    //!< Valid bit for abs_max and abs_min
  void size(int n){ lmax=new T[n]; lmin=new T[n]; lv_bit=new bool[n]; 
                     memset( lv_bit, 0, sizeof(bool)*n );             }
};


template<class T>
Maxmin<T>::~Maxmin()
{
  delete[] lmax;
  delete[] lmin;
  delete[] lv_bit;
  if( num_slc ){
    for( int i=0; i<num_slc; i++ ) delete[] sl_ptr[i];
	delete[] sl_ptr;
  }
}

  
//! class to hold data common to all threads
template<class T>
class Master{
  public:
  Master(const char *fn,int s):slsz(s),fname(fn),maxtm(0),maxmin_ptr(NULL){}
  FileType    ftype;      //!< What type of file is it?
  string      fname;      //!< Argument sent to the class
  char*       scanstr;    //!< Scan string
  int         slsz;       //!< \#points in a time slice (not bytes)
  int         maxtm;      //!< Max time (=\#slices-1)
  Maxmin<T>*  maxmin_ptr; //!< Pointer to maxmin structure
};


// need this line because of circular references
template<class T>
class DataReader;


//! class to hold thread specific data
template<class T>
class Slave {
  public:
  Slave(Master<T>*m=NULL, int datasize=0);
  ~Slave(){ delete DataReader_ptr; delete[] data; }
  Master<T>*       mthread;        // Pointer to master
  int              rdtm;           // Read time
  T*               data;           // thread data buffer
  int              size;           // size of data buffer
  bool             v_bit;          // Valid bit for data 
  int              unlock;         // Indicates to post a semaphore in case when the thread is waiting.
  pthread_t        threadID;       // Thread ID
  sem_t            semaphores;     // Semaphore
  sem_t            slice;          // Semaphore
  pthread_mutex_t  mutex_slave;    // Mutex to slave
  DataReader<T>*   DataReader_ptr; // pointer to derived DataReader class
  void resize( int s ){if(size)delete data; data = new T[s];size=s;}
  void master(Master<T>*m,int s){mthread=m;resize(s);}
};

template<class T>
Slave<T>::Slave( Master<T>*_mthread, int datasize ): mthread(_mthread),
	size(datasize), rdtm(-10000), v_bit(false), unlock(0), data(NULL)
{
    sem_init( &semaphores, 0, 0 );
    sem_init( &slice, 0, 0 );
    pthread_mutex_init ( &mutex_slave, NULL );
	if( size ) data = new T[size];
}


#include "DataReader.h"
#include "IGBreader.h"
#include "asciireader.h"
#include "FileSeqCGreader.h"

// class to control threaded data reading
template<class T>
class ThreadedData : public DataClass<T> {
      using DataClass<T> :: data;
      using DataClass<T> :: maxtm;
      using DataClass<T> :: last_tm;
      using DataClass<T> :: slice_size;
      using DataClass<T> :: filename;
 public:
                    ThreadedData( const char *fn, int slsz);  
                    ~ThreadedData( );                  
  static    void*   ThreadCaller( void* _sthread );     //!< Thread to read in a requested time slice
  static    void*   absCollector( void* _sthread );     //!< Thread to read in the absolute max and min times
  static    void*   localCollector( void* _sthread );   //!< Thread to read in the local max and min times
  static    void*   tmsrCollector( void* _sthread );    //!< Thread to read in a requested time series
  virtual   T       max(int);	        //!< Maximum value at a slice of time
  virtual   T       max();	            //!< Maximum value of the entire series
  virtual   T       min(int);           //!< Minimum value at a slice of time
  virtual   T       min();              //!< Minimum value of the entire series
  virtual   T*      slice(int);         //!< Pointer to slice of data at a time 
  virtual   void    time_series( int, T* );    //!< Pointer to time series
  virtual   void    increment(int increment);  //!< Sets increment
 private:
  Master<T>*        mthread;               //!< Pointer to master
  Slave<T>*         sthread;               //!< Pointer to slave
  Slave<T>*         stmsr;                 //!< Pointer to slave
  Maxmin<T>*        maxmin_ptr;            //!< Pointer to maxmin
  int               incrementation;        //!< Incremenationl value
  pthread_mutex_t   mutex_incrementation;  //!< Mutex to incrementation
  int               element;               //!< Value to decide the next thread
  gzFile            in;
  bool              replaceable( Slave<T>*, int );
};


/**Constructor
 *
 * \param fn   filename
 * \param slsz number of points in a slice
 */
template<class T>
ThreadedData<T>::ThreadedData( const char *fn, int slsz ): 
           mthread( new Master<T>(fn,slsz)),incrementation(1),element(0) {
  
  pthread_mutex_init( &mutex_incrementation, NULL );
  slice_size=slsz;

  mthread->ftype = FileTypeFinder( fn );
  mthread->maxmin_ptr = maxmin_ptr = new Maxmin<T>;
 
  // ugly but I don't know what else to do besides specialization which is ugly 
  if     ( typeid(T) == typeid(double) ) mthread->scanstr = "%lf";
  else if( typeid(T) == typeid(float) )  mthread->scanstr = "%f";
  else if( typeid(T) == typeid(int) )    mthread->scanstr = "%d";
  else if( typeid(T) == typeid(short) )  mthread->scanstr = "%hd";
  else if( typeid(T) == typeid(long) )   mthread->scanstr = "%ld";
  else exit(1);

  // Allocation of slaves
  sthread = new Slave<T>[THREADS];
  for ( int k=0; k<THREADS; k++ ) 
    sthread[k].master( mthread, mthread->slsz );
  stmsr            = new Slave<T>( mthread );
  Slave<T>* sabs   = new Slave<T>( mthread );
  Slave<T>* slocal = new Slave<T>( mthread, mthread->slsz );

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
    sabs->DataReader_ptr   = new IGBreader<T>(mthread, sabs, maxmin_ptr);
    slocal->DataReader_ptr = new IGBreader<T>(mthread, slocal, maxmin_ptr);
    stmsr->DataReader_ptr  = new IGBreader<T>(mthread, stmsr, maxmin_ptr);
    for ( int k=0; k<THREADS; k++ )
      sthread[k].DataReader_ptr = new IGBreader<T>( mthread,
			  									sthread+k, maxmin_ptr);
    break;
  case FTascii:
    sabs->DataReader_ptr   = new asciireader<T>(mthread, sabs, maxmin_ptr);
    slocal->DataReader_ptr = new asciireader<T>(mthread, slocal, maxmin_ptr);
    stmsr->DataReader_ptr  = new asciireader<T>(mthread, stmsr, maxmin_ptr);
    for ( int k=0; k<THREADS; k++ )
      sthread[k].DataReader_ptr = new asciireader<T>(mthread, 
			  									sthread+k, maxmin_ptr);
    break;
  case FTfileSeqCG:
    {
	map<int,string>   CGfiles;
	CG_file_list( CGfiles, fn );
    sabs->DataReader_ptr =   new FileSeqCGreader<T>( mthread, 
										sabs, maxmin_ptr, CGfiles );
    slocal->DataReader_ptr = new FileSeqCGreader<T>( mthread, slocal,
										maxmin_ptr, CGfiles);
    stmsr->DataReader_ptr =  new FileSeqCGreader<T>( mthread, stmsr, 
			 							maxmin_ptr, CGfiles);
    for ( int k=0; k<THREADS; k++ )
      sthread[k].DataReader_ptr = new FileSeqCGreader<T>( mthread, 
			  						sthread+k, maxmin_ptr, CGfiles);
	}
    break;
  default:
    throw(1);
  }

  filename = fn;

  // Finds and sets maxtm
  sabs->DataReader_ptr->find_maxtm();
  maxtm = mthread->maxtm;
  maxmin_ptr->size(maxtm+1);

  // Call a thread to read in global max min times
  //if ( pthread_create( &sabs->threadID, NULL, absCollector, (void*)sabs) )
    //throw(1);

  // Call a thread to read in each time slice and get max&min times
  if(pthread_create(&(slocal->threadID), NULL, localCollector, (void*)slocal) )
    throw(1);

  for ( int i=0; i<THREADS; i++ ){
    if( pthread_create(&sthread[i].threadID, NULL, 
								ThreadCaller, (void*)&sthread[i]) )
      throw(1);
  }

  // Create a thread to read in slices
  if( pthread_create( &stmsr->threadID, NULL, tmsrCollector, (void*)stmsr) ) 
    throw(1);
}

/// Destructor
template<class T>
ThreadedData<T>::~ThreadedData() 
{
  delete maxmin_ptr;
  delete [] sthread;  
  delete stmsr;
}


//! static function to find the extrema over all times
template<class T>
void* ThreadedData<T>::absCollector( void* _sthread ) 
{
  Slave<T>*  sthr = (Slave<T>*)_sthread;

  while( !sthr->mthread->v_bit_local ) {}
  sthr->DataReader_ptr->abs_maxmin();
  sthr->mthread->maxmin_ptr->v_bit_abs = true;
  delete sthr;
}


//! static function to find the extrema for each time
template<class T>
void* ThreadedData<T>::localCollector( void* _sthread )
{
  Slave<T>*  sthr = (Slave<T>*)_sthread;
  sthr->DataReader_ptr->local_maxmin();
  sthr->mthread->maxmin_ptr->v_bit_local = true;
  delete sthr;
}


//! static function to extract the time series for a point
template<class T>
void* ThreadedData<T>::tmsrCollector( void* _sthread ) 
{
  Slave<T>* sthr = (Slave<T>*)_sthread;
  sthr->resize(sthr->mthread->maxtm+1);

  while(1){
    sem_wait( &sthr->semaphores );
	while( !sthr->mthread->maxmin_ptr->v_bit_local ){}
    sthr->DataReader_ptr->tmsr();
    sem_post( &sthr->slice );
  }
}


// get the maximum for a particular time
template<class T>
T ThreadedData<T>::max( int tm ) {

  // If tm exceeds maxtm, error
  if( tm>maxtm && maxtm != -1 )
    return 0;

  // If v_bit_local is 1 send back local max
  if( maxmin_ptr->v_bit_local )
    return maxmin_ptr->lmax[tm];

  // When lv_bit == 1, send back localmax
  while( !maxmin_ptr->lv_bit[tm] ){}    
  return maxmin_ptr->lmax[tm];  
}


// global maximum
template<class T>
T ThreadedData<T>::max() 
{
  // When v_bit_abs, send back abs max
  while( !maxmin_ptr->v_bit_abs ) {}
  return maxmin_ptr->abs_max;
}


template<class T>
T ThreadedData<T>::min( int tm ) 
{
  // If tm exceeds maxtm, error
  if( tm>maxtm && maxtm != -1 )
    return 0;
  // If v_bit_local is set, send back local min
  if( maxmin_ptr->v_bit_local )
    return maxmin_ptr->lmin[tm];
  // When lv_bit set, send back local min
  while( !maxmin_ptr->lv_bit[tm] ){}    
  return maxmin_ptr->lmin[tm];      
}


// global minimum
template<class T>
T ThreadedData<T>::min() 
{
  while( !maxmin_ptr->v_bit_abs ) {}
  return maxmin_ptr->abs_min;
}

template<class T>
T* ThreadedData<T>::slice( int tm )
{
  if( tm>maxtm && maxtm != -1 ) return 0;

  // Locks incrementation, copies incrementation and unlock it
  pthread_mutex_lock( &mutex_incrementation );
  pthread_mutex_unlock( &mutex_incrementation );
  
  // Searches cache for tm
  T* retval;
  bool not_found = true;
  for ( int i=0; i<THREADS; i++ ){
    if( sthread[i].rdtm == tm ){
	  not_found = false;
      while( !sthread[i].v_bit ){}
	  retval = sthread[i].data;    
	  break;
    }
  }

  while( not_found ) {
	if( !pthread_mutex_trylock(&sthread[element].mutex_slave) ){
	  if( replaceable( sthread+element, tm ) ){
		sthread[element].rdtm   = tm;
		sthread[element].v_bit  = false;	
		sthread[element].unlock = 1;
		sem_post( &sthread[element].semaphores );
		sem_wait( &sthread[element].slice );
		retval = sthread[element].data;    
		element = (element+1)%THREADS;
		break;
	  }
	  pthread_mutex_unlock( &sthread[element].mutex_slave );
	}
	element = (element+1)%THREADS;
  }

  // fill in the look ahead locations
  for( int j=-READ_BEHIND; j<=READ_AHEAD; j++ ){

	int read_time = tm + j*incrementation;

	if( !j || read_time<0 || read_time>maxtm ) continue;

    not_found = true;
	for( int k=0; k<THREADS; k++ )
	  if( sthread[k].rdtm == read_time ) {
		not_found = false;
		break;
	  }

	while( not_found ) {
	  if( !pthread_mutex_trylock(&sthread[element].mutex_slave) ){	
		if( replaceable( sthread+element, tm ) ){
		  sthread[element].rdtm = read_time;
		  sthread[element].v_bit = false;
		  sem_post( &sthread[element].semaphores );
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


// Slave thread used to obtain time slice
template<class T>
void* ThreadedData<T>::ThreadCaller( void* _sthread ) 
{
  Slave<T>* sthr = (Slave<T>*)_sthread;

  while(1){
    sem_wait( &(sthr->semaphores) );
    sthr->DataReader_ptr->reader();
    
    if( sthr->unlock ){
      sem_post( &(sthr->slice) );
      sthr->unlock = 0;
    }
    pthread_mutex_unlock( &sthr->mutex_slave );
  }
}


// Find the time series for a point
template<class T>
void ThreadedData<T>::time_series( int offset, T* buffer ) 
{
  while ( !maxmin_ptr->read ) {}
  stmsr->unlock = offset;
  stmsr->data = buffer;
  sem_post( &stmsr->semaphores );
  sem_wait( &stmsr->slice );
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
  if( incrementation>0 )
    return !s->v_bit || s->rdtm>t+READ_AHEAD*incrementation  || 
		   s->rdtm<t-READ_BEHIND*incrementation || (s->rdtm-t)%incrementation;  
  else
    return !s->v_bit || s->rdtm<t+READ_AHEAD*incrementation  || 
		   s->rdtm>t-READ_BEHIND*incrementation || (s->rdtm-t)%incrementation;  
}
#endif

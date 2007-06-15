#ifndef IGBREADER_H
#define IGBREADER_H

#include "DataReader.h"

template<class T>
class IGBreader : public DataReader<T> {

 public:
  IGBreader( Master<T>* _mthread, Slave<T>* _sthread, Maxmin<T>* _maxmin_ptr );
  ~IGBreader();
  virtual void reader();
  virtual void local_maxmin();
  virtual void tmsr();
  virtual void  find_maxtm();

 private:
  using DataReader<T>::    mthread;
  using DataReader<T>::    sthread;
  using DataReader<T>::    maxmin_ptr;
  using DataReader<T>::  data;
  using DataReader<T>::  in;
  char*      buf;
  int        slsz;		//!< size of slice in bytes
  IGBheader* head;
};

template<class T>
IGBreader<T>::IGBreader( Master<T>* _mthread, Slave<T>* _sthread, 
												Maxmin<T>* _maxmin_ptr ) 
{
  buf = 0;
  mthread = _mthread;
  sthread = _sthread;
  maxmin_ptr = _maxmin_ptr;

  // Open the file
  if ((in = gzopen((mthread->fname).c_str(), "rb")) == NULL){
    mthread->fname += ".gz";
    if( (in = gzopen((mthread->fname).c_str(), "rb")) == NULL)
      throw(1);
  }

  // Assign head, slsz and buf if it is an IGB file  
  head = new IGBheader( in );
  head->read();
  slsz = head->data_size()*head->x()*head->y()*head->z();
  buf = new char[slsz];
  data = sthread->data;
}


template<class T>
IGBreader<T>::~IGBreader() {

  gzclose( in );
  delete head;
  delete [] buf;
  delete [] data;
}


/** read in a particular time which is specidfied in the slave thread */
template<class T>
void IGBreader<T>::reader() {

  // make sure all intermediate results are not truncated
  z_off_t seekbyte = sthread->rdtm;
  seekbyte *= slsz;
  seekbyte += 1024;

  gzseek( in, seekbyte, SEEK_SET );
  read_IGB_data(data, 1, head, buf); 

  sthread->data = data;
  sthread->v_bit = true; 
}

/** find the minimum and maximum for each slice */
template<class T>
void IGBreader<T>::local_maxmin() {

  maxmin_ptr->read = true;
  int slice_size = mthread->slsz;
  
  // Set file pointer to location 1024
  gzseek(in, 1024, SEEK_SET);

  // Read in time slices then check for max and min
  for (int i=0; i<=mthread->maxtm; i++){

    //int numread = gzread( head->fileptr(), buf, slsz );
    //read_IGB_data(data, head, numread);
    read_IGB_data(data, 1, head, buf);

	maxmin_ptr->lmin[i] = data[0];
	maxmin_ptr->lmax[i] = data[0];

	for (int j=1; j<slice_size; j++){
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
void IGBreader<T>::tmsr(){
  T temp;

  gzseek(head->fileptr(), 1024+sthread->unlock*head->data_size(), SEEK_SET );
  for( int i=0; i<=mthread->maxtm; i++ ){
    gzread( head->fileptr(), buf, head->data_size() );
	if( head->endian() != head->systeme() ) head->swab( buf, 1 );
	sthread->data[i] = head->convert_buffer_datum( buf, 0 );
	if( i<mthread->maxtm )
	  gzseek( head->fileptr(), slsz-head->data_size(), SEEK_CUR );
  }
  gzrewind(head->fileptr());
}


/**  Find the maximum time (one less than the number of frames */
template <class T>
void IGBreader<T>::find_maxtm() {
  if( mthread->fname.rfind(".gz") == mthread->fname.size()-3 ) {
	cerr << "\nPlease uncompress IGB files\n\n";
	throw(1);
  }
  struct stat result;
  stat( mthread->fname.c_str(), &result );	
  mthread->maxtm = (result.st_size - 1024)/
	  			(head->data_size()*head->x()*head->y()*head->z() ) - 1;	
  if( mthread->maxtm <= 0 ){
    free( data );
    if( mthread->ftype == FTIGB ) delete head;
    throw( 1 );  
  } 
}

#endif

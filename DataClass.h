#ifndef DATACLASS_H
#define DATACLASS_H

#include <zlib.h>
#include "IGBheader.h"
#include <map>
#include <string>
using namespace std;

enum DataReaderEnum {AllInMem, Threaded };
enum fileType { FTIGB=1, FTascii, FTfileSeqCG, FTDynPt, FTother };

fileType FileTypeFinder ( const char *fn );

void CG_file_list( map<int,string>&filelist, const char *fn );

/** The basic class for reading data */
template<class T>
class DataClass
{
  public:
    virtual T      max(int)=0;	     //!< maximum data value at a time
    virtual T      max()=0;	         //!< maximum data value at a time
    virtual T      min(int)=0;       //!< minimum data value at a time
    virtual T      min()=0;          //!< minimum data value at a time
    virtual T*     slice(int)=0;     //!< pointer to time slice of data
    virtual void   time_series( int, T* )=0; //!< time series for a point
    virtual void   increment(int)=0;         //!< time slice increment
    int    max_tm(){return maxtm;} //!< maximum allowable time
    int    slice_sz(){return slice_size;} //!< size of 1 time slice
    void   slice_sz(int a){slice_size=a;} //!< set size of slice
    string file(){return filename;}       //!< return the file
    float  t0(void){ return _t0; }        //!< initial time read
    float  dt(void){ return _dt; }        //!< time increment

    DataClass():data(NULL),maxtm(0),last_tm(-1),slice_size(0){}
    virtual ~DataClass(){}

  protected:
    T*     data;       //!< data
    int    maxtm;      //!< number of time slice
    int    last_tm;    //!< last time that can be requested
    int    slice_size; //!< amountof data in one time slice
    float  _dt;        //!< time increment
    float  _t0;        //!< initial time read in
    string filename;   //!< file containing data
};

#endif

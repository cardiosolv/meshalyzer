#ifndef DATAREADER_H
#define DATAREADER_H

#define BLOCK_SLSZ 20              //!< Number of intermediary slice pointers

/** virtual class for threaded data reading */
template<class T>
class DataReader
{

  public:
    virtual void reader()=0;
    virtual void local_maxmin()=0;
    virtual void tmsr()=0;
    virtual void find_maxtm()=0;

    DataReader():mthread(0),sthread(0),maxmin_ptr(0),data(0),in(NULL){}

  protected:
    Master<T>*    mthread;
    Slave<T>*     sthread;
    Maxmin<T>*    maxmin_ptr;
    T*            data;
    gzFile        in;
};
#endif

//////////////////////////////////////////////////////////////////////////////////
// Instruction on how to write a derived class for a new data type
//
// Constructor:
//   Given:
//      Guarnteed valid values: In master:ftype, fname, scanstr, slsz,
//                                   maxmin_ptr, this_ptr, and btw_size
//      All other values are either invalid or unkown.
//   Required:
//      Initialize anything that you may need later such as opening file
//      (to be determined by the programmer)
//
// Destructor: (Writer defined)
//
// reader():
//   Given:
//      Guarnteed valid values: master(all elements), maxmin(all elements), and slave(all elements)
//   Required:
//      From the given tm in slave->rdtm, find the data of the time slice; give the pointer location to slave->data
//      When the data is found, set slave->v_bit to 1
//
// local_maxmin():
//   Given:
//      Same given as constructor
//   Required:
//      Set maxmin->read to 1 or 0 depending whether you need to find something to read the data such as a byte offset: 1 for read now, 0 for need to find something
//      Find the max and min values of each time slice and put the result to maxmin->lmax and maxmin->lmin
//      NOTE: maxmin->lmax, maxmin->lmin is type void.  You must type cast to proper type before assigning any values
//      If necessary, find the byte offset at the beginning of each slice of time.
//      Set maxmin->del_sl_ptr to 1 or 0 depending on whether byte offset was found: 1 for found, 0 for not found
//
// tmsr():
//   Given:
//      Same given as constructor except that slave->unlock has the offset value and slave->unlock has the slave->data has the array location where the value is to be stored
//   Required:
//      Find time series using the offset and store the result into slave->data
//
// find_maxtm():
//   Given:
//      Same given as constructor
//   Required:
//      Find the maximum time and put the result into master->maxtm
//
///////////////////////////////////////////////////////////////////////////////////

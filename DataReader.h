#ifndef DATAREADER_H
#define DATAREADER_H

#define BLOCK_SLSZ 20              //!< Number of intermediary slice pointers

/** virtual class for threaded data reading

*/
template<class T>
class DataReader
{

  public:
    /**
     * \pre 
     *
     * Guarnteed valid values: master(all elements), maxmin(all
     * elements), and slave(all elements)
     *
     * \post
     *
     * From the given tm in slave->rdtm, find the data of the time
     * slice; give the pointer location to slave->data When the data
     * is found, set slave->v_bit to 1
    */
    virtual void reader()=0;
    /**
     * \pre
     *
     * Same given as constructor
     *
     * \post
     *
     * Set maxmin->read to 1 or 0 depending whether you need to find
     * something to read the data such as a byte offset: 1 for read
     * now, 0 for need to find something Find the max and min values
     * of each time slice and put the result to maxmin->lmax and
     * maxmin->lmin NOTE: maxmin->lmax, maxmin->lmin is type void.
     * You must type cast to proper type before assigning any values
     * If necessary, find the byte offset at the beginning of each
     * slice of time.  Set maxmin->del_sl_ptr to 1 or 0 depending on
     * whether byte offset was found: 1 for found, 0 for not found
     */
    virtual void local_maxmin()=0;
    /**
     * \pre
     *
     * Same given as constructor except that slave->unlock has the
     * offset value and slave->unlock has the slave->data has the
     * array location where the value is to be stored
     *
     * \post
     *
     * Find time series using the offset and store the result into
     * slave->data
     */
    virtual void tmsr()=0;
    /**
     * \pre
     * Same given as constructor
     * \post
     * Find the maximum time and put the result into master->maxtm
     */
    virtual void find_maxtm()=0;

    /**
     * \pre
     *
     * Guarnteed valid values: In master:ftype, fname, scanstr, slsz,
     * maxmin_ptr, this_ptr, and btw_size All other values are either
     * invalid or unkown.
     *
     * \post
     *
     * Initialize anything that you may need later such as opening
     * file (to be determined by the programmer)
     */
    DataReader():mthread(0),sthread(0),maxmin_ptr(0),data(0),in(NULL){}
    DataReader(Master<T>*m, Slave<T>*s, Maxmin<T>* mnmx):
              mthread(m),sthread(s),maxmin_ptr(mnmx),data(0),in(NULL){}

  protected:
    Master<T>*    mthread;
    Slave<T>*     sthread;
    Maxmin<T>*    maxmin_ptr;
    T*            data;
    gzFile        in;
};
#endif

//////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

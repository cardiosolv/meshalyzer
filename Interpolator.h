#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

/** Class to linearly interpolate data
 */
template <class T>
class Interpolator
{
  public:
    Interpolator():_n0(0),_n1(0),_c(0){}
    Interpolator(int a, int b, T r):_n0(a),_n1(b),_c(r){}
    void set( int a, int b, T r ){_n0=a; _n1=b; _c=r; }
    T    interpolate( T* d ){return (1.-_c)*d[_n0]+_c*d[_n1];}
  private:
    int    _n0;
    int    _n1;
    T _c;    //!< relative weight of _n0
};

#endif

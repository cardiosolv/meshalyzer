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
    T    interpolate( T* d, double *br ){
      double range = br[1]-br[0];
      if( fabs(d[_n0]-d[_n1])<0.7*range ) return (1.-_c)*d[_n0]+_c*d[_n1];
      T a, b;
      if( d[_n0]<br[0]+range/2 ) {
        a = d[_n0] + range;
        b = d[_n1];
      } else {
        a = d[_n0];
        b = d[_n1] + range;
      }
      double res = (1.-_c)*a+_c*b;
      if( res<br[0] ) res += range;
      if( res>br[1] ) res -= range;
      return res;
    }
  private:
    int    _n0;
    int    _n1;
    T _c;    //!< relative weight of _n0
};

#endif

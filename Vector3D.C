// Vector3D.C
// (c) Ingmar Bitter '96-'99
// $Id: Vector3D.C,v 1.2 2005/09/21 20:08:05 vigmond Exp $

#include "Vector3D.h"
#include "math.h"
#include <sstream>

template<>
void Vector3D<int>::Demo(int, char**) {
    V3d x(3.5,2.5,1.5); V3i k(x); double s = 2.5;
    cout << endl <<"Demo of class Vector3D<>";
    cout << endl <<"size : " << sizeof(V3i) << " Bytes";
    cout << endl <<"public member functions:";
    cout << endl <<"  V3d x(3.5,2.5,1.5); V3i k(x); == x=" << x <<", k=" << k;
    cout << endl <<"  k==x;\t== " << (k==x)<<"\t k!=x;\t== " << (k!=x);
    cout << endl <<"  k(1,2,3);\t== " << (k(1,2,3))<<"\t x=k;\t== " << (x=k);    
    cout << endl <<"  k==x;\t== " << (k==x)<<"\t k!=x;\t== " << (k!=x);
    cout << endl <<"  k.X();\t== " << (k.X()) <<"\t k.U();\t== " << (k.U()) <<"\t k.R();\t== " << (k.R()) <<"\t k[0];\t== " << (k[0]);
    cout << endl <<"  k.Y();\t== " << (k.Y()) <<"\t k.V();\t== " << (k.V()) <<"\t k.G();\t== " << (k.G()) <<"\t k[1];\t== " << (k[1]);
    cout << endl <<"  k.Z();\t== " << (k.Z()) <<"\t k.W();\t== " << (k.W()) <<"\t k.B();\t== " << (k.B()) <<"\t k[2];\t== " << (k[2]);
    cout << endl <<"  +k;\t== " << (+k) <<"\t k+x;\t== " << (k+x);
    cout << endl <<"  -k;\t== " << (-k) <<"\t k-x;\t== " << (k-x);
    cout << endl <<"  x+=k;\t== " << (x+=k);
    cout << endl <<"  x-=k;\t== " << (x-=k);
    cout << endl <<"  double s(2.5);\t== "<< s;
    cout << endl <<"  s*x;\t== " << (s*x) <<"\t x*s;\t== " << (x*s) <<"\t x*=s;\t== " << (x*=s);
    cout << endl <<"  x/s;\t== " << (x/s) <<"\t x/=s;\t== " << (x/=s);
    cout << endl <<"  x.DotProduct(x);\t== " << x.DotProduct(x);
    cout << endl <<"  x.Dot(x);\t\t== " << x.Dot(x);
    cout << endl <<"  x.CrossProduct(x);\t== " << x.CrossProduct(x);
    cout << endl <<"  x.Cross(x);\t\t== " << x.Cross(x);
  cout << endl <<"End of demo of class Vector3D\n\n";
} // Demo

// end of Vector3D.C

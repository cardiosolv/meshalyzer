// Matrix4x4.C
// (c) Ingmar Bitter '96-'99
// $Id: Matrix4x4.C,v 1.3 2005/09/21 20:08:05 vigmond Exp $

// Matrix stored Type matrix[colums][rows]
// second element is second entry of first row denoted by e[1][0]
// indices are viewed as x/y-coords of the positions in the matrix
// thus the first entry is colum number, second is row number

#include "Matrix4x4.h"

template<>
void Matrix4x4<int>::Demo(int, char **)
{
  Matrix4x4<double> a, b(1,0,0,0, 0,2,0,0, 0,0,3,0, 0,0,0,1);
	double s = 2.5; int i = 2;
	Vector3D<double> v(2.5,2.5,2.5);
	cout << endl <<"Demo of class Matrix4x4";
	cout << endl <<"size : " << sizeof(Matrix4x4<int>) << " Bytes";
	cout << endl <<"public member functions:";
	cout << endl <<"  Matrix4x4<int> a; == " << a;
	cout << endl <<"  Matrix4x4<int> b; == "<< b;
	cout << endl <<"  a==b;\t== " << (a==b) <<"  \ta!=b;\t== " << (a!=b);
	cout << endl <<"  a=b;\t== " << (a=b);
	cout << endl <<"  +a;\t== " << (+a) ; // <<"\t a+b;\t== " << (a+b);
	cout << endl <<"  -a;\t== " << (-a) ; // <<"\t a-b;\t== " << (a-b);
	cout << endl <<"  a+=b;\t== " << (a+=b);
	cout << endl <<"  a-=b;\t== " << (a-=b);
	cout << endl <<"  double s(2.5);\t== "<< s <<"  int i(2);";
  Matrix4x4<double> m(a);
	cout << endl <<"  Matrix4x4<double> m(a);";
	cout << endl <<"  m*s;\t== " << (m*s);
	cout << endl <<"  m*i;\t== " << (m*i);
	cout << endl <<"  m*=i;\t== "<< (m*=i);
	cout << endl <<"  m/s;\t== " << (m/s);
	cout << endl <<"  m/=i;\t== "<< (m/=i);
	cout << endl <<"  Vector3D<double> v(2.5,2.5,2.5);\t== "<< v ;
	Vector3D<double> vi(2,2,2);
	cout << endl <<"  Vect or3D<int> vi(2,2,2); == "<<vi;
	cout << endl <<"  m;\t== " << (m);
	cout << endl <<"  m*v;\t== " << (m*v);
	cout << endl <<"  m*vi;\t== " << (m*vi);
	cout << endl <<"  m.AddTranslateX(3);\t== " << (m.AddTranslateX(3));
	cout << endl <<"  m.AddTranslateY(4);\t== " << (m.AddTranslateY(4));
	cout << endl <<"  m.AddTranslateZ(5);\t== " << (m.AddTranslateZ(5));
	cout << endl <<"  m*v;\t== " << (m*v);	
	cout << endl <<"  m*b;\t== " << (m*b);
	cout << endl <<"  m*=m;\t== " << (m*=m);
	cout << endl <<"  m.Transpose();\t== " << m.Transpose();
	cout << endl <<"  m.Trans();    \t== " << m.Trans();
	cout << endl <<"  m.Determinant();\t== " << m.Determinant();
	cout << endl <<"  m.Det();        \t== " << m.Det();
	cout << endl <<"  m.Inverse();\t== " << m.Inverse();
	cout << endl <<"  m.Inv();    \t== " << m.Inv();
  Vector3D<double> u(-1,0,2);
  cout<<endl<<"  u="<<u<<"  m*u="<<(m*u);
  m.Translate(1,1,1);
  cout<<endl<<"  u="<<u<<"  m*u="<<(m*u);
  m.Scale(.5,.5,.5);
  cout<<endl<<"  u="<<u<<"  m*u="<<(m*u);
	cout << endl <<"End of demo of class Matrix4x4\n\n";
} // Demo

// end of Matrix4x4.C

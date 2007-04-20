// ModInt.C
// (c) Ingmar Bitter '96-'99 / Hanspeter Pfister '97
// $Id: ModInt.C,v 1.1 2004/02/06 21:50:57 vigmond Exp $

#include "ModInt.h"

void ModInt::Demo(int , char **) 
{
  ModInt k(3), l(5,3), m(l), n;

  cout << endl <<"Demo of class ModInt";
  cout << endl <<"size : " << sizeof(ModInt) << " Bytes";
  cout << endl <<"public members: - none";
  cout << endl <<"public member functions:";

  cout << endl <<"Constructors";

  cout << endl <<"  ModInt k(3), l(5,3), m(l), n = "<<k<<", "<<l<<", "<<m<<", "<<n;
  cout << endl << BoolStr[k==3 && l==2 && m==2 && n==0];

  cout << endl <<"  k(0,3), k(1,3), k(2,3), k(3,3), k(4,3) = "
       <<k(0,3)<<", "<<k(1,3)<<", "<<k(2,3)<<", "<<k(3,3)<<", "<<k(4,3);
	cout << endl << BoolStr[k(0,3)==0 && k(1,3)==1 && k(2,3)==2 && k(3,3)==0 && k(4,3)==1];

  cout << endl <<"  k(0,4), k(1,4), k(2,4), k(3,4), k(4,4) = "
       <<k(0,4)<<", "<<k(1,4)<<", "<<k(2,4)<<", "<<k(3,4)<<", "<<k(4,4);
	cout << endl << BoolStr[k(0,4)==0 && k(1,4)==1 && k(2,4)==2 && k(3,4)==3 && k(4,4)==0];

  cout << endl <<"  k(-1,3), k(-2,3), k(-3,3), k(-4,3) = "
       <<k(-1,3)<<", "<<k(-2,3)<<", "<<k(-3,3)<<", "<<k(-4,3);
	cout << endl << BoolStr[k(-1,3)==2 && k(-2,3)==1 && k(-3,3)==0 && k(-4,3)==2];

	cout << endl <<"  k.Abs() = "<< k.Abs();
	cout << endl << BoolStr[k.Abs()==2];

  cout << endl <<"Comparisons";

  cout << endl <<"  k==k = "<<(k==k)<<"\tk<=k = "<<(k<=k)<<"\tk<k = "<<(k<k) 
       <<"\tk==9 = "<<(k==9)<<"\t9.<=k = "<<(9.<=k)<<"\tk<n = "<<(k<n);
	cout << endl << BoolStr[(k==k)==1 && (k<=k)==1 && (k<k)==0 && (k==9)==0 
                                 && (9.<=k)==0 && (k<n)==0];

  cout << endl <<"  k!=k = "<<(k!=k)<<"\tk>=k = "<<(k>=k)<<"\tk>k = "<<(k>k)
       <<"\tk!=9 = "<<(k!=9)<<"\t9.>=k = "<<(9.>=k)<<"\tk>n = "<<(k>n);
	cout << endl << BoolStr[(k!=k)==0 && (k>=k)==1 && (k>k)==0 && (k!=9)==1 
                                 && (9.>=k)==1 && (k>n)==1];

  cout << endl <<"Unary Operators";

  cout << endl <<"  ++k  = "<<(++k);
	cout << endl << BoolStr[k==0];
  cout << endl <<"  k++  = "<<(k++);
	cout << endl << BoolStr[k==1];
  cout << endl <<"  --k  = "<<(--k);
	cout << endl << BoolStr[k==0];
  cout << endl <<"  k--  = "<<(k--);
	cout << endl << BoolStr[k==2];
  cout << endl <<"  k--  = "<<(k--);
	cout << endl << BoolStr[k==1];
  cout << endl <<"  k--  = "<<(k--);
	cout << endl << BoolStr[k==0];
  cout << endl <<"  k--  = "<<(k--);
	cout << endl << BoolStr[k==2];
  cout << endl <<"  k--  = "<<(k--);
	cout << endl << BoolStr[k==1];

  cout << endl <<"ModInt op ModInt";
  k(1,5);
  n(2,5);
  cout << endl <<"  k = " << k << "\tn = " << n;

  cout << endl <<"  k+=n  = "<<(k+=n);
	cout << endl << BoolStr[k==3];
  cout << endl <<"  k*=n  = "<<(k*=n);
	cout << endl << BoolStr[k==1];
  cout << endl <<"  k-=n  = "<<(k-=n);
	cout << endl << BoolStr[k==4];
  cout << endl <<"  k/=n  = "<<(k/=n);
	cout << endl << BoolStr[k==2];

  k(4,5);
  n(2,5);
  cout << endl <<"  k = " << k << "\tn = " << n;

  cout << endl <<"  k+n  = "<<(k+n) <<"\tk-n  = "<<(k-n);
  cout         <<"\tk*n  = "<<(k*n) <<"\tk/n  = "<<(k/n);
	cout << endl << BoolStr[(k+n)==1 && (k-n)==2 && (k*n)==3 && (k/n)==2]; 
 
  cout << endl <<"ModInt op double";
  k(4,5);
  double a(2.2);
  cout << endl <<"  k = " << k << "\ta = " << a;

	cout << endl <<"  k+=a = "<<(k+=a);
	cout << endl << BoolStr[k==1];
  cout << endl <<"  k-=a = "<<(k-=a);
	cout << endl << BoolStr[k==4];
	cout << endl <<"  k*=a = "<<(k*=a);
	cout << endl << BoolStr[k==3];
  cout << endl <<"  k/=a = "<<(k/=a);
	cout << endl << BoolStr[k==1];
    
	cout << endl <<"  k+a = "<<(k+a) << "\tk-a = "<<(k-a);
	cout         <<"\tk*a = "<<(k*a) <<"\tk/a = "<<(k/a);
	cout << endl << BoolStr[(k+a)==3 && (k-a)==4 && 
                                 (k*a)==2 && (k/a)==0]; 

  cout << endl <<"double op ModInt";
  k(4,5);
  cout << endl <<"  k = " << k << "\ta = " << a;

	cout << endl <<"  a+k = "<<(a+k) << "\ta-k = "<<(a-k);
	cout         <<"\ta*k = "<<(a*k) <<"\ta/k = "<<(a/k);
	cout << endl << BoolStr[(a+k)==(2.2+4) && (a-k)==(2.2-4) && 
                                 (a*k)==(2.2*4) && (a/k)==(2.2/4)]; 

	cout << endl <<"  a+=k = "<<(a+=k);
	cout << endl << BoolStr[a==6.2];
  cout << endl <<"  a-=k = "<<(a-=k);
	cout << endl << BoolStr[a==2.2];
	cout << endl <<"  a*=k = "<<(a*=k);
	cout << endl << BoolStr[a==8.8];
  cout << endl <<"  a/=k = "<<(a/=k);
	cout << endl << BoolStr[a==2.2];

	ModInt array[6];
  int i = 0;
	for (k(0); i<10; ++k, ++i)
    cout << endl << " array["<<k<<"] = "<<(array[k] = k);

  cout << endl <<"End of demo of class ModInt\n\n";
}	//	Demo


// end of ModInt.C


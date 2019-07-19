// Mouse.C
// (c) Ingmar Bitter '99
// $Id: Mouse.C,v 1.1 2004/02/06 21:50:57 vigmond Exp $

#include "Mouse.h"

void Mouse::Demo(int, char**) {
  Mouse mouse;
  cout << endl <<"Demo of class Mouse";
  cout << endl <<"size : " << sizeof(Mouse) << " Bytes";
  cout << endl <<"public member functions:";
  cout << endl << "  mouse " << mouse;
  mouse.SetWindowSize(100,100);
  cout << endl << "  mouse.SetWindowSize(100,100) " << mouse;
  //mouse.SetFrustumPosAndSize(-1,-1,2,2);
  //cout << endl << "  mouse.SetFrustumPosAndSize(-1,-1,2,2) " << mouse;
  mouse.UpdatePos(20,30);
  cout << endl << "  mouse.UpdatePos(20,30) " << mouse;
  mouse.UpdatePos(40,60);
  cout << endl << "  mouse.UpdatePos(40,60) " << mouse;
  cout << endl << "End of demo of class Mouse " << endl << endl;
} // Demo

// ostream & operator << (ostream & os, Mouse & mouse) { return mouse.Ostream(os); }

// end of Mouse.C

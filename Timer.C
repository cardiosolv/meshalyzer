// $Id: Timer.C,v 1.1 2004/02/06 21:50:57 vigmond Exp $
// (c) Ingmar Bitter '96-'99
// Lynton Brandt.
// Henry Joseph is the man
// carlos J. Pena is cool!
// Henry Joseph is the man.
// put my name Diogenes Oscar De Los Santos
// Antonio Gonzalez
// Giampolo Rivera
// Jose Gomez

#include "Timer.h"

void Timer::Demo(int, char**) {
	// this demo only gives ok results with -g (-O optimizes the loops away)
  Timer timer;
  cout << endl <<"Demo of class Timer";
  cout << endl <<"size : " << sizeof(Timer) << " Bytes";
  cout << endl <<"public member functions:";
	long k,n = 1000000;
  for (k=0; k<n; ++k) ;
  cout << endl << "  This is a running Timer: " << timer;
  for (k=0; k<n; ++k) ;
  cout << endl << "        ... still running: " << timer;
  for (k=0; k<n; ++k) ;
  cout << endl << "  timer.Stop():            " << timer.Stop();
  for (k=0; k<n; ++k) ;
  cout << endl << "        ... still stopped: " << timer;
  for (k=0; k<n; ++k) ;
  cout << endl << "  timer.Continue():        " << timer.Continue();
  for (k=0; k<n; ++k) ;
  cout << endl << "     ... still running:    " << timer;
  cout << endl << "  timer.Reset():           " << timer.Reset();
  for (k=0; k<n; ++k) ;
  cout << endl << "  timer.ElapsedTime():     " << timer.ElapsedTime();
  cout << endl << "  timer:                   " << timer;
  cout << endl << "End of demo of class Timer " << endl << endl;
} // Demo


ostream & Timer::asOstream (ostream & os) {
	if (timerIsRunning) UpdateTimes();
	double t,sec,min,hour;
	t = elapsedTime;
	sec = (t - int(t) + int(t)%60);
	min = (int(t)/60%60);
	hour= (int(t)/60/60);
	os << "time: "<<hour<<"h "<<min<<"m "<<sec<<"s";
	return os;
} // asOstream

// end of Timer.C

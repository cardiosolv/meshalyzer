// Timer.h
// (c) Ingmar Bitter '96-'99
// $Id: Timer.h,v 1.1 2004/02/06 21:50:57 vigmond Exp $

// Timer to track program running time (wall clock time)

#ifndef _Timer_h_	// prevent multiple includes
#define _Timer_h_

#include "Global.h"
#include <strstream.h>
#include <sys/time.h>

class Timer {
public:
  static void	Demo (int argc, char *argv[]);
	inline Timer ();
  inline Timer & Reset();
  inline Timer & Stop();
  inline double	 ElapsedTime();
  inline Timer & Continue();  
  friend inline ostream & operator << (ostream & os, Timer & timer);
  ostream & asOstream (ostream & os);
  inline const char * c_str();
protected:
  inline void	UpdateTimes();
  bool   timerIsRunning;
  double startTime,  elapsedTime;
  char   string[100];
}; // Timer

inline Timer::Timer () : timerIsRunning(true) {
	Reset(); 
} // constructor

inline Timer & Timer::Reset() {
	startTime = 0;
	UpdateTimes();
	startTime  = elapsedTime; 
	timerIsRunning = true;
	return *this; 
} // Reset

inline Timer & Timer::Stop() { 
	if (timerIsRunning) { 
		UpdateTimes(); 
		timerIsRunning = false;
	} 
	return *this; 
} // Stop

inline double	Timer::ElapsedTime() {
	if (timerIsRunning) 
		UpdateTimes(); 
	return elapsedTime; 
} // ElapsedTime

inline Timer & Timer::Continue() {
	if (!timerIsRunning) {
		double tmp  = elapsedTime;
		UpdateTimes();
		startTime += elapsedTime  - tmp;
		timerIsRunning = true; }
	return *this; 
} // Continue
  
inline ostream & operator << (ostream & os, Timer & timer) {
	return timer.asOstream(os);
} // operator <<

inline const char * Timer::c_str() {
	strstream stream;
	stream << *this;
	strcpy(string,stream.str());
	return string;
} // c_str

inline void	Timer::UpdateTimes() {
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	elapsedTime = double(tv.tv_sec) + 1e-6*double(tv.tv_usec) - startTime;
} // UpdateTimes

#endif	// _Timer_h_


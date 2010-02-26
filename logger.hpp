#pragma once

#include <string>
#include <fstream>

//#define LOGGING_ENABLED

#ifdef LOGGING_ENABLED

#define LOG1(msg)					\
({							\
  Logger & log = Logger::GetInstance();			\
  log << msg << std::endl;				\
  log.flush();						\
  log.bump();						\
})

#define LOG2(msg1, msg2)				\
({							\
  Logger & log = Logger::GetInstance();			\
  log << msg1 << msg2 << std::endl;			\
  log.flush();						\
  log.bump();						\
})

#define LOG_TIMER(msg)					\
({							\
  Logger & log = Logger::GetInstance();			\
  log << log.elapsed() << ": " << msg << std::endl;	\
  log.flush();						\
  log.reset();						\
})

#define LOG_TIMER_RESET				\
({						\
  Logger & log = Logger::GetInstance();		\
  log.reset();					\
})

#else

#define LOG1(msg) ;
#define LOG2(msg1, msg2) ;
#define LOG_TIMER(msg) ;
#define LOG_TIMER_RESET ;

#endif

class Logger : public ofstream
{
public:
  static Logger & GetInstance()
  {
    static Logger * logger = 0;

    if (!logger)
      logger = new Logger();
    
    return *logger;
  }

private:
  Logger()
    : ofstream("logger.txt")
  {
    reset();
  }

public:
  ~Logger()
  {
  }

  void reset()
  {
    timer = clock();
  }

  void bump()
  {
  }

  double elapsed()
  {
    double ret = (clock() - timer) / (double) CLOCKS_PER_SEC;
    reset();
    return ret;
  }

private:
  clock_t timer;

};

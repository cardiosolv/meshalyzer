#include <pthread.h>
#include <semaphore.h>
#include <zlib.h>
#include <string.h>

class ThreadBuffer {
	public:
		ThreadBuffer( int sz, int, sem_t* );
		void fill( gzFile gf, pthread_mutex_t* fmutex, int t ); 
		int retrieve(int, void *);
		int slice_no(){ return number; }
		~ThreadBuffer(){ delete[] buffer; pthread_mutex_destroy(&mutex); }
	private:
		pthread_mutex_t mutex;		// control buffer access
		sem_t*          sem;		// semaphore
		int   size;					// size of slice
		int   number;				// slice number stored
		char* buffer;				// store for uncompressed data
		int   offset;				// position of first datum in file stream
		bool  unread;				// true if data no read
};


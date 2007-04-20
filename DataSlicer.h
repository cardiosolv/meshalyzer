#include "ThreadBuffer.h"
#include "IGBheader.h"
const int NUM_BUF_SLICES=5;// number of read ahead buffers

// data structure for pthread call
typedef struct spfill {
	ThreadBuffer*    buf;		// buffer
	int              time;		// time to load into buffer
	gzFile           in;
	pthread_mutex_t* f_mutex;
	sem_t*			 sem;
} Spawn_fill_t;

class DataSlicer {
	public:
		DataSlicer( gzFile gf, const int of, const int ss, 
										const int tmax, const int i );
		~DataSlicer();
		int get( int t, void * );
		void increment( int i ){ inc = i; }
		int  increment(){ return inc; }
		void timeSeries( int, unsigned char *, int );
	private:
		void updatebuffer( int b, int t );
		void updatebuffers( int t );
		pthread_t thread[NUM_BUF_SLICES];	// spawn thread for new data
		ThreadBuffer* buffer[NUM_BUF_SLICES];// buffers for data
		pthread_mutex_t	f_mutex;			// control access to in
		sem_t			sem;				// semaphore for #fill buffers
		gzFile in;							// input file
		int offset;							// offset to data in file
		int slicesz;						// slice size
		int tmax;							// maximum time
		int inc;							// increment between slices
};
		
		
		

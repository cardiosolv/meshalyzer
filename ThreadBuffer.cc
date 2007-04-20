#include "ThreadBuffer.h"

ThreadBuffer ::	ThreadBuffer( int sz, int of, sem_t* sm ): size(sz), 
						offset(of), sem(sm), number(-1), unread(true) {
	pthread_mutex_init( &mutex, NULL );
	buffer = new char[size];
}

void 
ThreadBuffer :: fill( gzFile gf, pthread_mutex_t* fmutex, int t ) {
	
	int pos = offset + t*size;
	pthread_mutex_lock( fmutex );
	gzseek( gf, pos, SEEK_SET );
	pthread_mutex_lock( &mutex );
	gzread( gf, buffer, size );
	pthread_mutex_unlock( fmutex );
	number = t;
	unread = true;
	pthread_mutex_unlock( &mutex );
}

/*
 * get a time slice
 * only read in the slice if the time has changed since the last call
 *
 * -1 indicates the buffer did not contain data for the requested time
 */
int 
ThreadBuffer :: retrieve( int t, void *dest ) {

	int retval=0;
	pthread_mutex_lock( &mutex );
	if( number==t && unread ) { 
		memcpy( dest, buffer, size );
		unread == false;
	} else
		retval = -1;
	pthread_mutex_unlock( &mutex );
	sem_trywait( sem );
	return retval;
}


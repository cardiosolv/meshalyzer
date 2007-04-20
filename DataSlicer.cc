#include "DataSlicer.h"

// thread to read data into a buffer
void* spawn_fill( void *d ) {
	pthread_detach( pthread_self() );
	Spawn_fill_t* sf= (Spawn_fill_t*)d;
	sem_post( sf->sem );
	sf->buf->fill( sf->in, sf->f_mutex, sf->time );
	delete sf;
	return NULL;
}

DataSlicer :: DataSlicer( gzFile gf, const int of, const int ss, 
								const int t, const int ic):offset(of),
										in(gf),slicesz(ss),tmax(t),inc(ic){

	pthread_mutex_init( &f_mutex, NULL );
	sem_init( &sem, 0, 0 );

	for( int i=0; i<NUM_BUF_SLICES; i++ )
		buffer[i] = new ThreadBuffer( slicesz, offset, &sem );
}

DataSlicer :: ~DataSlicer() {

	pthread_mutex_destroy( &f_mutex );

	for( int i=0; i<NUM_BUF_SLICES; i++ ){
		delete buffer[i];
		buffer[i] = NULL;
	}
}
											
/*
 * get a slice of data and put it in dest
 *
 * use the semaphore to make sure that there are no pending updates
 * 
 * return -1 if an error
 */
int 
DataSlicer::get( int t, void *dest )
{
	int b;

	if( t>tmax || t<0 )
		return -1;
	
	int semval;
	bool retrieved = false;
	
	do {
		do {
			for( b=0; b<NUM_BUF_SLICES; b++ )
				if( buffer[b]->retrieve( t, dest ) != -1 ) {
					retrieved=true;
					break;
				}
			sem_getvalue( &sem, &semval );
		}while( semval && b==NUM_BUF_SLICES );
		if( retrieved==false ){
			updatebuffers( t );
		}
	}while( retrieved==false );

	updatebuffers( t );
}

/*
 * spawn a thread to fill one buffer
 */
void DataSlicer :: updatebuffer( int b, int t ) {
	if(  t>=0 && t<= tmax ) {
		Spawn_fill_t* sf = new Spawn_fill_t;
		sf->buf  = buffer[b];
		sf->time = t;
		sf->in = in;
		sf->f_mutex = &f_mutex;
		sf->sem = &sem;
		if( pthread_create( &(thread[b]), NULL, spawn_fill, (void *)sf ) )
			fprintf( stderr, "Error: cannot spawn new thread\n");
	}
}

/*
 * fill all buffers
 *
 * centre around the time t
 */
void DataSlicer :: updatebuffers( int t ) {

	// determine first and last instances 
	int lower = t-inc*(NUM_BUF_SLICES/2-!(NUM_BUF_SLICES%2));
	int upper = lower + (NUM_BUF_SLICES-1)*inc;
	if( lower>upper ) {
		int tmp = lower;
		lower = upper;
		upper = tmp;
	}
	while( lower<0 )
		lower += abs(inc);
	while( upper>tmax )
		upper -= abs(inc);

	int j=0;
	for( int s=lower; s<=upper; s+=abs(inc) ){
		int i;
		for( i=0; i<NUM_BUF_SLICES; i++ )	//look for slice in buffer
			if( buffer[i]->slice_no() == s )
				break;
		if( i==NUM_BUF_SLICES ) {			// if not there, add it
			for( ; j<NUM_BUF_SLICES; j++ ) {//find out-of-range slice
				int sn = buffer[j]->slice_no();
				if( sn<lower ||  sn>upper || ((sn-lower)%inc) ) {
					updatebuffer( j, s );
					j++;
					break;
				}
			}
		}
	}
}


/*
 * return the time series for a given point
 *
 * point - offset into slice for the point
 * buf   - buffer in which to place data
 * size  - size of datum
 */
void
DataSlicer :: timeSeries(int point, unsigned char* buf, int size )
{
	int pos=offset+point;
	pthread_mutex_lock( &f_mutex );
	for( int i=0; i<=tmax; i++ ) {
		gzseek( in, pos, SEEK_SET );
		gzread( in, buf, size );
		pos += slicesz;
		buf += size;
	}
	pthread_mutex_unlock( &f_mutex );
}	


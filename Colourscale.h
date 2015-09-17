/*
 * Class for a colour map which returns a vector suitable for a glColor()
 * call
 */

#ifndef COLOURSCALE_INC
#define COLOURSCALE_INC
#include <FL/gl.h>
#include <math.h>
#include "PNGwrite.h"

typedef enum {
  CS_HOT, CS_GREY, CS_RGREY, CS_GGREY, CS_BGREY, CS_IGREY, CS_RAINBOW, CS_BL_RAINBOW, 
  CS_COLD_HOT, CS_CG, CS_MATLAB_REV, CS_MATLAB, CS_ACID, CS_P2G, CS_VIRIDIS
} CScale_t;

class Colourscale
{
  public:
    Colourscale(int ts=64, CScale_t tcs=CS_BL_RAINBOW );
    void   scale( CScale_t );				// set scale
    inline CScale_t scale(void){ return scaletype; }
    void   calibrate( double, double ); 	// set range
    inline float min(){return mindat;}		// return minimum data value
    inline float max(){return maxdat;}		// return maximum data value
    void colourize( float );				// colour vector for data value
    void colourize( float, float );			// same as above, specify alpha
    inline int size(){ return n; }			// get the size
    void   size( int );	    				// set the size
    inline GLfloat* entry(int a){return cmap[a];}	// return an entry
    GLfloat *colorvec( double );
    void   output_png( const char * );            // output the map
  private:
    GLfloat** cmap;							// the map
    int       n;							// size of map
    double    a, b;							// map data to colour map
    CScale_t  scaletype;
    double    mindat, maxdat;
};

#endif


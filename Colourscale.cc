#include "Colourscale.h"

Colourscale :: Colourscale( int ts, CScale_t tcs )
{
  n = 0;
  mindat = 0;
  maxdat = 1;
  scaletype = tcs;
  size( ts );
}


void Colourscale :: calibrate( double tmin, double tmax )
{
  mindat = tmin;
  maxdat = tmax;

  if ( maxdat != mindat ) {
    a = static_cast<float>(n)/(maxdat-mindat);
    b = static_cast<float>(n) - a*maxdat;
  } else {
    a = 0.;
    b = 1.;
  }
}

void Colourscale :: scale( CScale_t cs )
{
  int i;
  float intrvl;

  float ispan = static_cast<float>(n);
  scaletype = cs;
  switch ( cs ) {
    case CS_GREY: 							/* bw */
      for ( i=0; i<n; i++ )
        cmap[i][0] = cmap[i][1] = cmap[i][2] = ((float)i)/ispan;
      break;
    case CS_HOT:
      intrvl = roundf(ispan/3.);
      for ( i=0; i<intrvl; i++ ) {
        cmap[i][0] = (float)(i)/(intrvl-1);
        cmap[i][1] = cmap[i][2] = 0;
      }
      for ( ; i<2*intrvl; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = (float)(i-intrvl)/(intrvl-1);
        cmap[i][2] = 0;
      }
      for ( ; i<n; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = 1;
        cmap[i][2] = (float)(i-2*intrvl)/(n-2*intrvl-1);
      }
      break;
    case CS_RAINBOW:
      intrvl = ispan/6.;
      for ( i=0; i<intrvl; i++ ) {
        cmap[i][0] =  1-(float)(i)/intrvl;
        cmap[i][1] = 0;
        cmap[i][2] = 1l;
      }
      for ( ; i<2*intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = (float)(i-intrvl)/intrvl;
        cmap[i][2] = 1;
      }
      for ( ; i<3*intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = 1;
        cmap[i][2] = 1-(float)(i-2*intrvl)/intrvl;
      }
      for ( ; i<4*intrvl; i++ ) {
        cmap[i][0] = (float)(i-3*intrvl)/intrvl;
        cmap[i][1] = 1;
        cmap[i][2] = 0;
      }
      for ( ; i<5*intrvl; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = 1-(float)(i-4*intrvl)/intrvl;
        cmap[i][2] = 0;
      }
      for ( ; i<n; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = 0;
        cmap[i][2] =  (float)(i-5*intrvl)/intrvl;
      }
      break;
    case CS_COLD_HOT:
      intrvl = ispan/2.;
      for ( i=0; i<0+intrvl; i++ ) {
        cmap[i][2] =  1;
        cmap[i][0] =  cmap[i][1] = (float)(i)/intrvl ;
      }
      for ( ; i<n; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] =  cmap[i][2] = (float)(n-1-i)/intrvl ;
      }
      break;
    case CS_CG:
      intrvl = ispan/4.;
      for ( i=0; i<intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = (float)(i)/intrvl;
        cmap[i][2] = 1;
      }
      for ( ; i<2*intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = 1;
        cmap[i][2] = (float)(2*intrvl-i)/intrvl;
      }
      for ( ; i<3*intrvl; i++ ) {
        cmap[i][0] = (float)(i-2*intrvl)/intrvl;
        cmap[i][1] = 1;
        cmap[i][2] = 0;
      }
      for ( ; i<n; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = (float)(4*intrvl-i)/intrvl;
        cmap[i][2] = 0;
      }
      break;
    case CS_MATLAB: // should be CS_MATLAB_REV according controls window
      intrvl = ispan/8.;
      for ( i=0; i<intrvl; i++ ) {
        cmap[i][0] = 0.5 + (float)(i)/(2*intrvl);
        cmap[i][1] = 0;
        cmap[i][2] = 0;
      }
      for ( ; i<3*intrvl; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = (float)(i-intrvl)/(2*intrvl);
        cmap[i][2] = 0;
      }
      for ( ; i<5*intrvl; i++ ) {
        cmap[i][0] = (float)(5*intrvl-i)/(2*intrvl);
        cmap[i][1] = 1;
        cmap[i][2] = (float)(i-3*intrvl)/(2*intrvl);
      }
      for ( ; i<7*intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = (float)(7*intrvl-i)/(2*intrvl);
        cmap[i][2] = 1;
      }
      for ( ; i<n; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = 0;
        cmap[i][2] = (float)(9*intrvl-i)/(2*intrvl);;
      }
      break;
    case CS_MATLAB_REV: // should be CS_MATLAB
      intrvl = ispan/8.;
      for ( i=0; i<intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = 0;
        cmap[i][2] = 0.5 + (float)(i+1)/(2*intrvl);
      }
      for ( ; i<3*intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = (float)(i-intrvl+1)/(2*intrvl);
        cmap[i][2] = 1;
      }
      for ( ; i<5*intrvl; i++ ) {
        cmap[i][0] = (float)(i-3*intrvl+1)/(2*intrvl);
        cmap[i][1] = 1;
        cmap[i][2] = (float)(5*intrvl-i-1)/(2*intrvl);
      }
      for ( ; i<7*intrvl; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = (float)(7*intrvl-i-1)/(2*intrvl);
        cmap[i][2] = 0;
      }
      for ( ; i<n; i++ ) {
        cmap[i][0] = 0.5 + (float)(8*intrvl-i-1)/(2*intrvl);
        cmap[i][1] = 0;
        cmap[i][2] = 0;
      }
      break;
    case CS_BL_RAINBOW:
    default:
      intrvl = ispan/6.;
      for ( i=0; i<intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = 0;
        cmap[i][2] = (float)(i)/intrvl ;
      }
      for ( ; i<2*intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = (float)(i-intrvl)/intrvl;
        cmap[i][2] = 1;
      }
      for ( ; i<3*intrvl; i++ ) {
        cmap[i][0] = 0;
        cmap[i][1] = 1;
        cmap[i][2] = (float)(3*intrvl-i)/intrvl;
      }
      for ( ; i<4*intrvl; i++ ) {
        cmap[i][0] = (float)(i-intrvl*3)/intrvl;
        cmap[i][1] = 1;
        cmap[i][2] = 0;
      }
      for ( ; i<5*intrvl; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = (float)(5*intrvl-i)/intrvl;
        cmap[i][2] = 0;
      }
      for ( ; i<n; i++ ) {
        cmap[i][0] = 1;
        cmap[i][1] = float(i-5*intrvl)/intrvl;
        cmap[i][2] = float(i-5*intrvl)/intrvl;
      }
      break;
  }
}


void Colourscale :: colourize( float val )
{
  int indx=int(rint(a*val+b));
  if ( indx<0 ) indx = 0;
  else if ( indx>=n ) indx = n-1;
  glColor3fv( cmap[indx] );
}


void Colourscale :: colourize( float val, float alpha )
{
  int indx=int(rint(a*val+b));
  if ( indx<0 ) indx = 0;
  else if ( indx>=n ) indx = n-1;
  cmap[indx][3] = alpha;
  glColor4fv( cmap[indx] );
}

void Colourscale :: size( int s )
{
  if ( s<3 ) return;

  if ( n ) {
    for ( int i=0; i<n; i++ ) delete[] cmap[i];
    delete[] cmap;
  }

  n = s;
  cmap = new GLfloat* [n];
  for ( int i=0; i<n; i++ ) cmap[i] = new GLfloat[4];

  calibrate( mindat, maxdat );
  scale( scaletype );
}

GLfloat* Colourscale :: colorvec( double val )
{
  int indx=int(rint(a*val+b));
  if ( indx<0 ) indx = 0;
  else if ( indx>=n ) indx = n-1;
  return cmap[indx];
}


void
Colourscale :: output_png( const char *filename )
{
  PNGwrite cbar( fopen(filename,"w") );
  int y        = 128;
  int colwidth = 1024/n;
  int x        = colwidth*n;
  cbar.size( x, y );
  unsigned char buffer[x*y*3];
  for( int line = 0; line<y; line++ ){
    for( int i=0; i<n; i++ ) {
      unsigned char r=lroundf(cmap[i][0]*255);
      unsigned char g=lroundf(cmap[i][1]*255);
      unsigned char b=lroundf(cmap[i][2]*255);
      for( int j=0; j<colwidth; j++ ) {
        buffer[(i*colwidth+j+line*x)*3]   = r;
        buffer[(i*colwidth+j+line*x)*3+1] = g;
        buffer[(i*colwidth+j+line*x)*3+2] = b;
      }
    }
  }
  cbar.write( buffer, 1 );
}

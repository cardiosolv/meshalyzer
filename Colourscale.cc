#include "Colourscale.h"

Colourscale :: Colourscale( int ts, CScale_t tcs ) : n(0), mindat(0.),maxdat(1.)
{
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

int viridis[][3] = {
  {  68,   1,  84 },
  {  68,   2,  85 },
  {  68,   3,  87 },
  {  69,   5,  88 },
  {  69,   6,  90 },
  {  69,   8,  91 },
  {  70,   9,  92 },
  {  70,  11,  94 },
  {  70,  12,  95 },
  {  70,  14,  97 },
  {  71,  15,  98 },
  {  71,  17,  99 },
  {  71,  18, 101 },
  {  71,  20, 102 },
  {  71,  21, 103 },
  {  71,  22, 105 },
  {  71,  24, 106 },
  {  72,  25, 107 },
  {  72,  26, 108 },
  {  72,  28, 110 },
  {  72,  29, 111 },
  {  72,  30, 112 },
  {  72,  32, 113 },
  {  72,  33, 114 },
  {  72,  34, 115 },
  {  72,  35, 116 },
  {  71,  37, 117 },
  {  71,  38, 118 },
  {  71,  39, 119 },
  {  71,  40, 120 },
  {  71,  42, 121 },
  {  71,  43, 122 },
  {  71,  44, 123 },
  {  70,  45, 124 },
  {  70,  47, 124 },
  {  70,  48, 125 },
  {  70,  49, 126 },
  {  69,  50, 127 },
  {  69,  52, 127 },
  {  69,  53, 128 },
  {  69,  54, 129 },
  {  68,  55, 129 },
  {  68,  57, 130 },
  {  67,  58, 131 },
  {  67,  59, 131 },
  {  67,  60, 132 },
  {  66,  61, 132 },
  {  66,  62, 133 },
  {  66,  64, 133 },
  {  65,  65, 134 },
  {  65,  66, 134 },
  {  64,  67, 135 },
  {  64,  68, 135 },
  {  63,  69, 135 },
  {  63,  71, 136 },
  {  62,  72, 136 },
  {  62,  73, 137 },
  {  61,  74, 137 },
  {  61,  75, 137 },
  {  61,  76, 137 },
  {  60,  77, 138 },
  {  60,  78, 138 },
  {  59,  80, 138 },
  {  59,  81, 138 },
  {  58,  82, 139 },
  {  58,  83, 139 },
  {  57,  84, 139 },
  {  57,  85, 139 },
  {  56,  86, 139 },
  {  56,  87, 140 },
  {  55,  88, 140 },
  {  55,  89, 140 },
  {  54,  90, 140 },
  {  54,  91, 140 },
  {  53,  92, 140 },
  {  53,  93, 140 },
  {  52,  94, 141 },
  {  52,  95, 141 },
  {  51,  96, 141 },
  {  51,  97, 141 },
  {  50,  98, 141 },
  {  50,  99, 141 },
  {  49, 100, 141 },
  {  49, 101, 141 },
  {  49, 102, 141 },
  {  48, 103, 141 },
  {  48, 104, 141 },
  {  47, 105, 141 },
  {  47, 106, 141 },
  {  46, 107, 142 },
  {  46, 108, 142 },
  {  46, 109, 142 },
  {  45, 110, 142 },
  {  45, 111, 142 },
  {  44, 112, 142 },
  {  44, 113, 142 },
  {  44, 114, 142 },
  {  43, 115, 142 },
  {  43, 116, 142 },
  {  42, 117, 142 },
  {  42, 118, 142 },
  {  42, 119, 142 },
  {  41, 120, 142 },
  {  41, 121, 142 },
  {  40, 122, 142 },
  {  40, 122, 142 },
  {  40, 123, 142 },
  {  39, 124, 142 },
  {  39, 125, 142 },
  {  39, 126, 142 },
  {  38, 127, 142 },
  {  38, 128, 142 },
  {  38, 129, 142 },
  {  37, 130, 142 },
  {  37, 131, 141 },
  {  36, 132, 141 },
  {  36, 133, 141 },
  {  36, 134, 141 },
  {  35, 135, 141 },
  {  35, 136, 141 },
  {  35, 137, 141 },
  {  34, 137, 141 },
  {  34, 138, 141 },
  {  34, 139, 141 },
  {  33, 140, 141 },
  {  33, 141, 140 },
  {  33, 142, 140 },
  {  32, 143, 140 },
  {  32, 144, 140 },
  {  32, 145, 140 },
  {  31, 146, 140 },
  {  31, 147, 139 },
  {  31, 148, 139 },
  {  31, 149, 139 },
  {  31, 150, 139 },
  {  30, 151, 138 },
  {  30, 152, 138 },
  {  30, 153, 138 },
  {  30, 153, 138 },
  {  30, 154, 137 },
  {  30, 155, 137 },
  {  30, 156, 137 },
  {  30, 157, 136 },
  {  30, 158, 136 },
  {  30, 159, 136 },
  {  30, 160, 135 },
  {  31, 161, 135 },
  {  31, 162, 134 },
  {  31, 163, 134 },
  {  32, 164, 133 },
  {  32, 165, 133 },
  {  33, 166, 133 },
  {  33, 167, 132 },
  {  34, 167, 132 },
  {  35, 168, 131 },
  {  35, 169, 130 },
  {  36, 170, 130 },
  {  37, 171, 129 },
  {  38, 172, 129 },
  {  39, 173, 128 },
  {  40, 174, 127 },
  {  41, 175, 127 },
  {  42, 176, 126 },
  {  43, 177, 125 },
  {  44, 177, 125 },
  {  46, 178, 124 },
  {  47, 179, 123 },
  {  48, 180, 122 },
  {  50, 181, 122 },
  {  51, 182, 121 },
  {  53, 183, 120 },
  {  54, 184, 119 },
  {  56, 185, 118 },
  {  57, 185, 118 },
  {  59, 186, 117 },
  {  61, 187, 116 },
  {  62, 188, 115 },
  {  64, 189, 114 },
  {  66, 190, 113 },
  {  68, 190, 112 },
  {  69, 191, 111 },
  {  71, 192, 110 },
  {  73, 193, 109 },
  {  75, 194, 108 },
  {  77, 194, 107 },
  {  79, 195, 105 },
  {  81, 196, 104 },
  {  83, 197, 103 },
  {  85, 198, 102 },
  {  87, 198, 101 },
  {  89, 199, 100 },
  {  91, 200,  98 },
  {  94, 201,  97 },
  {  96, 201,  96 },
  {  98, 202,  95 },
  { 100, 203,  93 },
  { 103, 204,  92 },
  { 105, 204,  91 },
  { 107, 205,  89 },
  { 109, 206,  88 },
  { 112, 206,  86 },
  { 114, 207,  85 },
  { 116, 208,  84 },
  { 119, 208,  82 },
  { 121, 209,  81 },
  { 124, 210,  79 },
  { 126, 210,  78 },
  { 129, 211,  76 },
  { 131, 211,  75 },
  { 134, 212,  73 },
  { 136, 213,  71 },
  { 139, 213,  70 },
  { 141, 214,  68 },
  { 144, 214,  67 },
  { 146, 215,  65 },
  { 149, 215,  63 },
  { 151, 216,  62 },
  { 154, 216,  60 },
  { 157, 217,  58 },
  { 159, 217,  56 },
  { 162, 218,  55 },
  { 165, 218,  53 },
  { 167, 219,  51 },
  { 170, 219,  50 },
  { 173, 220,  48 },
  { 175, 220,  46 },
  { 178, 221,  44 },
  { 181, 221,  43 },
  { 183, 221,  41 },
  { 186, 222,  39 },
  { 189, 222,  38 },
  { 191, 223,  36 },
  { 194, 223,  34 },
  { 197, 223,  33 },
  { 199, 224,  31 },
  { 202, 224,  30 },
  { 205, 224,  29 },
  { 207, 225,  28 },
  { 210, 225,  27 },
  { 212, 225,  26 },
  { 215, 226,  25 },
  { 218, 226,  24 },
  { 220, 226,  24 },
  { 223, 227,  24 },
  { 225, 227,  24 },
  { 228, 227,  24 },
  { 231, 228,  25 },
  { 233, 228,  25 },
  { 236, 228,  26 },
  { 238, 229,  27 },
  { 241, 229,  28 },
  { 243, 229,  30 },
  { 246, 230,  31 },
  { 248, 230,  33 },
  { 250, 230,  34 },
  { 253, 231,  36 }
};

#define INTERP(a,b,c) ( (int) (((float) (a) / (float) (b)) * (c)) )
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
    case CS_RGREY: 							/* bw */
      for ( i=0; i<n; i++ ) {
        cmap[i][0] = ((float)i)/ispan;
        cmap[i][1] = cmap[i][2] = 0;
      }
      break;
    case CS_GGREY: 							/* bw */
      for ( i=0; i<n; i++ ) {
        cmap[i][1] = ((float)i)/ispan;
        cmap[i][0] = cmap[i][2] = 0;
      }
      break;
    case CS_BGREY: 							/* bw */
      for ( i=0; i<n; i++ ) {
        cmap[i][2] = ((float)i)/ispan;
        cmap[i][0] = cmap[i][1] = 0;
      }
      break;
    case CS_IGREY: 							/* bw */
      for ( i=0; i<n; i++ ) {
        cmap[i][0] = cmap[i][2] = 0.0;
        cmap[i][1] = ((float)i)/ispan;
      }
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
    case CS_ACID:
      intrvl = ispan/2.;
      for ( i=0; i<0+intrvl; i++ ) {
        cmap[i][0] = cmap[i][2] = 1.0;
        cmap[i][1] = (float)(i)/intrvl ;
  }
      for ( ; i<n; i++ ) {
        cmap[i][0] = cmap[i][1] = 1.0;
        cmap[i][2] = (float)(n-1-i)/intrvl ;
      }
      break;
    case CS_P2G:
      for ( i = 0; i < n; i++ ) {
        cmap[i][0] = cmap[i][2] = (1.0 - (((float)i)/ispan));
        cmap[i][1] = ((float)i)/ispan;
      }
      break;
    case CS_VIRIDIS:
      for ( i = 0; i < n; i++ ) {
        cmap[i][0] = ((float) viridis[INTERP(i,n,255)][0] / 255.0);
        cmap[i][1] = ((float) viridis[INTERP(i,n,255)][1] / 255.0);
        cmap[i][2] = ((float) viridis[INTERP(i,n,255)][2] / 255.0);
      }
      break;
  }
}
#undef INTERP

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

/* src/IGBheader.h.  Generated automatically by configure.  */
#ifndef IGBheader_h
#define IGBheader_h

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <zlib.h>
#include <cstring>
#include <cctype>
#include <assert.h>
#include <float.h>
#include <limits.h>
#include "short_float.h"

#define NALLOC 100

#define	    NUL		0
#define	    INCONNU	0

#define     IGB_BIG_ENDIAN		666666666
#define     IGB_LITTLE_ENDIAN	777777777
#define	    N_SYSTEMES	2


/* ------------------------ TYPES definition ------------------------------ */
#define	    IGB_BYTE      1  /* -- byte ----------------------------------- */
#define	    IGB_CHAR	  2  /* -- Char ----------------------------------- */
#define	    IGB_SHORT	  3  /* -- short ---------------------------------- */
#define	    IGB_LONG	  4  /* -- long ----------------------------------- */
#define	    IGB_FLOAT	  5  /* -- float ---------------------------------- */
#define	    IGB_DOUBLE	  6  /* -- Double --------------------------------- */
#define	    IGB_COMPLEX	  7  /* -- 2 x float (real part, imaginary part) -- */
#define	    IGB_D_COMPLEX 8  /* -- 2 x Double (real part, imaginary part) - */
#define	    IGB_RGBA	  9  /* -- 4 x byte (red, green, blue, alpha) ----- */
#define	    IGB_STRUCTURE 10 /* -- Structure ------------------------------ */
#define	    IGB_POINTER   11 /* -- void * --------------------------------- */
#define	    IGB_LIST	  12 /* -- List   --------------------------------- */
#define	    IGB_INT	      13 /* -- integer -------------------------------- */
#define	    IGB_UINT	  14 /* -- unsigned integer------------------------ */
#define	    IGB_USHORT	  15 /* -- unsigned short integer------------------ */
#define     IGB_VEC3_f    16 /* -- 3 X float ------------------------------ */
#define     IGB_VEC3_d    17 /* -- 3 X double ----------------------------- */
#define     IGB_VEC4_f    18 /* -- 4 X float ------------------------------ */
#define     IGB_VEC4_d    19 /* -- 4 X double ----------------------------- */
#define     IGB_HFLOAT    20 /* -- half float ----------------------------- */
#define	    IGB_MIN_TYPE  1
#define	    IGB_MAX_TYPE  20

#define Byte hByte

/* Types de trames */
#define MIN_TRAME	0
#define	C8		0
#define	C4		1
#define	HEX		2
#define HEXEDGES	3
#define HEXBRIDGES	4
#define HEXLINES	5
#define HEX2		6
#define MAX_TRAME	6
#define NTRAMES		7


/* define for endedness */
#define IGB_ENDIAN_VAL -1.24e5
#define IGB_LITTLE_END_REP 0,48,242,199

// error codes
#define ERR_EOF_IN_HEADER      1
#define ERR_LINE_TOO_LONG      2
#define ERR_UNPRINTABLE_CHAR   3
#define ERR_IGB_SYNTAX         4
#define ERR_UNDEFINED_X_Y_TYPE 5
#define ERR_SIZE_REDEFINED     6 
#define ERR_SIZE_NOT_DEFINED   7
#define WARN_DIM_INCONSISTENT  256

/*
    Definition des types List, bytes, Char, Double, complex d_complex
    et rgba
*/
typedef	    struct List
{
  long    nitems;
  char    *items;
}
List;
typedef	    unsigned char	byte;
#ifndef __GL_GL_H__
typedef     unsigned char	Byte;
typedef     char		*String;
#endif
typedef	    signed char	    	Char;
typedef	    struct S_Complex	S_Complex;
typedef	    double	    	Double;
typedef     struct D_Complex	D_Complex;
typedef     float		Float;
typedef     int			Int;
typedef     long		Long;
typedef     int			Int;
typedef     unsigned int        UInt;
typedef     short		Short;
typedef     int			BooleaN;
typedef     int			Flag;
typedef     char		*RDir;
typedef     char		*RFile;
typedef     char		*RWDir;
typedef     char		*RWFile;
typedef     char		*WDir;
typedef     char		*WFile;
typedef     char		**Text;
typedef     void		Any;
struct S_Complex
{
  Float	real, imag;
};
struct D_Complex	
{
  Double	real, imag;
};
#ifndef _COMPLEX_DEFINED
typedef struct complex
{
  float   reel ;
  float   imag ;
}
complex ;
typedef struct d_complex
{
  Double  reel ;
  Double  imag ;
}
d_complex ;
#define _COMPLEX_DEFINED
#endif
typedef union rgba {
  unsigned	long    l;
  byte		b[4];
} rgba ;

/* Indice de chaque composante dans le vecteur b[] de l'union rgba */
#define RGBA_ROUGE 3
#define RGBA_VERT  2
#define RGBA_BLEU  1
#define RGBA_ALPHA 0

#define HFLT_MIN -65504
#define HFLT_MAX  65504

/* -------------- Definition du type des variables globales de header.c - */

#ifndef	HEADER_GLOBALS
extern	bool     	   Header_Quiet;
extern	const char    *Header_Type[IGB_MAX_TYPE+1];
extern	unsigned short Data_Size[IGB_MAX_TYPE+1];
extern	int            Num_Components[IGB_MAX_TYPE+1];
extern	const char	  *Header_Systeme[N_SYSTEMES];
extern  unsigned long  Header_Systeme_No[N_SYSTEMES];
extern	char		   Header_Message[256];
#endif

class IGBheader;

template<class T>
int read_IGB_data( T* dp, int numt, IGBheader* h, char *buf=NULL );

template<class T>
T IGB_convert_buffer_datum( IGBheader*, void *buf, int a );

class IGBheader
{
  private:
    void* file;				   	  //!< file ptr to read/write header
    bool gzipping;		  	 	  // are we gzipping data?
    int   v_x=0, v_y=0, v_z=1, v_t=1;  //!< dimensions --------------------
    int   v_type=0;               //!< type arithmetique -------------
    int   v_taille ;              //!< taille des pixels (type STRUCTURE)
    unsigned long v_systeme=endian() ;     //!< systeme informatique ----------
    int    v_num=0;               //!< numero de la tranche ----------
    int    v_bin=0;               //!< nombre de couleurs ------------
    int    v_trame=C8;            //!< trame(connectivite) -----------
    unsigned int v_lut=0 ;        //!< nombre de bytes table couleurs
    unsigned int v_comp=0 ;       //!< nombre de bytes table compres.
    float  v_epais=0;             //!< epaiseur d'une tranche --------
    float  v_org_x=1., v_org_y=1., v_org_z=1., v_org_t=0.; //!< coin sup gauche --------
    float  v_inc_x=1., v_inc_y=1., v_inc_z=1., v_inc_t=1.; //!< distance entre pixels -
    float  v_dim_x, v_dim_y, v_dim_z, v_dim_t ; //!< dimension totale -------
    float *v_vect_z=NULL;         //!< coord z de chaque tranche -----
    char   v_unites_x[41]={}, v_unites_y[41]={}, v_unites_z[41]={}, v_unites_t[41]={};
    //!< unites de mesure --------------
    char   v_unites[41]={};       //!< unites de mesure pour les valeurs des pixels -----------
    float  v_facteur=1., v_zero=0.;  //!< facteur d'echelle et valeur du zero -
    char   v_struct_desc[41]={};  //!< description de la structure ---
    char   v_aut_name[41]={};     //!< nom de l'auteur ---------------
    char** v_comment=(char**)calloc(NALLOC,sizeof(char*));    //!< commentaires ------------------
    void*  v_transparent=NULL;	  //!< transparent value for data
    int    puts_fcn(void *, char *);

    // boolean flags to indicate if a default value has been overridden
    bool bool_x=false, bool_y=false, bool_z=false, bool_t=false;
    bool bool_type=false;
    bool bool_taille=false;
    bool bool_num=false;
    bool bool_bin=false;
    bool bool_trame=false;
    bool bool_lut=false;
    bool bool_comp=false;
    bool bool_epais=false;
    bool bool_org_x=false, bool_org_y=false, bool_org_z=false, bool_org_t=false;
    bool bool_inc_x=false, bool_inc_y=false, bool_inc_z=false, bool_inc_t=false;
    bool bool_dim_x, bool_dim_y=false, bool_dim_z=false, bool_dim_t=false;
    bool bool_vect_z=false;
    bool bool_unites_x=false, bool_unites_y=false, bool_unites_z=false, bool_unites_t=false;
    bool bool_unites=false;
    bool bool_facteur=false, bool_zero=false;
    bool bool_struct_desc=false;
    bool bool_aut_name=false;
    bool bool_comment=false;
    bool bool_transparent=false;
    char transstr[257];

  public:
    IGBheader( gzFile a = NULL, bool read=false, bool quiet=false );
    IGBheader( FILE *f, bool read=false, bool quiet=false );
    IGBheader( IGBheader *h ) { *this=*h; }
    ~IGBheader();
    int   write();
    int   read( bool quiet=false );
    void  fileptr( gzFile f );
    void  fileptr( FILE* f );
    void* fileptr(void){ return file; }
    void  close(void){if(gzipping)gzclose((gzFile)file);else fclose((FILE*)file);}
    void  swab( void *, int nd=-1 );
    inline double from_raw( double a ){ return a*v_facteur+v_zero; }
    inline double to_raw( double a ){ return (a-v_zero)/v_facteur; }
	inline int  slice_sz(){ return v_x*v_y*v_z; }
    void  comment( char* );
    inline char **comment(void){return v_comment;}
    inline int data_size(void){return Data_Size[v_type];}
    inline int x(void){ return v_x; }
    inline int x( bool &set ){ set=bool_x; return v_x; }
  inline void x( int a ){ v_x = a; bool_x = true;}
    inline int y(void){ return v_y; }
    inline int y( bool &set ){ set=bool_y; return v_y; }
  inline void y( int a ){ v_y = a; bool_y = true;}
    inline int z(void){ return v_z; }
    inline int z( bool &set ){ set=bool_z; return v_z; }
  inline void z( int a ){ v_z = a; bool_z = true;}
    inline int t(void){ return v_t; }
    inline int t( bool &set ){ set=bool_t; return v_t; }
  inline void t( int a ){ v_t = a; bool_t = true;}
    inline int type(void){ return v_type; }
    inline int type( bool &set ){ set=bool_type; return v_type; }
  inline void type( int a ){ v_type = a; bool_type = true; }
    void type( char* );
    inline int taille(void){ return v_taille; }
    inline int taille( bool &set ){ set=bool_taille; return v_taille; }
  inline void taille( int a ){ v_taille = a; bool_taille = true;}
    inline int num(void){ return v_num; }
    inline int num( bool &set ){ set=bool_num; return v_num; }
  inline void num( int a ){ v_num = a; bool_num = true;}
    inline int bin(void){ return v_bin; }
    inline int bin( bool &set ){ set=bool_bin; return v_bin; }
  inline void bin( int a ){ v_bin = a; bool_bin = true;}
    inline int trame(void){ return v_trame; }
    inline int trame( bool &set ){ set=bool_trame; return v_trame; }
  inline void trame( int a ){ v_trame = a; bool_trame = true;}
    const char* systemestr(void);
    inline int systeme( void ){ return v_systeme; }
    void       systeme( const char* s );
    inline unsigned int lut(void){ return v_lut; }
    inline unsigned int lut( bool &set ){ set=bool_x; return v_x; }
  inline void lut( unsigned int a ){ v_lut = a; bool_lut = true; }
    inline unsigned int comp(void){ return v_comp; }
    inline unsigned int comp( bool &set ){ set=bool_lut; return v_lut; }
  inline void comp( unsigned int a ){ v_comp = a; bool_comp = true;}
    inline float epais(void){ return v_epais; }
    inline float epais( bool &set ){ set=bool_epais; return v_epais; }
  inline void epais( float a ){ v_epais = a; bool_epais = true;}
    inline float org_x(void){ return v_org_x; }
    inline float org_x( bool &set ){ set=bool_org_x; return v_org_x; }
  inline void org_x( float a ){ v_org_x = a; bool_org_x = true; }
    inline float org_y(void){ return v_org_y; }
    inline float org_y( bool &set ){ set=bool_org_y; return v_org_y; }
  inline void org_y( float a ){ v_org_y = a; bool_org_y = true; }
    inline float org_z(void){ return v_org_z; }
    inline float org_z( bool &set ){ set=bool_org_z; return v_org_z; }
  inline void org_z( float a ){ v_org_z = a;  bool_org_z  = true;}
    inline float org_t(void){ return v_org_t; }
    inline float org_t( bool &set ){ set=bool_org_t; return v_org_t; }
  inline void org_t( float a ){ v_org_t = a;  bool_org_t = true;}
    inline void inc_x( float a ){ v_inc_x = a;  bool_inc_x = true;}
    inline float inc_x( bool &set ){ set=bool_inc_x; return v_inc_x; }
  inline float inc_x(void){ return v_inc_x; }
    inline void inc_y( float a ){ v_inc_y = a;  bool_inc_y = true;}
    inline float inc_y( bool &set ){ set=bool_inc_y; return v_inc_y; }
  inline float inc_y(void){ return v_inc_y; }
    inline void inc_z( float a ){ v_inc_z = a;  bool_inc_z = true;}
    inline float inc_z( bool &set ){ set=bool_inc_z; return v_inc_z; }
  inline float inc_z(void){ return v_inc_z; }
    inline void inc_t( float a ){ v_inc_t = a;  bool_inc_t = true;}
    inline float inc_t( bool &set ){ set=bool_inc_t; return v_inc_t; }
  inline float inc_t(void){ return v_inc_t; }
    inline void dim_x( float a ){ v_dim_x = a;  bool_dim_x = true;}
    inline float dim_x( bool &set ){ set=bool_dim_x; return v_dim_x; }
  inline float dim_x(void){ return v_dim_x; }
    inline void dim_y( float a ){ v_dim_y = a;  bool_dim_y = true;}
    inline float dim_y( bool &set ){ set=bool_dim_y; return v_dim_y; }
  inline float dim_y(void){ return v_dim_y; }
    inline void dim_z( float a ){ v_dim_z = a;  bool_dim_y = true;}
    inline float dim_z( bool &set ){ set=bool_dim_z; return v_dim_z; }
  inline float dim_z(void){ return v_dim_z; }
    inline void dim_t( float a ){ v_dim_t = a;  bool_dim_t = true;}
    inline float dim_t( bool &set ){ set=bool_dim_t; return v_dim_t; }
  inline float dim_t(void){ return v_dim_t; }
    inline void facteur( float a ){ v_facteur = a; bool_facteur = true; }
    inline float facteur( bool &set ){ set=bool_facteur; return v_facteur; }
  inline float facteur(void){ return v_facteur; }
    inline void zero( float a ){ v_zero = a; bool_zero = true; }
    inline float zero( bool &set ){ set=bool_zero; return v_zero; }
  inline float zero(void){ return v_zero; }
    inline void vect_z( float* a ){ v_vect_z = a; bool_vect_z = true; }
    inline float* vect_z( bool &set ){ set=bool_vect_z; return v_vect_z; }
  inline float* vect_z(void){ return v_vect_z; }
    inline void unites_x( const char* a ){ strcpy(v_unites_x, a); bool_unites_x = true; }
    inline char* unites_x( bool &set ){ set=bool_unites_x; return v_unites_x; }
  inline char* unites_x(void){ return v_unites_x; }
    inline void unites_y( const char* a ){ strcpy(v_unites_y, a); bool_unites_y = true;}
    inline char* unites_y( bool &set ){ set=bool_unites_y; return v_unites_y; }
  inline char* unites_y(void){ return v_unites_y; }
    inline void unites_z( const char* a ){ strcpy(v_unites_z,a); bool_unites_z = true; }
    inline char* unites_z( bool &set ){ set=bool_unites_z; return v_unites_z; }
  inline char* unites_z(void){ return v_unites_z; }
    inline void unites_t( const char* a ){ strcpy(v_unites_t, a); bool_unites_t = true; }
    inline char* unites_t( bool &set ){ set=bool_unites_t; return v_unites_t; }
  inline char* unites_t(void){ return v_unites_t; }
    inline void unites( const char* a ){ strcpy(v_unites, a); bool_unites = true; }
    inline char* unites( bool &set ){ set=bool_unites; return v_unites; }
  inline char* unites(void){ return v_unites; }
    inline void struct_desc( const char* a ){ strcpy(v_struct_desc, a); bool_struct_desc = true; }
    inline char* struct_desc( bool &set ){ set=bool_struct_desc; return v_struct_desc; }
  inline char* struct_desc(void){ return v_struct_desc; }
    inline void aut_name( const char* a ){ strcpy(v_aut_name, a); bool_aut_name = true; }
    inline char* aut_name( bool &set ){ set=bool_aut_name; return v_aut_name; }
  inline char* aut_name(void){ return v_aut_name; }
    inline void  transparent( void* a ){ v_transparent=a; bool_transparent = true; }
    inline void* transparent( bool &set ){ set=bool_transparent; return v_transparent; }
  inline void* transparent(void){ return v_transparent; }
    inline char* transparentstr(void){ return transstr; }
    int  num_components(){ return Num_Components[v_type]; }
    int  endian();
    template<class T> int read_data( T* dp, int numt=1, char *buf=NULL );
    template<class T> void write_data( T* dp, int numt=1, char *buf=NULL );
    template<class T> void to_bin( void *buf, T d );
    template<class T> T convert_buffer_datum( void *buf, int a );
};


/** write out a number of time slices 
 *
 * \param dp[in] buffer of data
 * \param numt    number of time slices
 * \param buf     temporary buffer of raw data
 */
template<class T>
void
IGBheader::write_data( T* dp, int numt, char *buf )
{
  int    slicesize = data_size()*slice_sz()*numt;
  bool   alloc_buf = false;
  if ( buf==NULL ) {
    buf = new char[slicesize];
    alloc_buf = true;
  }
  
  int numprimitive = slice_sz()*num_components(); 
  for ( int a=0; a<numprimitive; a++ ) 
    to_bin<T>( buf+a*data_size(), dp[a] );

  if( gzipping ) 
    gzwrite( (gzFile)file, buf, slicesize );
  else
    fwrite( buf, 1, slicesize, (FILE*)file );

  if ( alloc_buf ) delete[] buf;
}


/** read in a number of time slices 
 *
 * \param dp[out] values read
 * \param numt    number of time slices
 * \param buf     temporary buffer of raw data
 *
 * \pre    \p dp must be allocated
 * \post   \dp will be filled
 * \return the number of items read
 */
template<class T>
int
IGBheader::read_data( T* dp, int numt, char *buf )
{
  int    slicesize = data_size()*slice_sz()*numt;
  bool   alloc_buf = false;
  if ( buf==NULL ) {
    buf = new char[slicesize];
    alloc_buf = true;
  }
  
  int numread;
  if( gzipping ) 
    numread = gzread( (gzFile)(file), buf, slicesize )/data_size();
  else
    numread = fread(  buf, slice_sz()*numt, data_size(), (FILE *)(file) );

  if ( systeme() != endian() ) swab(buf, numread);

  int numprimitive = numread*num_components(); // adjust vector types

  for ( int a=0; a<numprimitive; a++ )
    dp[a] = convert_buffer_datum<T>( buf, a );
  
  if ( alloc_buf ) delete[] buf;
  return numread;
}


/** convert the IGB data to the proper type
 *
 * \param h     IGB header
 * \param buf   raw IGB data
 * \param a     index of datum
 */
template<class T>
T IGBheader::convert_buffer_datum( void *buf, int a )
{
  T  datum;

  switch ( type() ) {
    case IGB_BYTE:
      datum = ((unsigned char *)buf)[a];
      break;
    case IGB_CHAR:
      datum = ((signed char *)buf)[a];
      break;
    case IGB_SHORT:
      datum = ((short *)buf)[a];
      break;
    case IGB_LONG:
      datum = ((long *)buf)[a];
      break;
    case IGB_FLOAT:
    case IGB_VEC3_f:
    case IGB_VEC4_f:
      datum = ((float *)buf)[a];
      break;
    case IGB_DOUBLE:
    case IGB_VEC3_d:
    case IGB_VEC4_d:
      datum = ((double *)buf)[a];
      break;
    case IGB_INT:
      datum = ((int *)buf)[a];
      break;
    case IGB_UINT:
      datum = ((unsigned int *)buf)[a];
      break;
    case IGB_USHORT:
      datum = ((unsigned short *)buf)[a];
      break;
    case IGB_HFLOAT:
      datum = floatFromShort(((short_float *)buf)[a]);
      break;
    default:
      memset(&datum,0,sizeof(datum));
  }
  return datum=from_raw(datum);
}

/** 
 * \def CONVERT_TYPE(D,m,M,B) 
 *      convert the value stored in variable \b datum to type \a D,
 *      clipping it to the range [\a m, \a M] and store it in the location 
 *      pointed to by \a B
 */
#define CONVERT_TYPE(D,m,M,B) { if(datum<m)datum=m;else if(datum>M)datum=M; \
                D a0 = (D)datum; *((D*)(B))=a0;}; 

/** convert the data to the binary representation
 *
 * \param buf   place to put raw binary IGB data
 * \param d     datum
 */
template<class T>
void 
IGBheader::to_bin( void *buf, T d )
{
  double datum=to_raw(d);
  switch ( type() ) {
      case IGB_BYTE:
          CONVERT_TYPE( unsigned char, 0, UCHAR_MAX, buf )
          break;
      case IGB_CHAR:
          CONVERT_TYPE( char, CHAR_MIN, CHAR_MAX, buf )
          break;
      case IGB_SHORT:
          CONVERT_TYPE( short, SHRT_MIN, SHRT_MAX, buf )
          break;
      case IGB_LONG:
          CONVERT_TYPE( long, LONG_MIN, LONG_MAX, buf )
          break;
      case IGB_FLOAT:
          CONVERT_TYPE( float, FLT_MIN, FLT_MAX, buf )
          break;
      case IGB_VEC3_f:
      case IGB_VEC4_f:
          assert(0);
          break;
      case IGB_DOUBLE:
          CONVERT_TYPE( double, DBL_MIN, DBL_MAX, buf )
          break;
      case IGB_VEC3_d:
      case IGB_VEC4_d:
          assert(0);
          break;
      case IGB_INT:
          CONVERT_TYPE( int, INT_MIN, INT_MAX, buf )
          break;
      case IGB_UINT:
          CONVERT_TYPE( unsigned int, 0, UINT_MAX, buf )
          break;
      case IGB_USHORT:
          CONVERT_TYPE( unsigned short, 0, USHRT_MAX, buf )
          break;
      case IGB_HFLOAT:
          float fdatum; 
          CONVERT_TYPE( float, HFLT_MIN, HFLT_MAX, &fdatum )
          *((short_float*)buf) = shortFromFloat(fdatum);
          break;
      default:
          assert(0);
  }
}

#endif	//IGBheader_h

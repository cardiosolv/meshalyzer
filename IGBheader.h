/* src/IGBheader.h.  Generated automatically by configure.  */
#ifndef IGBheader_h
#define IGBheader_h

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <zlib.h>
#include <cstring>
#include <cctype>
#include<assert.h>

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
#define     IGB_VEC3_f    15 /* -- 3 X float ------------------------------ */
#define     IGB_VEC3_d    16 /* -- 3 X double ----------------------------- */
#define     IGB_VEC4_f    17 /* -- 4 X float ------------------------------ */
#define     IGB_VEC4_d    18 /* -- 4 X double ----------------------------- */
#define	    IGB_MIN_TYPE	  1
#define	    IGB_MAX_TYPE	 18

#define Byte hByte

/* define for endedness */
#define IGB_ENDIAN_VAL -1.24e5
#define IGB_LITTLE_END_REP 0,48,242,199


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

template<class T, int M>
class IGB_Vec {
	public:
		T d[M];
        T& operator[](const int indx){return d[indx];}
		operator float (void) {return sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);}
		IGB_Vec<T,M>(void){}
		IGB_Vec<T,M>(double &f){ for(int i=0; i<M; i++ )d[i]=*(&f+i);}
	    IGB_Vec<T,M>& operator=(float *f){memcpy(d,f,M*sizeof(int));return *this;}
};
typedef IGB_Vec<float,3>  IGB_Vec3_f;
typedef IGB_Vec<float,4>  IGB_Vec4_f;
typedef IGB_Vec<double,3> IGB_Vec3_d;
typedef IGB_Vec<double,4> IGB_Vec4_d;


/* Indice de chaque composante dans le vecteur b[] de l'union rgba */
#define RGBA_ROUGE 3
#define RGBA_VERT  2
#define RGBA_BLEU  1
#define RGBA_ALPHA 0


/* -------------- Definition du type des variables globales de header.c - */

#ifndef	HEADER_GLOBALS
extern	int		 	   Header_Quiet;
extern	const char    *Header_Type[IGB_MAX_TYPE+1];
extern	unsigned short Data_Size[IGB_MAX_TYPE+1];
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
    void* file;					//!< file ptr to read/write header
    bool gzipping;		  		// are we gzipping data?
    int   v_x, v_y, v_z, v_t ;  //!< dimensions --------------------
    int   v_type ;              //!< type arithmetique -------------
    int   v_taille ;            //!< taille des pixels (type STRUCTURE)
    unsigned long v_systeme ;   //!< systeme informatique ----------
    int    v_num ;              //!< numero de la tranche ----------
    int    v_bin ;              //!< nombre de couleurs ------------
    int    v_trame ;            //!< trame(connectivite) -----------
    unsigned int v_lut ;        //!< nombre de bytes table couleurs
    unsigned int v_comp ;       //!< nombre de bytes table compres.
    float  v_epais ;            //!< epaiseur d'une tranche --------
    float  v_org_x, v_org_y, v_org_z, v_org_t ; //!< coin sup gauche --------
    float  v_inc_x, v_inc_y, v_inc_z, v_inc_t ; //!< distance entre pixels -
    float  v_dim_x, v_dim_y, v_dim_z, v_dim_t ; //!< dimension totale -------
    float  v_fac_x, v_fac_y, v_fac_z, v_fac_t ; //!< facteurs d'echelle par pixel en X, Y, Z et T
    float *v_vect_z ;           //!< coord z de chaque tranche -----
    char   v_unites_x[41], v_unites_y[41], v_unites_z[41], v_unites_t[41] ;
    //!< unites de mesure --------------
    char   v_unites[41] ;       //!< unites de mesure pour les valeurs des pixels -----------
    float  v_facteur, v_zero ;  //!< facteur d'echelle et valeur du zero -
    char   v_struct_desc[41] ;  //!< description de la structure ---
    char   v_aut_name[41] ;     //!< nom de l'auteur ---------------
    char** v_comment ;          //!< commentaires ------------------
    void*  v_transparent;		//!< transparent value for data
    int    puts_fcn(void *, char *);

    // boolean flags to indicate if a default value has been overridden
    bool bool_x, bool_y, bool_z, bool_t;
    bool bool_type;
    bool bool_taille;
    bool bool_num;
    bool bool_bin;
    bool bool_trame;
    bool bool_lut;
    bool bool_comp;
    bool bool_epais;
    bool bool_org_x, bool_org_y, bool_org_z, bool_org_t;
    bool bool_inc_x, bool_inc_y, bool_inc_z, bool_inc_t;
    bool bool_dim_x, bool_dim_y, bool_dim_z, bool_dim_t;
    bool bool_fac_x, bool_fac_y, bool_fac_z, bool_fac_t;
    bool bool_vect_z;
    bool bool_unites_x, bool_unites_y, bool_unites_z, bool_unites_t;
    bool bool_unites;
    bool bool_facteur, bool_zero;
    bool bool_struct_desc;
    bool bool_aut_name;
    bool bool_comment ;
    bool bool_transparent;
    char transstr[257];

  public:
    IGBheader( gzFile a = NULL );
    ~IGBheader();
    int   write();
    int   read();
    void  fileptr( gzFile f );
    void  fileptr( FILE* f );
    void* fileptr(void){ return file; }
    void  swab( void *, int nd=-1 );
    inline double from_raw( double a ){ return a/v_facteur+v_zero; }
    inline double to_raw( double a ){ return (a-v_zero)*v_facteur; }
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
    inline float inc_x( float a ){ v_inc_x = a;  bool_inc_x = true;}
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
    inline void fac_x( float a ){ v_fac_x = a; bool_fac_x = true; }
    inline float fac_x( bool &set ){ set=bool_fac_x; return v_fac_x; }
  inline float fac_x(void){ return v_fac_x; }
    inline void fac_y( float a ){ v_fac_y = a; bool_fac_y = true; }
    inline float fac_y( bool &set ){ set=bool_fac_y; return v_fac_y; }
  inline float fac_y(void){ return v_fac_y; }
    inline void fac_z( float a ){ v_fac_z = a; bool_fac_z = true; }
    inline float fac_z( bool &set ){ set=bool_fac_z; return v_fac_z; }
  inline float fac_z(void){ return v_fac_z; }
    inline void fac_t( float a ){ v_fac_t = a; bool_fac_t = true; }
    inline float fac_t( bool &set ){ set=bool_fac_t; return v_fac_t; }
  inline float fac_t(void){ return v_fac_t; }
    inline void facteur( float a ){ v_facteur = a; bool_facteur = true; }
    inline float facteur( bool &set ){ set=bool_facteur; return v_facteur; }
  inline float facteur(void){ return v_facteur; }
    inline void zero( float a ){ v_zero = a; bool_zero = true; }
    inline float zero( bool &set ){ set=bool_zero; return v_zero; }
  inline float zero(void){ return v_zero; }
    inline void vect_z( float* a ){ v_vect_z = a; bool_vect_z = true; }
    inline float* vect_z( bool &set ){ set=bool_vect_z; return v_vect_z; }
  inline float* vect_z(void){ return v_vect_z; }
    inline void unites_x( char* a ){ strcpy(v_unites_x, a); bool_unites_x = true; }
    inline char* unites_x( bool &set ){ set=bool_unites_x; return v_unites_x; }
  inline char* unites_x(void){ return v_unites_x; }
    inline void unites_y( char* a ){ strcpy(v_unites_y, a); bool_unites_y = true;}
    inline char* unites_y( bool &set ){ set=bool_unites_y; return v_unites_y; }
  inline char* unites_y(void){ return v_unites_y; }
    inline void unites_z( char* a ){ strcpy(v_unites_z,a); bool_unites_z = true; }
    inline char* unites_z( bool &set ){ set=bool_unites_z; return v_unites_z; }
  inline char* unites_z(void){ return v_unites_z; }
    inline void unites_t( char* a ){ strcpy(v_unites_t, a); bool_unites_t = true; }
    inline char* unites_t( bool &set ){ set=bool_unites_t; return v_unites_t; }
  inline char* unites_t(void){ return v_unites_t; }
    inline void unites( char* a ){ strcpy(v_unites, a); bool_unites = true; }
    inline char* unites( bool &set ){ set=bool_unites; return v_unites; }
  inline char* unites(void){ return v_unites; }
    inline void struct_desc( char* a ){ strcpy(v_struct_desc, a); bool_struct_desc = true; }
    inline char* struct_desc( bool &set ){ set=bool_struct_desc; return v_struct_desc; }
  inline char* struct_desc(void){ return v_struct_desc; }
    inline void aut_name( char* a ){ strcpy(v_aut_name, a); bool_aut_name = true; }
    inline char* aut_name( bool &set ){ set=bool_aut_name; return v_aut_name; }
  inline char* aut_name(void){ return v_aut_name; }
    inline void  transparent( void* a ){ v_transparent=a; bool_transparent = true; }
    inline void* transparent( bool &set ){ set=bool_transparent; return v_transparent; }
  inline void* transparent(void){ return v_transparent; }
    inline char* transparentstr(void){ return transstr; }
    int  endian();
};


/** read in raw data and convert to proper value of proper type
 *
 * \param dp    data buffer
 * \param numt  \#time slices
 * \param h     IGB header
 *
 * \return number of elements read
 */
template <class T>
int
read_IGB_data( T* dp, int numt, IGBheader* h, char *buf )
{
  int    slicesize = h->data_size()*h->x()*h->y()*h->z()*numt;
  bool   alloc_buf = false;
  if ( buf==NULL ) {
    buf = new char[slicesize];
    alloc_buf = true;
  }
  int numread = gzread( h->fileptr(), buf, slicesize )/h->data_size();
  if ( numread == Z_NULL ) return 0;
  if ( h->systeme() != h->endian() ) h->swab(buf, numread);
  for ( int a=0; a<numread; a++ )
    dp[a] = IGB_convert_buffer_datum<T>( h, buf, a );
  if ( alloc_buf ) delete[] buf;
  return numread;
}


/** convert the IGB data to the proper type
 *
 * \param type  type of IGB data
 * \param buf   raw IGB data
 * \param a     index of datum
 */
template<class T>
T IGB_convert_buffer_datum( IGBheader *h, void *buf, int a )
{
  T  datum;

  switch ( h->type() ) {
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
      datum = ((float *)buf)[a];
      break;
    case IGB_DOUBLE:
      datum = ((double *)buf)[a];
      break;
    case IGB_INT:
      datum = ((int *)buf)[a];
      break;
    case IGB_UINT:
      datum = ((unsigned int *)buf)[a];
      break;
    case IGB_VEC3_f:
      datum = ((float *)buf)[a];
      break;
    default:
      memset(&datum,0,sizeof(datum));
  }
  return h->from_raw(datum);
}


#endif	//IGBheader_h

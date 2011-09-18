/**********************************************************************
 
  igbhead - program to display and remove/modify/create IGB headers

  usage igbhead [options] file

  author: Edward Vigmond

 **********************************************************************/
#include <iostream>
#include <cstdio>
#include <string>
#include "IGBheader.h"
#include "cmdline.h"
using namespace std;

void output_header( IGBheader* );
int  jive(IGBheader *header, gzFile in);
void write_slice(IGBheader *header, gzFile in, string ofnb, void *buf, int bufsize, int, bool memfem);
bool iscompressed (string fn);

class FILE_PTR {
	public:
		FILE_PTR( gzFile g ):gzip(true),gp(g){}
		FILE_PTR( FILE *f ):gzip(false),fp(f){}
		void close(){ if ( gzip )gzclose(gp); else fclose(fp); } 
		void write(void *buf, int n){if(gzip) gzwrite( gp, buf, n );
		                             else fwrite( buf, n, 1, fp ); }
		void printfl(float f){if(gzip)gzprintf(gp,"%.7f\n",f);else
						          fprintf( fp, "%.7f\n", f ); }
	private:
        bool    gzip;
		FILE   *fp;
		gzFile  gp;
};
		
template<class T>
void write_buffer(FILE_PTR *fp, T* buf, int n, IGBheader* h )
{
  for(int i=0;i<n;i++)
    fp->printfl( buf[i]/h->facteur()+h->zero() );
}


main( int argc, char* argv[] )
{
  gengetopt_args_info args_info;

  // let's call our cmdline parser 
  if (cmdline_parser (argc, argv, &args_info) != 0)
     exit(1);

  if( argc == 1 || args_info.inputs_num != 1 ) {
	cerr << "\nigbhead [OPTIONS] file" << endl;
	cerr << "\tadd, edit or remove igb headers" << endl;
	cerr << "\trun igbhead --help to see options" << endl << endl;
	exit(0);
  }

  // the last arg must be a file name
  gzFile in = gzopen( args_info.inputs[0], "r" );
  if( in == NULL ) {
	  cerr << "File not found: " << args_info.inputs[0] << endl;
	  exit(1);
  }
  IGBheader* header= new IGBheader( in );
  if( header->read() == 0 ) gzrewind( in );

  // just output the current header
  if( argc==2 ) {
	output_header( header );
	gzclose( in );
	exit(0);
  }

  gzseek( in, 0, SEEK_CUR ); // nop so that I/O gets synced

  if( args_info.x_given ) header->x( args_info.x_arg );
  if( args_info.y_given ) header->y( args_info.y_arg );
  if( args_info.z_given ) header->z( args_info.z_arg );
  if( args_info.t_given ) header->t( args_info.t_arg );
  if( args_info.data_type_given ) header->type( args_info.data_type_arg );
  if( args_info.system_given ) header->systeme( args_info.system_arg );
  if( args_info.dim_x_given ) header->dim_x( args_info.dim_x_arg );
  if( args_info.dim_y_given ) header->dim_y( args_info.dim_y_arg );
  if( args_info.dim_z_given ) header->dim_z( args_info.dim_z_arg );
  if( args_info.dim_t_given ) header->dim_t( args_info.dim_t_arg );
  if( args_info.org_x_given ) header->org_x( args_info.org_x_arg );
  if( args_info.org_y_given ) header->org_y( args_info.org_y_arg );
  if( args_info.org_z_given ) header->org_z( args_info.org_z_arg );
  if( args_info.org_t_given ) header->org_t( args_info.org_t_arg );
  if( args_info.inc_x_given ) header->inc_x( args_info.inc_x_arg );
  if( args_info.inc_y_given ) header->inc_y( args_info.inc_y_arg );
  if( args_info.inc_z_given ) header->inc_z( args_info.inc_z_arg );
  if( args_info.inc_t_given ) header->inc_t( args_info.inc_t_arg );
  if( args_info.units_given ) header->unites( args_info.units_arg );
  if( args_info.x_units_given ) header->unites_x( args_info.x_units_arg );
  if( args_info.y_units_given ) header->unites_y( args_info.y_units_arg );
  if( args_info.z_units_given ) header->unites_z( args_info.z_units_arg );
  if( args_info.t_units_given ) header->unites_t( args_info.t_units_arg );
  if( args_info.clear_comment_given ) header->comment(NULL);
  if( args_info.comment_given ) header->comment( args_info.comment_arg );
  if( args_info.data_factor_given ) header->facteur(args_info.data_factor_arg);
  if( args_info.data_zero_given ) header->zero( args_info.data_zero_arg );
  if( args_info.author_given ) header->aut_name( args_info.author_arg );
  if( args_info.transparent_given ){
	if( strlen(args_info.transparent_arg)!=2*Data_Size[header->type()] ){
	  cerr << "Incorrect tranparent value specified\n";
	  exit(1);
	} 
	// convert hex digits to bytes
	char* v = new char[Data_Size[header->type()]];
	char s[3], *pp;
	s[2] = '\0';
	for( int i=0; i<Data_Size[header->type()]; i++ ){
	  s[0] = args_info.transparent_arg[i*2];
	  s[1] = args_info.transparent_arg[i*2+1];
	  v[i] = strtol( s, &pp, 16 );
	}
	header->transparent( v );
  }
  if( args_info.no_transparent_given ) header->transparent( NULL );

  if( args_info.explode_given || args_info.memfeminize_given ) {
	// jive time first
	int   slicesize = jive(header, in);

	// save file pointer
	z_off_t zo = gztell( in );
	int nslcs = header->t();
	header->t(1);
    string ofnb = args_info.inputs[0];
	char *buf = (char *)malloc(sizeof(char)*slicesize);

	bool  memfem = false;
	if( args_info.memfeminize_given )
	  memfem = true;

	for(int i=0;i<nslcs;i++)
	  write_slice(header, in, ofnb, buf, slicesize, i, memfem);

	free(buf);
	header->t(nslcs);
	gzseek(in, zo, SEEK_SET );
  }

  // count the number of data bytes 
  if( args_info.jive_time_given )
	jive( header, in );

  // make a temporary file
  string tmpfn = ".";
  tmpfn += args_info.inputs[0];
  tmpfn += ".tmp";
 
  // determine output file
  string ofn;
  if( args_info.frankenstein_given ){
	  ofn = args_info.frankenstein_arg;
	  gzclose( in );
	  IGBheader *h = new IGBheader(in=gzopen( args_info.frankenstein_arg,"r" ));
	  if( h->read() == 0 ) gzrewind( in );
	  delete h;;
  } else
	  ofn = args_info.inputs[0]; 
  if(args_info.output_file_given)
	  ofn = args_info.output_file_arg;
	  
  FILE_PTR *out_fp;
  if( iscompressed(ofn) ) {
    gzFile gzout;
	header->fileptr(gzout = gzopen( tmpfn.c_str(), "w" ) );
    out_fp = new FILE_PTR( gzout );
  } else {
    FILE* fout;
	header->fileptr(fout= fopen( tmpfn.c_str(), "w" ) );
    out_fp = new FILE_PTR( fout );
  }
  header->write();

  const int bufsize=1024;
  char buf[bufsize];
  int nb;
  while( (nb=gzread( in, buf, bufsize )) > 0 ) out_fp->write( buf, nb );
  out_fp->close();
  gzclose( in );
  rename(  tmpfn.c_str(), ofn.c_str() );
  delete out_fp;
}

void output_header( IGBheader* header )
{
	bool tf;
    printf( "x dimension:\t%d\n", header->x() );
    printf( "y dimension:\t%d\n", header->y() );
    printf( "z dimension:\t%d\n", header->z() );
    printf( "t dimension:\t%d\n", header->t() );
    printf( "data type:\t%s\n", Header_Type[header->type()] );
    header->unites(tf);
	if( tf ) {
        printf( "Pixel units:\t%s\n", header->unites() );
  	}
	if( header->transparent() != NULL ) {
        printf( "Transparent:\t%s\n", header->transparentstr() );
	}
    header->zero(tf);
	if( tf ) {
        printf( "Pixel zero:\t%g\n", header->zero() );
    }
    header->facteur(tf);
	if( tf ) {
        printf( "Pixel scaling:\t%g\n", header->facteur() );
    }
    header->dim_x(tf);
	if( tf ) {
        printf( "X size:\t\t%g\n", header->dim_x() );
    }
    header->unites_x(tf);
	if( tf ) {
        printf( "X units:\t%s\n", header->unites_x() );
    }
    header->inc_x(tf);
	if( tf ) {
        printf( "Increment in x:\t%g\n", header->inc_x() );
    }
    header->org_x(tf);
	if( tf ) {
        printf( "X origin:\t%g\n", header->org_x() );
    }
    header->dim_y(tf);
	if( tf ) {
        printf( "Y size:\t\t%g\n", header->dim_y() );
    }
    header->unites_y(tf);
	if( tf ) {
        printf( "Y units:\t%s\n", header->unites_y() );
    }
    header->inc_y(tf);
	if( tf ) {
        printf( "Increment in y:\t%g\n", header->inc_y() );
    }
    header->org_y(tf);
	if( tf ) {
        printf( "Y origin:\t%g\n", header->org_y() );
    }
    header->dim_z(tf);
	if( tf ) {
        printf( "Z size:\t\t%g\n", header->dim_z() );
    }
    header->unites_z(tf);
	if( tf ) {
        printf( "Z units:\t%s\n", header->unites_z() );
    }
    header->inc_z(tf);
	if( tf ) {
        printf( "Increment in z:\t%g\n", header->inc_z() );
    }
    header->org_z(tf);
	if( tf ) {
        printf( "Z origin:\t%g\n", header->org_z() );
    }
    header->dim_t(tf);
	if( tf ) {
        printf( "T size:\t\t%g\n", header->dim_t() );
    }
	header->unites_t(tf);
	if( tf ) {
        printf( "T units:\t%s\n", header->unites_t() );
    }
    header->inc_t(tf);
	if( tf ) {
        printf( "Increment in t:\t%g\n", header->inc_t() );
    }
    header->org_t(tf);
	if( tf ) {
        printf( "T origin:\t%g\n", header->org_t() );
    }
	printf( "Created on:\t%s\n", header->systemestr());
    header->aut_name(tf);
	if( tf ) {
        printf( "Author:\t\t%s\n", header->aut_name() );
    }
	if( header->comment() != NULL ){
	  	char **cm = header->comment();
		int i=0;
		while( cm[i] != NULL )
        	printf( "#%s\n", cm[i++] );
    }
}

int jive(IGBheader *header, gzFile in)
{

  // count the number of data bytes
  z_off_t zo = gztell( in );
  const int bufsize=8196;
  char buff[bufsize];
  long long nb=0, nr;
  while( (nr=gzread( in, buff, bufsize )) == bufsize )
    nb += nr;
  nb += nr;
  int slicesize = header->x()*header->y()*header->z()*header->data_size();
  header->t( nb/slicesize );
  gzseek( in, zo, SEEK_SET );
  return(slicesize);
}

/** write slices when exploding or memfeminizing
 */
void write_slice(IGBheader *header, gzFile in, string ofnb, void *buf, int bufsize, int slnum, bool memfem)
{
  long   nr;

  string ofn = ofnb;
  if( ofn.rfind(".gz")==ofn.length()-3 ) ofn.resize(ofn.length()-3);
  if( ofn.rfind(".igb")==ofn.length()-4 ) ofn.resize(ofn.length()-3);

  char   fnm[256];
  if( (nr=gzread( in, buf, bufsize )) == bufsize )  {
	
    sprintf( fnm, "%s%d.%s", ofn.c_str(), slnum, !memfem?"igb":"dat" );

	FILE_PTR *fp;

	if( iscompressed(ofnb) ) {
      gzFile gzout;
	  header->fileptr(gzout = gzopen( strcat(fnm,".gz"), "w" ) );
	  fp = new FILE_PTR( gzout );
	} else {
      FILE * fout;
	  header->fileptr(fout= fopen( fnm, "w" ) );
	  fp = new FILE_PTR( fout );
	}

	if(!memfem)  {
	  header->write();
	  fp->write( buf, bufsize );
	  fp->close();
	} else  {
	  int     ntokens = bufsize/header->data_size();

	  switch(header->type())  {

		  case IGB_FLOAT:
			  write_buffer( fp, (float *)buf, ntokens, header );
			  break;

		  case IGB_DOUBLE:
			  write_buffer( fp, (double *)buf, ntokens, header );
			  break;

		  case IGB_SHORT:
			  write_buffer( fp, (short *)buf, ntokens, header );
			  break;

		  case IGB_INT:
			  write_buffer( fp, (int *)buf, ntokens, header );
			  break;

		  case IGB_VEC3_f:
			  write_buffer( fp, (float *)buf, 3*ntokens, header );
			  break;

		  case IGB_VEC4_f:
			  write_buffer( fp, (float *)buf, 4*ntokens, header );
			  break;

		  default:
			  fprintf(stderr, "Datatype %s not supported by --explode || --memfemize yet", Header_Type[header->type()]);
		exit(-1);
	  }
	  fp->close( );
	}
    delete fp;
  } else {
    fprintf(stderr, "Not enough slices in file\n" );
    exit(-1);
  }
}

bool iscompressed (string fn)
{
  return fn.rfind(".gz") == fn.length()-3; 
}

#include "PNGwrite.h"
#include <stdlib.h>

PNGwrite :: PNGwrite( FILE *out ) : 
            width(0),height(0),ctype(PNG_COLOR_TYPE_RGB),
            interlace_type(PNG_INTERLACE_NONE),colour_depth(8)
{
  fp = out;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );

  if (png_ptr == NULL) {
    fclose(fp);
    return;
  }
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
    return;
  }
  setjmp(png_jmpbuf(png_ptr));
  png_init_io(png_ptr, fp);
}

PNGwrite :: ~PNGwrite( void )
{}


/** write the frame buffer to a PNG file
 *
 * \param data  the buffer contents
 * \param align data alignment for each row
 */
int PNGwrite :: write( void *data, int align )
{
  setjmp(png_jmpbuf(png_ptr));

  png_set_IHDR( png_ptr, info_ptr, width, height, colour_depth, ctype,
                interlace_type, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  if ( ctype == PNG_COLOR_TYPE_PALETTE )
    png_set_PLTE( png_ptr, info_ptr, cpalette, colour_depth );

  png_write_info(png_ptr, info_ptr);
  png_bytep row_pointers[height];
  int compsperpixel;
  if ( ctype == PNG_COLOR_TYPE_GRAY )
    compsperpixel = 1;
  else if ( ctype == PNG_COLOR_TYPE_GRAY_ALPHA )
    compsperpixel = 2;
  else if ( ctype == PNG_COLOR_TYPE_PALETTE )
    compsperpixel = 1;
  else if ( ctype == PNG_COLOR_TYPE_RGB )
    compsperpixel = 3;
  else if ( ctype == PNG_COLOR_TYPE_RGB_ALPHA )
    compsperpixel = 4;
  //flip vertically and make sure rows are aligned
  int pixel_data = compsperpixel*width*colour_depth/8;
  int pad = (align - (pixel_data%align))%align; 
  for ( png_uint_32 k = 0; k < height; k++) {
    row_pointers[height-1-k] = (png_byte *)data + (pixel_data+pad)*k;
  }
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  fclose( fp );
  return 1;
}

void
PNGwrite :: description( string datafile, string colour_range ) 
{
  png_text tptr[3];

  for( int i=0; i<3; i++ ) {
    tptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
  }

  tptr[1].key         = "Description";
  char *a             = strdup(colour_range.c_str());
  tptr[1].text        = a;
  
  tptr[0].key         = "Software";
  tptr[0].text        = "meshalyzer";
  
  tptr[2].key         = "Data file";
  char *b             = strdup(datafile.c_str());
  tptr[2].text        = b;
  
  png_set_text(png_ptr, info_ptr, tptr, datafile.length()?3:1 );
  
  free(a) ;
  free(b) ;
} 



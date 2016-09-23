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
PNGwrite :: description( const char* datafile, const char *colour_range, const char *tm ) 
{
  const int ntext = 4;

  png_text tptr[ntext];
  char *strptr[ntext];

  for( int i=0; i<ntext; i++ ) {
    tptr[i].compression = PNG_TEXT_COMPRESSION_NONE;
  }

  tptr[0].key         = "Software";
  tptr[0].text        = "meshalyzer";
  
  tptr[1].key         = "Description";
  strptr[1]           = strdup(colour_range);
  tptr[1].text        = strptr[1];
  
  tptr[2].key         = "Data file";
  strptr[2]           = strdup(datafile);
  tptr[2].text        = strptr[2];
  
  tptr[3].key         = "Data frame";
  strptr[3]           = strdup(tm);
  tptr[3].text        = strptr[3];
  
  png_set_text(png_ptr, info_ptr, tptr, strlen(datafile)?ntext:1 );
  
  for( int i=1; i<ntext; i++ ) 
    free(strptr[i]);
} 



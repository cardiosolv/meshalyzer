/*
	flounder - copyright 2002 Edward J. Vigmond

	This file is part of flounder.

	flounder is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

	flounder is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
    along with flounder; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "PNGwrite.h"

PNGwrite :: PNGwrite( FILE *out )
{
  fp = out;
  width = height = 0;
  ctype = PNG_COLOR_TYPE_RGB;
  interlace_type = PNG_INTERLACE_NONE;
  colour_depth = 8;

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

int PNGwrite :: write( void *data )
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
  //flip vertically
  for ( png_uint_32 k = 0; k < height; k++)
    row_pointers[height-1-k] = (png_byte *)data + compsperpixel*k*width*colour_depth/8;
  png_write_image(png_ptr, row_pointers);
  png_write_end(png_ptr, info_ptr);
  png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
  fclose( fp );
  return 1;
}


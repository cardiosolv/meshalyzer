#include <GLee.h>
#include "Frame.h"

#ifdef OSMESA
#include <GL/osmesa.h>
#endif 
#define COUT_ERROR(A) if(gle==A)cout<<#A<<endl;

Frame :: Frame( TBmeshWin *t ) : _tbwm(t), _buffer(NULL)
{
}


Frame :: Frame( TBmeshWin *t, int w, int h, string fname ) : _tbwm(t), _buffer(NULL)
{
  dump( w, h, fname );
}


Frame :: ~Frame()  
{
  delete_objs();
}


void 
Frame :: delete_objs()
{
#ifdef OSMESA
  OSMesaDestroyContext( _ctx );
#else
  delete[] _buffer;
  glDeleteRenderbuffersEXT(1, &_color_rb);
  glDeleteRenderbuffersEXT(1, &_depth_rb);
  glDeleteFramebuffersEXT(1, &_fb);
#endif // OSMESA
}


/** output specified frames
 *
 * \param w     output image width
 * \param h     output image height
 * \param fname base file name
 * \param f0    first frame number
 * \param f1    largest frame = f0+n-1
 * \param stride output frame number stride
 *
 * \note \p fname will have ".png" removed if present and XXXXX.png 
 *       where XXXXX is the frame number
 *
 * \note the frame numbers written are those which satisfy f0<=n<=f1
 *
 * \return the number of frames written
 */
int
Frame :: write( int w, int h, string fname, int f0, int f1, int stride )
{
  if( !_tbwm->set_time(f0) ) return 0;

  if( fname.length()>=4 && (fname.substr(fname.length()-4)==".png") )
    fname = fname.substr(0,fname.length()-4);

  int numout=0;
  for( int f=f0; f<=f1; f+=stride) {
    if( !_tbwm->set_time(f) )
      break;
    string file=fname;
    char strnum[8];
    sprintf( strnum, "%05d.png", f );
    file += strnum;
    dump( w, h, file );
    numout++;
  }
  return  numout;
}


/** output specified frame
 *
 * \param w     output image width
 * \param h     output image height
 * \param fname file name
 * \param f     frame number
 *
 * \note \p fname will have ".png" appended if not present
 */
int
Frame :: write( int w, int h, string fname, int f )
{
  if( !_tbwm->set_time(f) ) return 0;

  if( fname.length()<4 || (fname.substr(fname.length()-4)!=".png") )
    fname+= ".png";

  dump( w, h, fname );
  return 1;
}


/* dump the frame buffer into a file 
 *
 * \param w
 * \param h
 * \param fname
 */
void 
Frame :: dump( int w, int h, string fname )
{
  FILE *out = fopen( fname.c_str(), "w" );
  PNGwrite* pngimg = new PNGwrite( out );
  pngimg->size( w, h );
  pngimg->depth( 8*sizeof(GLubyte) );

  char *datafile, cscale[1024]={0};
  if( _tbwm->dataBuffer ) {
    datafile = realpath( _tbwm->dataBuffer->file().c_str(), NULL );
    sprintf( cscale, "data range = [%f, %f], %d levels", _tbwm->cs->min(), _tbwm->cs->max(), _tbwm->cs->size() );
  } else
    datafile = (char *)calloc(1,1);

  pngimg->description( datafile, cscale );

  free(datafile );

  if( _tbwm->transBgd() )
    pngimg->colour_type( PNG_COLOR_TYPE_RGB_ALPHA );
  else
    pngimg->colour_type( PNG_COLOR_TYPE_RGB );

  if( _buffer==NULL || _w!=w || _h!=h ) {   // resize 
    
    if( _buffer )   // not first time, destroy old buffers/contexts
      delete_objs();
	
    _w = w;
    _h = h;
    _buffer = new GLubyte[w*h*4];

#ifdef OSMESA
    _ctx = OSMesaCreateContext(OSMESA_RGBA, NULL);

    if (!OSMesaMakeCurrent(_ctx, _buffer, GL_UNSIGNED_BYTE, w, h)) {
      cout << "Failed OSMesaMakeCurrent" << endl;
      throw 1;
    }
#else 
    // create new frame buffer object

    glGenFramebuffersEXT(1, &_fb);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fb);
    glGenRenderbuffersEXT(1, &_color_rb);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _color_rb);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, w, h );
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
            GL_RENDERBUFFER_EXT, _color_rb);

    glGenRenderbuffersEXT(1, &_depth_rb);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depth_rb);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h );
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
            GL_RENDERBUFFER_EXT, _depth_rb);

    GLenum gle=glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    COUT_ERROR(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
    COUT_ERROR(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT);
    COUT_ERROR(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
    COUT_ERROR(GL_FRAMEBUFFER_UNSUPPORTED_EXT);
    COUT_ERROR(GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT);
    COUT_ERROR(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT);
    COUT_ERROR(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT);
    COUT_ERROR(GL_FRAMEBUFFER_UNDEFINED);
    COUT_ERROR(GL_FRAMEBUFFER_UNSUPPORTED);
    COUT_ERROR(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);

    if( gle!=GL_FRAMEBUFFER_COMPLETE ) {
      cout << "aborting:" << gle << endl;
      throw 1;
    }
#endif //OSMESA
  }

#ifdef OSMESA
#else
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _fb);
#endif //OSMESA

  _tbwm->invalidate();
  _tbwm->draw();

  glReadBuffer(GL_BACK);
  if( _tbwm->transBgd() )
    glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid *)_buffer);
  else
    glReadPixels(0,0,w,h,GL_RGB,GL_UNSIGNED_BYTE,(GLvoid *)_buffer);
  int align;
  glGetIntegerv(GL_PACK_ALIGNMENT, &align);
  pngimg->write( _buffer, align );
  delete pngimg;

#ifdef OSMESA
#else
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // restore the display
#endif //OSMESA

}


#include "AuxGrid.h"
#include <iostream>
#include <fstream>
#include <vector>

char *get_line(gzFile);

/** reads in an integer from a file throws exception on error
 *
 * \param in file pointer
 * \return integer read from file
 *
 * \throw exception if int is not read from file
 */
int get_num( gzFile in )
{
  int number;
  //if( !sscanf( get_line(in), "%d", &number ) )
  //  throw 1;

  char *p = get_line(in);
  if (p)
    number = atoi(p);
  else
    throw;

  return number;
}

/** private internal helper function that indexes frames to file positions */
class AuxGridIndexer
{
public:

  /** constructor
   *
   * Indexes the associated file at construction
   *
   * \param filename to index (without extention)
   * \param pt_offset in model to apply
   */
  AuxGridIndexer(char * filename, GLfloat const * pt_offset)
    : _pt_offset(pt_offset),
      _model(0),
      _data(0),
      _frame(-1)
  {
    char *ext = strstr( filename, ".pts_t" );
    if( ext ) 
      *ext = '\0';

    _pts_in = index(_vec_pts_pos, filename, ".pts_t");
    _elem_in = index(_vec_elem_pos, filename, ".elem_t", 
                                              (int) _vec_pts_pos.size());
    _dat_in = index(_vec_dat_pos, filename, ".dat_t",
                                              (int) _vec_pts_pos.size());

    if (!_pts_in) {
      std::cerr << "Failed to open .pts_t file" << std::endl;
      throw 2;
    }
  }

  /** destructor
   *
   * Free's memory allocated internally by the object and closes all open file
   * pointers.
   */
  ~AuxGridIndexer()
  {
    if (_model) {
      delete _model;
      _model = 0;
    }

    if (_data) {
      delete _data;
      _data = 0;
    }

    gzclose(_pts_in);
    gzclose(_elem_in);
    gzclose(_dat_in);
  }
  
  /** load model from frame in timepoints
   *
   * \param frame to load
   * \return pointer to model
   */
  Model *GetModel(int frame)
  { 
    // check if we have the frame loaded, if so just return the current model
    if (frame == _frame) {
      return _model;
    }
 
    // ensure the frame requested is valid
    if (frame >= _vec_pts_pos.size()) {
      std::cerr << "GetModel() request to invalid frame number" << std::endl;
      throw 3;
    }

    // delete previous model
    if (_model) {
      // @todo (gd) : figure out why this is segfaulting, MEMORY LEAK!!!
      // well turns out Models aren't really good in their destructors, and
      // parent classes are missing virtual keywords in their destructors and
      // I think generally it's kindof a mess, fix that then, uncomment the
      // following line.
      delete _model;
      _model = 0;
    }
      
    // delete previous data
    if (_data) {
      delete _data;
      _data = 0;
    }
    
    // set position in all files to frame
    seek(frame);

    // load model from file
    _model = new Model();
    _model->read_instance(_pts_in, _elem_in, _pt_offset);
    
    // load data
    if (_dat_in) {
      int count = get_num(_dat_in);
      _data = new DATA_TYPE[count];
      for (int i=0; i<count; i++) {
        char *p = get_line(_dat_in );
        if(p) _data[i] = strtod( p, NULL);
      }
    }
  
    // update internal frame number
    _frame = frame;
  
    return _model;
  }

  /** get the number of time points indexed
   *
   * \return the number of available timepoints
   */
  int num_tm()
  {
    return (int) _vec_pts_pos.size();
  }

  /** return true if all time instances have the same number of points
   */
  bool plottable()
  {
    if( !_dat_in ) return false;

    if(_vec_pts_pos.size() == 1 ) return false;

    seek(0);
    int numpts = get_num(_pts_in);
    for( int i=1; i<_vec_pts_pos.size(); i++ ) {
      seek(i);
      if( get_num(_pts_in) != numpts ) {
        seek(0);
        return false;
      }
    }
    seek(0);
    return true;
  }

  /** return a time series for a point
   *
   * \param v the vertex number
   * \param d the series (will be allocated)
   * \return the size of the vector
   */
  int time_series( int v, double *&d )
  {
    if(!_dat_in) {
      d = NULL;
      return 0;
    }
    z_off_t place = gztell(_dat_in);
    d = new double[_vec_dat_pos.size()];
    for( int i=0; i<_vec_dat_pos.size(); i++ ) {
      gzseek(_dat_in, _vec_dat_pos[i], SEEK_SET);
      // add 1 to the loop to read the number of vertices in the frame
      for( int j=0; j<v+1; j++ )
        get_line(_dat_in);
      d[i] = strtod( get_line(_dat_in), NULL );
    }
    gzseek( _dat_in, place, SEEK_SET );
    return _vec_dat_pos.size();
  }
  

private:  

  /** index a files frame positions and store in vector
   *
   * This function will clear the vector, get the number of frames stored in the file
   * and store the position in the file associated with the start of each frame for
   * later use. If the expected frame count is required and provided, then we will
   * throw an exception if a different number of frames is encountered.
   * 
   * \param vecpos       vector of file positions to store when indexing
   * \param filename     without extention
   * \param extention    filename to attempt to index
   * \param match_frames number of frames we expect the file to have 
   *                     (-1 if not required to match)
   *
   * \return open file handle or 0 if file could not be opened
   */
  gzFile index(std::vector<z_off_t> & vecpos, char const * filename, 
                                         char const * extention, int match_frames = -1)
  {
    try {
      // clear vector of positions (indexes)
      vecpos.clear();

      // open input file to index
      gzFile infile = openFile(filename, extention);

      // read number of frames in file
      int frames = get_num(infile);

      // validate the number of frames
      if ((match_frames != -1) && (match_frames != frames )) {
        std::cerr << "incorrect number of timepoints in " << extention << 
            " file" << std::endl;
        throw 1;
      }

      // iterate all frames in file
      for (int frame = 0; frame < frames; frame++) {
        vecpos.push_back(gztell(infile));

        // skip lines of frame
        if (frame >= 50)
          std::cerr << frame << std::endl;

        int lines = get_num(infile);
        for (int line = 0; line < lines; line++) {
          get_line(infile);
        }
      }

      // return open file handle
      return infile;
    }
    catch(...) {
      return 0;
    }
  }

  /** set all files to position in file relating to frame
   *
   * \param frame to set all file positions at
   */
  void seek(int frame)
  {
    gzseek(_pts_in, _vec_pts_pos[frame], SEEK_SET);

    if (_elem_in) {
      gzseek(_elem_in, _vec_elem_pos[frame], SEEK_SET);
    }

    if (_dat_in) {
      gzseek(_dat_in, _vec_dat_pos[frame], SEEK_SET);
    }
  }


  gzFile _pts_in;                       //!< points backing file
  gzFile _elem_in;                      //!< element backing file
  gzFile _dat_in;                       //!< data backing file
  std::vector<z_off_t> _vec_pts_pos;    //!< frame indexed vector of positions into points file
  std::vector<z_off_t> _vec_elem_pos;   //!< frame indexed vector of positions into element file
  std::vector<z_off_t> _vec_dat_pos;    //!< frame indexed vector of positions into data file
  int _frame;                           //!< current frame (-1 if no frame loaded)
public:
  Model * _model;                       //!< current frame model
  DATA_TYPE *_data;                     //!< current frame data
private:
  GLfloat const * _pt_offset;           //!< offset to pass in for model creation (shouldn't really be here)
};


/**************************************************************************/
/********************** AuxGrid functions below here **********************/
/**************************************************************************/

/** constructor 
 *
 * \param fn        base file name
 * \param pt_offset center of model coordinates
 *
 * \throw 1 if any error in input
 */
AuxGrid::AuxGrid( char *fn, const GLfloat* pt_offset )
  : _display(true), _hilight(false), _hiVert(0), _plottable(false),
    _indexer(new AuxGridIndexer(fn, pt_offset)),_timeplot(NULL)
{
  for (int i=0; i<sizeof(*_3D); i++)
    _3D[i] = true;

  for( int i=0; i<maxobject; i++ ) {
    _show[i]     = true;
    _datafied[i] = false;
    _color[i][3] = 1;
  }

  _plottable = _indexer->plottable();
  if( _plottable ){
    _timeplot = new PlotWin("Aux Time Series");
    _sz_ts = _indexer->time_series(0,_time_series);
    _timeplot->set_data( _sz_ts, _time_series, 0 );
  }

  threeD( Cnnx, true );
  threeD( Vertex, true );
  _indexer->GetModel(0);
}


/** draw the grid at a particular time 
 *
 * \param t the time
 */
void 
AuxGrid :: draw( int t )
{
  if( !_display || t<0 ) {
    return;
  }

  if( t >= num_tm() ) t = num_tm()-1;

  Model *m = _indexer->GetModel(t);
  m->pt.size(size(Vertex));
  m->pt.threeD(threeD(Vertex));
  m->_cnnx->threeD(threeD(Cnnx));
  m->_cnnx->size(size(Cnnx));

  if( _autocol ) {
    optimize_cs(t);
  }

  if( _show[Vertex] ) {
    m->pt.draw( 0, m->pt.num()-1, color(Vertex), &cs, 
     (_indexer->_data && _datafied[Vertex]) ? _indexer->_data : NULL, 1, NULL );
  }

  if( _hilight ) {
    GLfloat hicol[] = { 1, 0, 0, 1 };
    m->pt.draw( _hiVert, hicol, 2*size(Vertex) ); 
    if( _timeplot && _timeplot->window->shown() )
      _timeplot->highlight(t);
  }

  if( _show[Cnnx] ) {
    m->_cnnx->draw( 0, m->_cnnx->num()-1, color(Cnnx),&cs,
       (_indexer->_data && _datafied[Cnnx]) ? _indexer->_data : NULL, 1, NULL );
  }

  if( _show[SurfEle]  ) {
    glPushAttrib(GL_POLYGON_BIT);

    if( _surf_fill ) {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    } 
    else {
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      glLineWidth(m->size(SurfEle,0));
    }
    
    GLfloat r[]={0,1,1,1};
    m->surface(0)->draw( color(SurfEle), &cs, 
	          (_indexer->_data && _datafied[SurfEle]) ? _indexer->_data : NULL,
                                  1, NULL, m->vertex_normals(m->surface(0)) );
    glPopAttrib();
  }

  if( _show[VolEle] && m->_vol ) {
    glPushAttrib(GL_POLYGON_BIT);

    if( _vol_fill ) {
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    } 
    else {
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      glLineWidth(m->size(VolEle,0));
    }
    for( int i=0; i<m->number(VolEle); i++ ) {
      m->_vol[i]->draw( 0, m->_cnnx->num()-1, m->get_color(VolEle),
		&cs, (_indexer->_data && _datafied[VolEle]) ? _indexer->_data : NULL, 1, NULL );
    }
    
    glPopAttrib();
  } 
}

/** destructor */
AuxGrid::~AuxGrid()
{
  delete _indexer;
  _indexer = 0;
  if( _timeplot )
    delete _timeplot;
  _timeplot = NULL;
}

void AuxGrid :: optimize_cs( int tm )
{
  if(_indexer->_data==NULL ) return;
  DATA_TYPE min=_indexer->_data[0], max=_indexer->_data[0];
  for( int i=1; i<_indexer->_model->pt.num(); i++ ) {

    if( _indexer->_data[i]<min ) {
      min=_indexer->_data[i];
    }

    if( _indexer->_data[i]>max ) {
      max=_indexer->_data[i];
    }

  }
  cs.calibrate( min, max );
}


void AuxGrid::color(Object_t o, GLfloat * r)
{
  for (int i=0; i<4; i++) {
    _color[o][i] = r[i];
  }
}


int AuxGrid::num_tm()
{
  return _indexer->num_tm();
}


/** show the plot of the time series
 *
 *  \param tm time currently displayed
 */
void AuxGrid::plot(int tm)
{
  if( !_plottable )
    return;
  _sz_ts = _indexer->time_series( _hiVert, _time_series );
  _timeplot->window->show();
  _timeplot->set_data( _sz_ts, _time_series, tm );
  _timeplot->window->redraw();
} 


/** is data present on the grid
 */
bool AuxGrid::data()
{
  return _indexer->_data != NULL;
}


/** how many vertices at present
 */
int AuxGrid::num_vert()
{
  return _indexer->_model->pt.num();
}

/** specify a vertex to highlight
 *
 * \return true val is valid
 * \post \p val contains the node data value
 */
bool AuxGrid :: highlight_vertex( int n, float &val, bool update_plot )
{
  if( n >= _indexer->_model->pt.num() ) return false;

  _hiVert = n;
  if( _indexer->_data ) {
    val = _indexer->_data[n];
    if( update_plot && _plottable && _timeplot->window->shown() ) {
      _sz_ts = _indexer->time_series( _hiVert, _time_series );
      _timeplot->set_data( _sz_ts, _time_series, 0 );
    }
    return true;
  }
  return false;
}

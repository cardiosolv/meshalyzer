#include "AuxGrid.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

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

class AuxGridFetcher
{
public:
  AuxGridFetcher():_model(NULL),_frame(-1),_data(NULL){}
  virtual Model *GetModel(int) =0;
  virtual int    num_tm() =0;
  virtual bool   plottable() =0;
  virtual int    time_series( int v, double *&d ) =0;
  Model *_model;
  float *_data;                     //!< current frame data
protected:
  int _frame;
};
  

class AuxGridHDF5 : public AuxGridFetcher {

public:
  AuxGridHDF5(const char * fn)
  {
#ifdef USE_HDF5
    string gtype;
    parse_HDF5_grid( fn, gtype, _indx );
    string af = fn;
    if( ch5_open( af.substr(0,af.find_last_of(":")).c_str(), &_hin ) )
      throw 1;

    ch5s_aux_grid info;
    ch5s_aux_grid_info( _hin, _indx, &info );
    _max_frame = info.time_steps;
    assert(0);
#endif
  }
  ~AuxGridHDF5()
  {
#ifdef USE_HDF5
    ch5_close( _hin );
#endif
  }
  /** load model from frame in timepoints
   *
   * \param frame to load
   * \return pointer to model
   */
  Model *GetModel(int frame)
  { 
#ifdef USE_HDF5
    // check if we have the frame loaded, if so just return the current model
    if (frame == _frame) {
      return _model;
    }
 
    // ensure the frame requested is valid
    if ( frame >= _max_frame ) {
      std::cerr << "GetModel() request to invalid frame number" << std::endl;
      throw 3;
    }

    // delete previous model
    if (_model) {
      delete _model;
      _model = NULL;
    }
      
    // delete previous data
    if (_data) {
      delete _data;
      _data = NULL;
    }
    
    _model = new Model();
    _model->read_instance(_hin, _indx, frame, _data);
    
    // update internal frame number
    _frame = frame;
  
    return _model;
#else
    assert(0);
#endif
  }
  int  num_tm(){return _max_frame; }
  bool plottable(){ return false; }
  int  time_series( int v, double *&d ){d=NULL; return 0;}
private:
#ifdef USE_HDF5
  hid_t        _hin;
#endif
  unsigned int _indx;     //!< grid index
  int          _max_frame;
};


/** private internal helper function that indexes frames to file positions */
class AuxGridIndexer : public AuxGridFetcher
{
public:

  /** constructor
   *
   * Indexes the associated file at construction
   *
   * \param filename to index (without extention)
   */
  AuxGridIndexer(const char * filename)
  {
    string base = filename;

    if (strstr(filename, ".pts_t") != NULL)
      base.erase(base.size()-6,6); // remove extension

    _pts_in  = index(_vec_pts_pos,  base.c_str(), ".pts_t");
    _elem_in = index(_vec_elem_pos, base.c_str(), ".elem_t",
                                       _vec_pts_pos.size() );
    _max_tm = max( _vec_pts_pos.size(), _vec_elem_pos.size() );
    _dat_in  = index(_vec_dat_pos,  base.c_str(), ".dat_t", _max_tm );

    _max_tm = max( _vec_dat_pos.size(), _max_tm )-1;

    if (!_pts_in) {
      std::cerr << "Failed to open .pts_t file" << std::endl;
      throw 2;
    }
    base.clear();
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
    if (frame > _max_tm) {
      std::cerr << "GetModel() request to invalid frame number" << std::endl;
      throw 3;
    }

    // delete previous model
    if (_model) {
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
    _model->read_instance(_pts_in, _elem_in);
    
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
    return _max_tm+1;
  }

  /** return true if all time instances have the same number of points
   */
  bool plottable()
  {
    if( !_dat_in ) return false;

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
   *                     (-1=if not required to match)
   *
   * \note 1 frame will always match any number of frames supplied
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
      if((match_frames!=-1) && (match_frames!=frames && frames!=1 && match_frames!=1)) {
        std::cerr << "incorrect number of timepoints in " << extention << 
            " file" << std::endl;
        throw 1;
      }

      // iterate all frames in file
      for (int frame = 0; frame < frames; frame++) {
        vecpos.push_back(gztell(infile));

        int lines = get_num(infile);

        // skip lines of frame
        if (frame >= 50)
          std::cerr << frame << " --- "  << lines << std::endl;

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
    if( _vec_pts_pos.size() > 1 )
      gzseek(_pts_in, _vec_pts_pos[frame], SEEK_SET);
    else
      gzseek(_pts_in, _vec_pts_pos[0], SEEK_SET);

    if (_elem_in) {
      if( _vec_elem_pos.size() > 1 )
        gzseek(_elem_in, _vec_elem_pos[frame], SEEK_SET);
      else
        gzseek(_elem_in, _vec_elem_pos[0], SEEK_SET);
    }

    if (_dat_in) {
      if( _vec_dat_pos.size() > 1 )
        gzseek(_dat_in, _vec_dat_pos[frame], SEEK_SET);
      else
        gzseek(_dat_in, _vec_dat_pos[0], SEEK_SET);
    }
  }

  long unsigned int _max_tm;            //!< maximum time
  gzFile _pts_in;                       //!< points backing file
  gzFile _elem_in;                      //!< element backing file
  gzFile _dat_in;                       //!< data backing file
  std::vector<z_off_t> _vec_pts_pos;    //!< frame indexed vector of positions into points file
  std::vector<z_off_t> _vec_elem_pos;   //!< frame indexed vector of positions into element file
  std::vector<z_off_t> _vec_dat_pos;    //!< frame indexed vector of positions into data file
};


/**************************************************************************/
/********************** AuxGrid functions below here **********************/
/**************************************************************************/

/** constructor 
 *
 * \param fn        base file name
 * \param ag        auxilliary grid from which settings are copied
 *
 * \throw 1 if any error in input
 */
AuxGrid::AuxGrid( const char *fn, AuxGrid *ag )
  : _display(true), _hilight(false), _hiVert(0), _plottable(false),
    _indexer(NULL),_timeplot(NULL),_clip(false)
{
  if( strstr( fn, ".pts_t" ) )
    _indexer = new AuxGridIndexer(fn);
  else if( strstr( fn, ".datH5:" ) )
    _indexer = new AuxGridHDF5(fn);
  else
    throw 1;

  if( ag ){
    _hilight = ag->_hilight;
    _hiVert  = ag->_hiVert;
    _clip    = ag->_clip;
    _display = ag->_display;
  }

  for( int i=0; i<maxobject; i++ ) {
    if( ag ) {
      _show[i]     = ag->_show[i]; 
      _datafied[i] = ag->_datafied[i]; 
      _size[i]     = ag->_size[i];
      _3D[i]       = ag->_3D[i];
      for( int j=0; j<4; j++ )
        _color[i][j] = ag->_color[i][j];
    } else {
      _show[i]     = true;
      _datafied[i] = false;
      _color[i][3] = 1;
      _3D[i]       = true;
    }
  }
  if( !ag ) {
      size(Vertex, 50);
      size(Cnnx, 100);
      size(SurfEle, 50);
      size(VolEle, 50);
  }

  _plottable = _indexer->plottable();

  if( _plottable ){
    _timeplot = new PlotWin("Aux Time Series");
    _sz_ts = _indexer->time_series(0,_time_series);
    _timeplot->set_data( _sz_ts, 0, _time_series, 0 );
  }

  if( _indexer )
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

  Model *m;
  m = _indexer->GetModel(t);

  m->pt.size(size(Vertex));
  m->pt.threeD(threeD(Vertex));
  m->_cnnx->threeD(threeD(Cnnx));
  m->_cnnx->size(size(Cnnx));

  if( _autocol ) {
    optimize_cs(t);
  }

  GLboolean cpv[6];
  if( !_clip )
    for( int i=0; i<6; i++ ) {
      glGetBooleanv( GL_CLIP_PLANE0+i, cpv+i );
      glDisable( GL_CLIP_PLANE0+i );
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

  if( !_clip ) 
    for( int i=0; i<6; i++ ) 
      if( cpv[i] )
        glEnable( GL_CLIP_PLANE0+i );
}

/** destructor */
AuxGrid::~AuxGrid()
{
  delete _indexer;
  _indexer = NULL;
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
  _timeplot->set_data( _sz_ts, _hiVert, _time_series, tm );
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
      _timeplot->set_data( _sz_ts, _hiVert, _time_series, 0 );
    }
    return true;
  }
  return false;
}


#include "AuxGrid.h"

/* constructor 
 *
 * \param fn base file name
 */
Auxgrid::AuxGrid:_num_tm_grid(1),_num_tm_data(0)( char *fn )
{
  gzFile ptin;
  try {
    ptin = openFile( fn, ".pts_t" );
  }catch(...) {throw 1;}

  _num_tm_grid = get_num( ptin );

  try { 
    elem_in = openFile( fn, ".elem_t" );
  } catch(...) {elem_in=NULL;}

  try { 
    data_in = openFile( fn, ".data_t" );
  } catch(...) {data_in=NULL;}

  //sanity checks
  if( elem_in && get_num( elem_in ) != _num_tm_grid ) {
    cerr << "points and element file times do not match" << endl;
    throw 1;
  }
  if( data_in ) {
    _num_tm_data = get_num( data_in );
    if( _num_tm_grid>1 && _num_tm_data>1 && _num_tm_grid!=_num_tm_data ) {
      cerr << "points and data file times do not match" << endl;
      throw 1;
    }
  }
  
  _num_mod = _num_tm_grid>_num_tm_data?_num_tm_grid:_num_tm_data;

  for( int i=0; i<_num_mod; i++ ) {
    _model.push_back(new Model() );
    read_pts_instance( pts_in );
    read_elem_instance( elem_in );
    read_data_instance( data_in );
  }
}


/** draw the grid at a particular time 
 *
 * \param t the time
 */
void 
AuxGrid :: draw( int t )
{
}


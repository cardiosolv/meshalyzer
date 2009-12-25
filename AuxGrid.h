/** This class implements an auxilliary grid which can be used to present 
 *  time varying (or fixed) data. The auxilliary grid itself can be totally dynamic,
 *  meaning that points and the elements derived from them can change. If both the
 *  grid and data are dynamic, there must be the same number of time instances. 
 *
 */
#ifndef AUXGRID_H
#define AUXGRID_H

#include "Model.h"
#include <vector>
#include "DrawingObjects.h"
#include "VecData.h"

class AuxGrid {
    private: 
        vector<Model *> _model;         //!< one model per time
        int         _num_tm_grid;
        int         _num_tm_data;
        int         _num_mod;
        DATA_TYPE **_data;
        void         read_data_instance( gzFile, int, DATA_TYPE*& );
        bool         _display;
        bool         _datafied[maxobject];
        bool         _surf_fill;  //!< if false, draw outline
        bool         _vol_fill;   //!< if false, draw wireframe
        bool         _autocol;
        bool         _show[maxobject]; //!< true to show object
    public:
        AuxGrid( char *fn, const GLfloat *ptoff=NULL );
        ~AuxGrid();
		Colourscale cs;
        void     draw( int );
        void     color( Object_t obj, GLfloat *r );
        GLfloat* color( Object_t o ){return _model[0]->get_color(o,-1);}
        void     showobj( Object_t obj, bool b ){_show[obj]=b;}
        bool     showobj( Object_t o ){return _show[o];}
        void     threeD( Object_t o, bool b );
        bool     threeD( Object_t o ){return _model[0]->threeD(o,-1);}
        void     size( Object_t o, float s );
        float    size( Object_t o ){return _model[0]->size(o,-1);}
        int      num_tm(){ return _num_mod; }
        void     display( bool b ){ _display=b; }
        void     datify( Object_t o, bool b ){ _datafied[o]=b; }
        void     surfill( bool b ){ _surf_fill=b; }
        void     volfill( bool b ){ _vol_fill=b; }
        void     autocolor( bool b ){ _autocol=b; }
        void     optimize_cs(int);
};

#endif

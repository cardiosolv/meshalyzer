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

class AuxGrid {
    private: 
        vector<Model *> _model;         //!< one model per time
        int    _num_tm_grid;
        int    _num_tm_data;
        bool   _threeD[maxobject];
        float  _size[maxobject];
        int    _num_mod;
    public:
        AuxGrid( char *fn );
        ~AuxGrid();
        void draw( int );
        void     color( Object_t obj, int s, float r, float g, float b, float a );
        GLfloat* color( Object_t obj, int s );
        void     showobj( Object_t obj, bool f );
        bool     showobj( Object_t o );
        void     threeD( Object_t o, bool b ){ _threeD[o] = b; }
        bool     threeD( Object_t o ){ return _threeD[o]; }
        void     size( Object_t o, float s ){ _size[o] = s; }
        float    size( Object_t o ){ return _size[o]; }
        int      num_tm( return model.size(); );
};

#endif

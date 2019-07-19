/** This class implements an auxilliary grid which can be used to present 
 *  time varying (or fixed) data. The auxilliary grid itself can be totally dynamic,
 *  meaning that points and the elements derived from them can change. If both the
 *  grid and data are dynamic, there must be the same number of time instances. 
 *
 */
#ifndef AUXGRID_H
#define AUXGRID_H

#include <vector>
#include "Model.h"
#include "DrawingObjects.h"
#include "plottingwin.h"

class AuxGridFetcher;

class AuxGrid {
    private:
        AuxGridFetcher * _indexer = NULL;  //!< private implementation of indexer
        bool         _display   = true;
        bool         _datafied[maxobject];
        bool         _surf_fill = true;  //!< if false, draw outline
        bool         _vol_fill  = false;   //!< if false, draw wireframe
        bool         _autocol=false;
        bool         _show[maxobject]; //!< true to show object
        bool         _3D[maxobject+2];
        GLfloat      _color[maxobject+2][4];
        float        _size[maxobject+2];
        bool         _plottable = false;    //!< true if a time series can be plotted
        int          _hiVert    = 0;
        bool         _hilight   = false;
        PlotWin*     _timeplot  = NULL;
        int          _sz_ts;               //!< size of time series
        double*      _time_series;
        bool         _clip      = false;   // is is clipped?

    public:
        AuxGrid( const char *fn, AuxGrid *ag=NULL );
        virtual ~AuxGrid();

        Colourscale cs;
        void     draw( int );
        void     color( Object_t o, GLfloat *r );
        GLfloat * color( Object_t o ){ return _color[o]; }
        void     showobj( Object_t o, bool b ) { _show[o]=b;}
        bool     showobj( Object_t o ) { return _show[o];}
        void     threeD( Object_t o, bool b ) { _3D[o] = b; }
        bool     threeD( Object_t o ) const { return _3D[o]; }
        void     size( Object_t o, float s ) { _size[o] = s; }
        float    size( Object_t o ) const {return _size[o]; }
        int      num_tm();
        void     display( bool b ){ _display=b; }
        void     datify( Object_t o, bool b ){ _datafied[o]=b; }
        void     surfill( bool b ){ _surf_fill=b; }
        void     volfill( bool b ){ _vol_fill=b; }
        void     autocolor( bool b ){ _autocol=b; }
        void     optimize_cs(int);
        bool     highlight_vertex(int n, float &val, bool update_plot=true);
        void     highlight(bool b){_hilight=b;}
        void     plot(int);
        bool     plottable(){return _plottable;}
        bool     data();
        int      num_vert();
        void     clip( bool b ){ _clip=b; }
};

#endif

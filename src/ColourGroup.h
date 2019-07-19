#ifndef COLOUR_GROUP
#define COLOUR_GROUP

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Item.H>


class ColourGroup : public Fl_Group {
    public:
      ColourGroup(int x, int y, int w, int h, const char *label = 0) : Fl_Group(x, y, w, h, label){}
      virtual int handle( int e );
    private :
      int i=0;
};
#endif

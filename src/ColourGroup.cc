#include "ColourGroup.h"
#include <iostream>
#include "trimesh.h"

extern Controls *ctrl_ptr;

/** reverse the maximum and minimum values */
void 
col_reverse_cb( Fl_Widget *w, void *p )
{ 
  float min=ctrl_ptr->mincolval->value();
  ctrl_ptr->mincolval->value(ctrl_ptr->maxcolval->value());
  ctrl_ptr->maxcolval->value(min);
  ctrl_ptr->mincolval->do_callback();
  ctrl_ptr->maxcolval->do_callback();
}

    
/** make the color scale symmeetric about wero using the minimum colour value */
void
colour_sym_min_cb( Fl_Widget *w, void *p )
{
  ctrl_ptr->maxcolval->value(-ctrl_ptr->mincolval->value());
  ctrl_ptr->maxcolval->do_callback();
}


/** make the color scale symmeetric about zero using the maximum colour value */
void
colour_sym_max_cb( Fl_Widget *w, void *p )
{
  ctrl_ptr->mincolval->value(-ctrl_ptr->maxcolval->value());
  ctrl_ptr->mincolval->do_callback();
}


Fl_Menu_Item colour_pop_menu[] = {
  { "&reverse",       'r', (Fl_Callback *)col_reverse_cb,    0, 0, 0, 0, 14, 56 },
  { "&symmetric min", 'm', (Fl_Callback *)colour_sym_min_cb, 0, 0, 0, 0, 14, 56 },
  { "&symmetric max", 'n', (Fl_Callback *)colour_sym_max_cb, 0, 0, 0, 0, 14, 56  },
  {0}
};


int
ColourGroup::handle( int e ) { 
  if(e==FL_PUSH && Fl::event_button()==3){
    const Fl_Menu_Item* m = colour_pop_menu->popup(Fl::event_x(), Fl::event_y(),NULL,0,0);
    if (m) {
      m->do_callback(this, (void*)m);
    }
  }else 
    Fl_Group::handle(e);
  return 0;
}


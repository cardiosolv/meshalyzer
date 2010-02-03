// generated by Fast Light User Interface Designer (fluid) version 1.0300

#include "ClipPlane.h"
#define SAVE_CP(A) xdir##A->value()<<" "<<ydir##A->value()<<" "<<zdir##A->value()<<" "<<intercept##A->value()<<" "<<dispas##A->value()
#define READ_CP(A,S) {double val; S>>val; xdir##A->value(val); S>>val; ydir##A->value(val); S>>val; zdir##A->value(val);S>>val; intercept##A->value(val); S>>val; dispas##A->value(int(val));}
#define CHECK_CP(A) {int val=dispas##A->value();if(val==3||val==4)tbmw->determine_cutplane(A);}

void ClipPlane::cb_xdir0_i(Fl_Value_Input*, void*) {
  normcb(0);
}
void ClipPlane::cb_xdir0(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_xdir0_i(o,v);
}

void ClipPlane::cb_ydir0_i(Fl_Value_Input*, void*) {
  normcb(0);
}
void ClipPlane::cb_ydir0(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_ydir0_i(o,v);
}

void ClipPlane::cb_zdir0_i(Fl_Value_Input*, void*) {
  normcb(0);
}
void ClipPlane::cb_zdir0(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_zdir0_i(o,v);
}

void ClipPlane::cb_unit0_i(Fl_Button*, void*) {
  unitize(xdir0,ydir0,zdir0);
normcb(0);
}
void ClipPlane::cb_unit0(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_unit0_i(o,v);
}

void ClipPlane::cb_Flip_i(Fl_Button*, void*) {
  flip(0);
}
void ClipPlane::cb_Flip(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_Flip_i(o,v);
}

void ClipPlane::cb_intercept0_i(Fl_Value_Slider*, void*) {
  normcb(0);
}
void ClipPlane::cb_intercept0(Fl_Value_Slider* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_intercept0_i(o,v);
}

void ClipPlane::cb_Off_i(Fl_Menu_*, void*) {
  drawIntercept( 0, false );
tbmw->redraw();
}
void ClipPlane::cb_Off(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Off_i(o,v);
}

void ClipPlane::cb_On_i(Fl_Menu_*, void*) {
  drawIntercept( 0, false );
tbmw->redraw();
}
void ClipPlane::cb_On(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_On_i(o,v);
}

void ClipPlane::cb_Whole_i(Fl_Menu_*, void*) {
  drawIntercept( 0, false );
tbmw->redraw();
}
void ClipPlane::cb_Whole(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Whole_i(o,v);
}

void ClipPlane::cb_Intersection_i(Fl_Menu_*, void*) {
  drawIntercept( 0, true );
tbmw->determine_cutplane(0);
tbmw->redraw();
}
void ClipPlane::cb_Intersection(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Intersection_i(o,v);
}

void ClipPlane::cb_Datified_i(Fl_Menu_*, void*) {
  drawIntercept( 0, true );
tbmw->determine_cutplane(0);
tbmw->redraw();
}
void ClipPlane::cb_Datified(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Datified_i(o,v);
}

Fl_Menu_Item ClipPlane::menu_dispas0[] = {
 {"Off", 0,  (Fl_Callback*)ClipPlane::cb_Off, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"On", 0,  (Fl_Callback*)ClipPlane::cb_On, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Whole Plane", 0,  (Fl_Callback*)ClipPlane::cb_Whole, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Intersection", 0,  (Fl_Callback*)ClipPlane::cb_Intersection, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Datified", 0,  (Fl_Callback*)ClipPlane::cb_Datified, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

void ClipPlane::cb_xdir1_i(Fl_Value_Input*, void*) {
  normcb(1);
}
void ClipPlane::cb_xdir1(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_xdir1_i(o,v);
}

void ClipPlane::cb_ydir1_i(Fl_Value_Input*, void*) {
  normcb(1);
}
void ClipPlane::cb_ydir1(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_ydir1_i(o,v);
}

void ClipPlane::cb_zdir1_i(Fl_Value_Input*, void*) {
  normcb(1);
}
void ClipPlane::cb_zdir1(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_zdir1_i(o,v);
}

void ClipPlane::cb_unit1_i(Fl_Button*, void*) {
  unitize(xdir1,ydir1,zdir1);
normcb(1);
}
void ClipPlane::cb_unit1(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_unit1_i(o,v);
}

void ClipPlane::cb_Flip1_i(Fl_Button*, void*) {
  flip(1);
}
void ClipPlane::cb_Flip1(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_Flip1_i(o,v);
}

void ClipPlane::cb_intercept1_i(Fl_Value_Slider*, void*) {
  normcb(1);
}
void ClipPlane::cb_intercept1(Fl_Value_Slider* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_intercept1_i(o,v);
}

void ClipPlane::cb_Off1_i(Fl_Menu_*, void*) {
  drawIntercept( 1, false );
tbmw->redraw();
}
void ClipPlane::cb_Off1(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Off1_i(o,v);
}

void ClipPlane::cb_On1_i(Fl_Menu_*, void*) {
  drawIntercept( 1, false );
tbmw->redraw();
}
void ClipPlane::cb_On1(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_On1_i(o,v);
}

void ClipPlane::cb_Whole1_i(Fl_Menu_*, void*) {
  drawIntercept( 1, false );
tbmw->redraw();
}
void ClipPlane::cb_Whole1(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Whole1_i(o,v);
}

void ClipPlane::cb_Intersection1_i(Fl_Menu_*, void*) {
  drawIntercept( 1, true );
tbmw->determine_cutplane(1);
tbmw->redraw();
}
void ClipPlane::cb_Intersection1(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Intersection1_i(o,v);
}

void ClipPlane::cb_Datified1_i(Fl_Menu_*, void*) {
  drawIntercept( 1, true );
tbmw->determine_cutplane(1);
tbmw->redraw();
}
void ClipPlane::cb_Datified1(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Datified1_i(o,v);
}

Fl_Menu_Item ClipPlane::menu_dispas1[] = {
 {"Off", 0,  (Fl_Callback*)ClipPlane::cb_Off1, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"On", 0,  (Fl_Callback*)ClipPlane::cb_On1, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Whole Plane", 0,  (Fl_Callback*)ClipPlane::cb_Whole1, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Intersection", 0,  (Fl_Callback*)ClipPlane::cb_Intersection1, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Datified", 0,  (Fl_Callback*)ClipPlane::cb_Datified1, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

void ClipPlane::cb_xdir2_i(Fl_Value_Input*, void*) {
  normcb(2);
}
void ClipPlane::cb_xdir2(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_xdir2_i(o,v);
}

void ClipPlane::cb_ydir2_i(Fl_Value_Input*, void*) {
  normcb(2);
}
void ClipPlane::cb_ydir2(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_ydir2_i(o,v);
}

void ClipPlane::cb_zdir2_i(Fl_Value_Input*, void*) {
  normcb(2);
}
void ClipPlane::cb_zdir2(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_zdir2_i(o,v);
}

void ClipPlane::cb_unit2_i(Fl_Button*, void*) {
  unitize(xdir2,ydir2,zdir2);
normcb(2);
}
void ClipPlane::cb_unit2(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_unit2_i(o,v);
}

void ClipPlane::cb_Flip2_i(Fl_Button*, void*) {
  flip(2);
}
void ClipPlane::cb_Flip2(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_Flip2_i(o,v);
}

void ClipPlane::cb_intercept2_i(Fl_Value_Slider*, void*) {
  normcb(2);
}
void ClipPlane::cb_intercept2(Fl_Value_Slider* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_intercept2_i(o,v);
}

void ClipPlane::cb_Off2_i(Fl_Menu_*, void*) {
  drawIntercept( 2, false );
tbmw->redraw();
}
void ClipPlane::cb_Off2(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Off2_i(o,v);
}

void ClipPlane::cb_On2_i(Fl_Menu_*, void*) {
  drawIntercept( 2, false );
tbmw->redraw();
}
void ClipPlane::cb_On2(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_On2_i(o,v);
}

void ClipPlane::cb_Whole2_i(Fl_Menu_*, void*) {
  drawIntercept( 2, false );
tbmw->redraw();
}
void ClipPlane::cb_Whole2(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Whole2_i(o,v);
}

void ClipPlane::cb_Intersection2_i(Fl_Menu_*, void*) {
  drawIntercept( 2, true );
tbmw->determine_cutplane(2);
tbmw->redraw();
}
void ClipPlane::cb_Intersection2(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Intersection2_i(o,v);
}

void ClipPlane::cb_Datified2_i(Fl_Menu_*, void*) {
  drawIntercept( 2, true );
tbmw->determine_cutplane(2);
tbmw->redraw();
}
void ClipPlane::cb_Datified2(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Datified2_i(o,v);
}

Fl_Menu_Item ClipPlane::menu_dispas2[] = {
 {"Off", 0,  (Fl_Callback*)ClipPlane::cb_Off2, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"On", 0,  (Fl_Callback*)ClipPlane::cb_On2, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Whole Plane", 0,  (Fl_Callback*)ClipPlane::cb_Whole2, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Intersection", 0,  (Fl_Callback*)ClipPlane::cb_Intersection2, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Datified", 0,  (Fl_Callback*)ClipPlane::cb_Datified2, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

void ClipPlane::cb_xdir3_i(Fl_Value_Input*, void*) {
  normcb(3);
}
void ClipPlane::cb_xdir3(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_xdir3_i(o,v);
}

void ClipPlane::cb_ydir3_i(Fl_Value_Input*, void*) {
  normcb(3);
}
void ClipPlane::cb_ydir3(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_ydir3_i(o,v);
}

void ClipPlane::cb_zdir3_i(Fl_Value_Input*, void*) {
  normcb(3);
}
void ClipPlane::cb_zdir3(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_zdir3_i(o,v);
}

void ClipPlane::cb_unit3_i(Fl_Button*, void*) {
  unitize(xdir3,ydir3,zdir3);
normcb(3);
}
void ClipPlane::cb_unit3(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_unit3_i(o,v);
}

void ClipPlane::cb_Flip3_i(Fl_Button*, void*) {
  flip(3);
}
void ClipPlane::cb_Flip3(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_Flip3_i(o,v);
}

void ClipPlane::cb_intercept3_i(Fl_Value_Slider*, void*) {
  normcb(3);
}
void ClipPlane::cb_intercept3(Fl_Value_Slider* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_intercept3_i(o,v);
}

void ClipPlane::cb_Off3_i(Fl_Menu_*, void*) {
  drawIntercept( 3, false );
tbmw->redraw();
}
void ClipPlane::cb_Off3(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Off3_i(o,v);
}

void ClipPlane::cb_On3_i(Fl_Menu_*, void*) {
  drawIntercept( 3, false );
tbmw->redraw();
}
void ClipPlane::cb_On3(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_On3_i(o,v);
}

void ClipPlane::cb_Whole3_i(Fl_Menu_*, void*) {
  drawIntercept( 3, false );
tbmw->redraw();
}
void ClipPlane::cb_Whole3(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Whole3_i(o,v);
}

void ClipPlane::cb_Intersection3_i(Fl_Menu_*, void*) {
  drawIntercept( 3, true );
tbmw->determine_cutplane(3);
tbmw->redraw();
}
void ClipPlane::cb_Intersection3(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Intersection3_i(o,v);
}

void ClipPlane::cb_Datified3_i(Fl_Menu_*, void*) {
  drawIntercept( 3, true );
tbmw->determine_cutplane(3);
tbmw->redraw();
}
void ClipPlane::cb_Datified3(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Datified3_i(o,v);
}

Fl_Menu_Item ClipPlane::menu_dispas3[] = {
 {"Off", 0,  (Fl_Callback*)ClipPlane::cb_Off3, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"On", 0,  (Fl_Callback*)ClipPlane::cb_On3, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Whole Plane", 0,  (Fl_Callback*)ClipPlane::cb_Whole3, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Intersection", 0,  (Fl_Callback*)ClipPlane::cb_Intersection3, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Datified", 0,  (Fl_Callback*)ClipPlane::cb_Datified3, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

void ClipPlane::cb_xdir4_i(Fl_Value_Input*, void*) {
  normcb(4);
}
void ClipPlane::cb_xdir4(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_xdir4_i(o,v);
}

void ClipPlane::cb_ydir4_i(Fl_Value_Input*, void*) {
  normcb(4);
}
void ClipPlane::cb_ydir4(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_ydir4_i(o,v);
}

void ClipPlane::cb_zdir4_i(Fl_Value_Input*, void*) {
  normcb(4);
}
void ClipPlane::cb_zdir4(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_zdir4_i(o,v);
}

void ClipPlane::cb_unit4_i(Fl_Button*, void*) {
  unitize(xdir4,ydir4,zdir4);
normcb(4);
}
void ClipPlane::cb_unit4(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_unit4_i(o,v);
}

void ClipPlane::cb_Flip4_i(Fl_Button*, void*) {
  flip(4);
}
void ClipPlane::cb_Flip4(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_Flip4_i(o,v);
}

void ClipPlane::cb_intercept4_i(Fl_Value_Slider*, void*) {
  normcb(4);
}
void ClipPlane::cb_intercept4(Fl_Value_Slider* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_intercept4_i(o,v);
}

void ClipPlane::cb_Off4_i(Fl_Menu_*, void*) {
  drawIntercept( 4, false );
tbmw->redraw();
}
void ClipPlane::cb_Off4(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Off4_i(o,v);
}

void ClipPlane::cb_On4_i(Fl_Menu_*, void*) {
  drawIntercept( 4, false );
tbmw->redraw();
}
void ClipPlane::cb_On4(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_On4_i(o,v);
}

void ClipPlane::cb_Whole4_i(Fl_Menu_*, void*) {
  drawIntercept( 4, false );
tbmw->redraw();
}
void ClipPlane::cb_Whole4(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Whole4_i(o,v);
}

void ClipPlane::cb_Intersection4_i(Fl_Menu_*, void*) {
  drawIntercept( 4, true );
tbmw->determine_cutplane(4);
tbmw->redraw();
}
void ClipPlane::cb_Intersection4(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Intersection4_i(o,v);
}

void ClipPlane::cb_Datified4_i(Fl_Menu_*, void*) {
  drawIntercept( 4, true );
tbmw->determine_cutplane(4);
tbmw->redraw();
}
void ClipPlane::cb_Datified4(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Datified4_i(o,v);
}

Fl_Menu_Item ClipPlane::menu_dispas4[] = {
 {"Off", 0,  (Fl_Callback*)ClipPlane::cb_Off4, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"On", 0,  (Fl_Callback*)ClipPlane::cb_On4, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Whole Plane", 0,  (Fl_Callback*)ClipPlane::cb_Whole4, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Intersection", 0,  (Fl_Callback*)ClipPlane::cb_Intersection4, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Datified", 0,  (Fl_Callback*)ClipPlane::cb_Datified4, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

void ClipPlane::cb_xdir5_i(Fl_Value_Input*, void*) {
  normcb(5);
}
void ClipPlane::cb_xdir5(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_xdir5_i(o,v);
}

void ClipPlane::cb_ydir5_i(Fl_Value_Input*, void*) {
  normcb(5);
}
void ClipPlane::cb_ydir5(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_ydir5_i(o,v);
}

void ClipPlane::cb_zdir5_i(Fl_Value_Input*, void*) {
  normcb(5);
}
void ClipPlane::cb_zdir5(Fl_Value_Input* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_zdir5_i(o,v);
}

void ClipPlane::cb_unit5_i(Fl_Button*, void*) {
  unitize(xdir5,ydir5,zdir5);
normcb(5);
}
void ClipPlane::cb_unit5(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_unit5_i(o,v);
}

void ClipPlane::cb_Flip5_i(Fl_Button*, void*) {
  flip(5);
}
void ClipPlane::cb_Flip5(Fl_Button* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->parent()->user_data()))->cb_Flip5_i(o,v);
}

void ClipPlane::cb_intercept5_i(Fl_Value_Slider*, void*) {
  normcb(5);
}
void ClipPlane::cb_intercept5(Fl_Value_Slider* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_intercept5_i(o,v);
}

void ClipPlane::cb_Off5_i(Fl_Menu_*, void*) {
  drawIntercept( 5, false );
tbmw->redraw();
}
void ClipPlane::cb_Off5(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Off5_i(o,v);
}

void ClipPlane::cb_On5_i(Fl_Menu_*, void*) {
  drawIntercept( 5, false );
tbmw->redraw();
}
void ClipPlane::cb_On5(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_On5_i(o,v);
}

void ClipPlane::cb_Whole5_i(Fl_Menu_*, void*) {
  drawIntercept( 5, false );
tbmw->redraw();
}
void ClipPlane::cb_Whole5(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Whole5_i(o,v);
}

void ClipPlane::cb_Intersection5_i(Fl_Menu_*, void*) {
  drawIntercept( 5, true );
tbmw->determine_cutplane(5);
tbmw->redraw();
}
void ClipPlane::cb_Intersection5(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Intersection5_i(o,v);
}

void ClipPlane::cb_Datified5_i(Fl_Menu_*, void*) {
  drawIntercept( 5, true );
tbmw->determine_cutplane(5);
tbmw->redraw();
}
void ClipPlane::cb_Datified5(Fl_Menu_* o, void* v) {
  ((ClipPlane*)(o->parent()->parent()->user_data()))->cb_Datified5_i(o,v);
}

Fl_Menu_Item ClipPlane::menu_dispas5[] = {
 {"Off", 0,  (Fl_Callback*)ClipPlane::cb_Off5, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"On", 0,  (Fl_Callback*)ClipPlane::cb_On5, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Whole Plane", 0,  (Fl_Callback*)ClipPlane::cb_Whole5, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Intersection", 0,  (Fl_Callback*)ClipPlane::cb_Intersection5, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {"Datified", 0,  (Fl_Callback*)ClipPlane::cb_Datified5, 0, 0, FL_NORMAL_LABEL, 0, 14, 0},
 {0,0,0,0,0,0,0,0,0}
};

ClipPlane::ClipPlane( TBmeshWin *tb ) {
  { window = new Fl_Double_Window(575, 495, "Clipping Plane");
    window->user_data((void*)(this));
    { Clip0 = new Fl_Group(5, 20, 185, 220, "Clipping Plane 1");
      Clip0->box(FL_UP_BOX);
      { Fl_Group* o = new Fl_Group(15, 75, 115, 145, "Plane Normal1");
        o->box(FL_UP_BOX);
        { xdir0 = new Fl_Value_Input(35, 85, 80, 25, "X:");
          xdir0->maximum(0);
          xdir0->step(0.01);
          xdir0->value(1);
          xdir0->callback((Fl_Callback*)cb_xdir0);
        } // Fl_Value_Input* xdir0
        { ydir0 = new Fl_Value_Input(35, 120, 80, 25, "Y:");
          ydir0->maximum(0);
          ydir0->step(0.01);
          ydir0->callback((Fl_Callback*)cb_ydir0);
        } // Fl_Value_Input* ydir0
        { zdir0 = new Fl_Value_Input(35, 155, 80, 25, "Z:");
          zdir0->maximum(0);
          zdir0->step(0.01);
          zdir0->callback((Fl_Callback*)cb_zdir0);
        } // Fl_Value_Input* zdir0
        { unit0 = new Fl_Button(20, 190, 55, 20, "Unitize");
          unit0->tooltip("set to unit normal");
          unit0->callback((Fl_Callback*)cb_unit0);
        } // Fl_Button* unit0
        { Fl_Button* o = new Fl_Button(80, 190, 40, 20, "Flip");
          o->callback((Fl_Callback*)cb_Flip);
        } // Fl_Button* o
        o->end();
      } // Fl_Group* o
      { intercept0 = new Fl_Value_Slider(145, 25, 25, 195, "Intercept");
        intercept0->tooltip("control intercept of plane with axes");
        intercept0->selection_color((Fl_Color)10);
        intercept0->minimum(-1);
        intercept0->callback((Fl_Callback*)cb_intercept0);
      } // Fl_Value_Slider* intercept0
      { dispas0 = new Fl_Choice(20, 30, 110, 20);
        dispas0->down_box(FL_BORDER_BOX);
        dispas0->menu(menu_dispas0);
      } // Fl_Choice* dispas0
      Clip0->end();
    } // Fl_Group* Clip0
    { Clip1 = new Fl_Group(195, 20, 185, 220, "Clipping Plane 2");
      Clip1->box(FL_UP_BOX);
      { Fl_Group* o = new Fl_Group(205, 75, 115, 145, "Plane Normal2");
        o->box(FL_UP_BOX);
        { xdir1 = new Fl_Value_Input(225, 85, 80, 25, "X:");
          xdir1->maximum(0);
          xdir1->step(0.01);
          xdir1->callback((Fl_Callback*)cb_xdir1);
        } // Fl_Value_Input* xdir1
        { ydir1 = new Fl_Value_Input(225, 120, 80, 25, "Y:");
          ydir1->maximum(0);
          ydir1->step(0.01);
          ydir1->value(1);
          ydir1->callback((Fl_Callback*)cb_ydir1);
        } // Fl_Value_Input* ydir1
        { zdir1 = new Fl_Value_Input(225, 155, 80, 25, "Z:");
          zdir1->maximum(0);
          zdir1->step(0.01);
          zdir1->callback((Fl_Callback*)cb_zdir1);
        } // Fl_Value_Input* zdir1
        { unit1 = new Fl_Button(210, 190, 55, 20, "Unitize");
          unit1->tooltip("set to unit normal");
          unit1->callback((Fl_Callback*)cb_unit1);
        } // Fl_Button* unit1
        { Fl_Button* o = new Fl_Button(270, 190, 40, 20, "Flip");
          o->callback((Fl_Callback*)cb_Flip1);
        } // Fl_Button* o
        o->end();
      } // Fl_Group* o
      { intercept1 = new Fl_Value_Slider(335, 25, 25, 195, "Intercept");
        intercept1->tooltip("control intercept of plane with axes");
        intercept1->selection_color((Fl_Color)178);
        intercept1->minimum(-1);
        intercept1->callback((Fl_Callback*)cb_intercept1);
      } // Fl_Value_Slider* intercept1
      { dispas1 = new Fl_Choice(210, 30, 110, 20);
        dispas1->down_box(FL_BORDER_BOX);
        dispas1->menu(menu_dispas1);
      } // Fl_Choice* dispas1
      Clip1->end();
    } // Fl_Group* Clip1
    { Clip2 = new Fl_Group(375, 20, 195, 220, "Clipping Plane 3");
      Clip2->box(FL_UP_BOX);
      { Fl_Group* o = new Fl_Group(395, 75, 115, 145, "Plane Normal3");
        o->box(FL_UP_BOX);
        { xdir2 = new Fl_Value_Input(415, 85, 80, 25, "X:");
          xdir2->maximum(0);
          xdir2->step(0.01);
          xdir2->callback((Fl_Callback*)cb_xdir2);
        } // Fl_Value_Input* xdir2
        { ydir2 = new Fl_Value_Input(415, 120, 80, 25, "Y:");
          ydir2->maximum(0);
          ydir2->step(0.01);
          ydir2->callback((Fl_Callback*)cb_ydir2);
        } // Fl_Value_Input* ydir2
        { zdir2 = new Fl_Value_Input(415, 155, 80, 25, "Z:");
          zdir2->maximum(0);
          zdir2->step(0.01);
          zdir2->value(1);
          zdir2->callback((Fl_Callback*)cb_zdir2);
        } // Fl_Value_Input* zdir2
        { unit2 = new Fl_Button(400, 190, 55, 20, "Unitize");
          unit2->tooltip("set to unit normal");
          unit2->callback((Fl_Callback*)cb_unit2);
        } // Fl_Button* unit2
        { Fl_Button* o = new Fl_Button(460, 190, 45, 20, "Flip");
          o->callback((Fl_Callback*)cb_Flip2);
        } // Fl_Button* o
        o->end();
      } // Fl_Group* o
      { intercept2 = new Fl_Value_Slider(525, 30, 25, 190, "Intercept");
        intercept2->tooltip("control intercept of plane with axes");
        intercept2->selection_color((Fl_Color)148);
        intercept2->minimum(-1);
        intercept2->callback((Fl_Callback*)cb_intercept2);
      } // Fl_Value_Slider* intercept2
      { dispas2 = new Fl_Choice(395, 30, 110, 20);
        dispas2->down_box(FL_BORDER_BOX);
        dispas2->menu(menu_dispas2);
      } // Fl_Choice* dispas2
      Clip2->end();
    } // Fl_Group* Clip2
    { Clip3 = new Fl_Group(5, 250, 185, 220, "Clipping Plane 4");
      Clip3->box(FL_UP_BOX);
      Clip3->align(Fl_Align(FL_ALIGN_BOTTOM));
      { Fl_Group* o = new Fl_Group(15, 305, 115, 145, "Plane Normal4");
        o->box(FL_UP_BOX);
        { xdir3 = new Fl_Value_Input(35, 315, 80, 25, "X:");
          xdir3->maximum(0);
          xdir3->step(0.01);
          xdir3->value(-1);
          xdir3->callback((Fl_Callback*)cb_xdir3);
        } // Fl_Value_Input* xdir3
        { ydir3 = new Fl_Value_Input(35, 350, 80, 25, "Y:");
          ydir3->maximum(0);
          ydir3->step(0.01);
          ydir3->callback((Fl_Callback*)cb_ydir3);
        } // Fl_Value_Input* ydir3
        { zdir3 = new Fl_Value_Input(35, 385, 80, 25, "Z:");
          zdir3->maximum(0);
          zdir3->step(0.01);
          zdir3->callback((Fl_Callback*)cb_zdir3);
        } // Fl_Value_Input* zdir3
        { unit3 = new Fl_Button(20, 420, 55, 20, "Unitize");
          unit3->tooltip("set to unit normal");
          unit3->callback((Fl_Callback*)cb_unit3);
        } // Fl_Button* unit3
        { Fl_Button* o = new Fl_Button(80, 420, 40, 20, "Flip");
          o->callback((Fl_Callback*)cb_Flip3);
        } // Fl_Button* o
        o->end();
      } // Fl_Group* o
      { intercept3 = new Fl_Value_Slider(145, 255, 25, 195, "Intercept");
        intercept3->tooltip("control intercept of plane with axes");
        intercept3->selection_color((Fl_Color)10);
        intercept3->minimum(-1);
        intercept3->callback((Fl_Callback*)cb_intercept3);
      } // Fl_Value_Slider* intercept3
      { dispas3 = new Fl_Choice(20, 260, 110, 20);
        dispas3->down_box(FL_BORDER_BOX);
        dispas3->menu(menu_dispas3);
      } // Fl_Choice* dispas3
      Clip3->end();
    } // Fl_Group* Clip3
    { Clip4 = new Fl_Group(195, 250, 185, 220, "Clipping Plane 5");
      Clip4->box(FL_UP_BOX);
      Clip4->align(Fl_Align(FL_ALIGN_BOTTOM));
      { Fl_Group* o = new Fl_Group(205, 305, 115, 145, "Plane Normal5");
        o->box(FL_UP_BOX);
        { xdir4 = new Fl_Value_Input(225, 315, 80, 25, "X:");
          xdir4->maximum(0);
          xdir4->step(0.01);
          xdir4->callback((Fl_Callback*)cb_xdir4);
        } // Fl_Value_Input* xdir4
        { ydir4 = new Fl_Value_Input(225, 350, 80, 25, "Y:");
          ydir4->maximum(0);
          ydir4->step(0.01);
          ydir4->value(-1);
          ydir4->callback((Fl_Callback*)cb_ydir4);
        } // Fl_Value_Input* ydir4
        { zdir4 = new Fl_Value_Input(225, 385, 80, 25, "Z:");
          zdir4->maximum(0);
          zdir4->step(0.01);
          zdir4->callback((Fl_Callback*)cb_zdir4);
        } // Fl_Value_Input* zdir4
        { unit4 = new Fl_Button(210, 420, 55, 20, "Unitize");
          unit4->tooltip("set to unit normal");
          unit4->callback((Fl_Callback*)cb_unit4);
        } // Fl_Button* unit4
        { Fl_Button* o = new Fl_Button(270, 420, 40, 20, "Flip");
          o->callback((Fl_Callback*)cb_Flip4);
        } // Fl_Button* o
        o->end();
      } // Fl_Group* o
      { intercept4 = new Fl_Value_Slider(335, 255, 25, 195, "Intercept");
        intercept4->tooltip("control intercept of plane with axes");
        intercept4->selection_color((Fl_Color)178);
        intercept4->minimum(-1);
        intercept4->callback((Fl_Callback*)cb_intercept4);
      } // Fl_Value_Slider* intercept4
      { dispas4 = new Fl_Choice(210, 260, 110, 20);
        dispas4->down_box(FL_BORDER_BOX);
        dispas4->menu(menu_dispas4);
      } // Fl_Choice* dispas4
      Clip4->end();
    } // Fl_Group* Clip4
    { Clip5 = new Fl_Group(385, 250, 185, 220, "Clipping Plane 6");
      Clip5->box(FL_UP_BOX);
      Clip5->align(Fl_Align(FL_ALIGN_BOTTOM));
      { Fl_Group* o = new Fl_Group(395, 305, 115, 145, "Plane Normal6");
        o->box(FL_UP_BOX);
        { xdir5 = new Fl_Value_Input(415, 315, 80, 25, "X:");
          xdir5->maximum(0);
          xdir5->step(0.01);
          xdir5->callback((Fl_Callback*)cb_xdir5);
        } // Fl_Value_Input* xdir5
        { ydir5 = new Fl_Value_Input(415, 350, 80, 25, "Y:");
          ydir5->maximum(0);
          ydir5->step(0.01);
          ydir5->callback((Fl_Callback*)cb_ydir5);
        } // Fl_Value_Input* ydir5
        { zdir5 = new Fl_Value_Input(415, 385, 80, 25, "Z:");
          zdir5->maximum(0);
          zdir5->step(0.01);
          zdir5->value(-1);
          zdir5->callback((Fl_Callback*)cb_zdir5);
        } // Fl_Value_Input* zdir5
        { unit5 = new Fl_Button(400, 420, 55, 20, "Unitize");
          unit5->tooltip("set to unit normal");
          unit5->callback((Fl_Callback*)cb_unit5);
        } // Fl_Button* unit5
        { Fl_Button* o = new Fl_Button(460, 420, 45, 20, "Flip");
          o->callback((Fl_Callback*)cb_Flip5);
        } // Fl_Button* o
        o->end();
      } // Fl_Group* o
      { intercept5 = new Fl_Value_Slider(525, 260, 25, 190, "Intercept");
        intercept5->tooltip("control intercept of plane with axes");
        intercept5->selection_color((Fl_Color)148);
        intercept5->minimum(-1);
        intercept5->callback((Fl_Callback*)cb_intercept5);
      } // Fl_Value_Slider* intercept5
      { dispas5 = new Fl_Choice(400, 260, 110, 20);
        dispas5->down_box(FL_BORDER_BOX);
        dispas5->menu(menu_dispas5);
      } // Fl_Choice* dispas5
      Clip5->end();
    } // Fl_Group* Clip5
    window->end();
  } // Fl_Double_Window* window
  tbmw = tb;
dim = 1;
for( int i=0; i<4; i++ )
	for( int j=0; j<6; j++ )
		x[j][i] = 0;
x[0][0] = 1;
x[1][1] = 1;
x[2][2] = 1;
x[3][0] = -1;
x[4][1] = -1;
x[5][2] = -1;

for( int i=0; i<6; i++ ) intercept[i] = 0;
}

bool ClipPlane::on(int cp) {
  if( cp==0 )
  return dispas0->value()>0;
else if( cp==1 )
  return dispas1->value()>0;
else if( cp==2 )
  return dispas2->value()>0;
else if( cp==3 )
  return dispas3->value()>0;
else if( cp==4 )
  return dispas4->value()>0;
else if( cp==5 )
  return dispas5->value()>0;
}

GLdouble* ClipPlane::plane(int cp) {
  return x[cp];
}

void ClipPlane::normcb( int cp) {
  if( !cp ) {
  x[cp][0] = xdir0->value();
  x[cp][1] = ydir0->value(); 
  x[cp][2] = zdir0->value();
  x[cp][3] = intercept0->value()*dim;
  if( drawIntercept(0) ) tbmw->determine_cutplane( 0 );
  if( dispas0->value()>0 ) tbmw->redraw();
} else if (cp==1) {
  x[cp][0] = xdir1->value();
  x[cp][1] = ydir1->value();
  x[cp][2] = zdir1->value();
  x[cp][3] = intercept1->value()*dim;
  if( drawIntercept(1) ) tbmw->determine_cutplane( 1 );
  if( dispas1->value()>0 ) tbmw->redraw();
}else if (cp==2) {
  x[cp][0] = xdir2->value();
  x[cp][1] = ydir2->value();
  x[cp][2] = zdir2->value();
  x[cp][3] = intercept2->value()*dim;
  if( drawIntercept(2) ) tbmw->determine_cutplane( 2 );
  if( dispas2->value()>0 ) tbmw->redraw();
}else if (cp==3) {
  x[cp][0] = xdir3->value();
  x[cp][1] = ydir3->value();
  x[cp][2] = zdir3->value();
  x[cp][3] = intercept3->value()*dim;
  if( drawIntercept(3) ) tbmw->determine_cutplane( 3 );
  if( dispas3->value()>0 ) tbmw->redraw();
}else if (cp==4) {
  x[cp][0] = xdir4->value();
  x[cp][1] = ydir4->value();
  x[cp][2] = zdir4->value();
  x[cp][3] = intercept4->value()*dim;
  if( drawIntercept(4) ) tbmw->determine_cutplane( 4 );
  if( dispas4->value()>0 ) tbmw->redraw();
}else if (cp==5) {
  x[cp][0] = xdir5->value();
  x[cp][1] = ydir5->value();
  x[cp][2] = zdir5->value();
  x[cp][3] = intercept5->value()*dim;
  if( drawIntercept(5) ) tbmw->determine_cutplane( 5 );
  if( dispas5->value()>0 ) tbmw->redraw();
}
}

void ClipPlane::set_dim( float d) {
  dim = d;
}

void ClipPlane::unitize(Fl_Value_Input* x, Fl_Value_Input* y,Fl_Value_Input* z) {
  double va = x->value();
double vb = y->value();
double vc = z->value();
double mag = sqrt(va*va+vb*vb+vc*vc);
x->value(va/mag);
y->value(vb/mag);
z->value(vc/mag);
}

bool ClipPlane::visible(int cp) {
  if( cp==0 )
  return dispas0->value()==2;
else if( cp==1 )
  return dispas1->value()==2;
else if( cp==2 )
  return dispas2->value()==2;
else if( cp==3 )
  return dispas3->value()==2;
else if( cp==4 )
  return dispas4->value()==2;
else if( cp==5 )
  return dispas5->value()==2;
}

void ClipPlane::flip(int cp) {
  if( cp==0 ) {
  xdir0->value(-xdir0->value());
  ydir0->value(-ydir0->value());
  zdir0->value(-zdir0->value());
  intercept0->value(-intercept0->value());
} else if( cp==1 ){
  xdir1->value(-xdir1->value());
  ydir1->value(-ydir1->value());
  zdir1->value(-zdir1->value());
  intercept1->value(-intercept1->value());
} else if( cp==2 ){
  xdir2->value(-xdir2->value());
  ydir2->value(-ydir2->value());
  zdir2->value(-zdir2->value());
  intercept2->value(-intercept2->value());
}else if( cp==3 ){
  xdir3->value(-xdir3->value());
  ydir3->value(-ydir3->value());
  zdir3->value(-zdir3->value());
  intercept3->value(-intercept3->value());
} else if( cp==4 ){
  xdir4->value(-xdir4->value());
  ydir4->value(-ydir4->value());
  zdir4->value(-zdir4->value());
  intercept4->value(-intercept4->value());
}else if( cp==5 ){
  xdir5->value(-xdir5->value());
  ydir5->value(-ydir5->value());
  zdir5->value(-zdir5->value());
  intercept5->value(-intercept5->value());
}
normcb(cp);
}

void ClipPlane::drawIntercept( int cp, bool b ) {
  intercept[cp] = b;
}

bool ClipPlane::drawIntercept( int cp ) {
  return intercept[cp];
}

bool ClipPlane::datafied( int cp ) {
  if( !cp )
  return dispas0->value()==4;
else if( cp==1 )
  return dispas1->value()==4;
else if( cp==2 )
  return dispas2->value()==4;
else if( cp==3 )
  return dispas3->value()==4;
else if( cp==4 )
  return dispas4->value()==4;
else if( cp==5 )
  return dispas5->value()==4;
}

void ClipPlane::save(ostream& ofs) {
  ofs << SAVE_CP(0) << endl;
ofs << SAVE_CP(1) << endl;
ofs << SAVE_CP(2) << endl;
ofs << SAVE_CP(3) << endl;
ofs << SAVE_CP(4) << endl;
ofs << SAVE_CP(5) << endl;
for( int i=0; i<6; i++ ) {
  for( int j=0; j<4; j++ )
    ofs << x[i][j] << " ";
  ofs << intercept[i] << endl;
}
}

void ClipPlane::read(istream& ifs) {
  READ_CP(0,ifs);
READ_CP(1,ifs);
READ_CP(2,ifs);
READ_CP(3,ifs);
READ_CP(4,ifs);
READ_CP(5,ifs);
for( int i=0; i<6; i++ ){
  for( int j=0; j<4; j++ )
    ifs >> x[i][j];
  ifs >> intercept[i];
}
CHECK_CP(0);
CHECK_CP(1);
CHECK_CP(2);
CHECK_CP(3);
CHECK_CP(4);
CHECK_CP(5);
}
#include <iostream>
using namespace std;

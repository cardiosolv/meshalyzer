// generated by Fast Light User Interface Designer (fluid) version 1.0108

#ifndef trimesh_h
#define trimesh_h
#include <FL/Fl.H>
#include "Surfaces.h"
#include <GL/gl.h>
class TBmeshWin;
#include "Model.h"
#include <FL/Fl_Double_Window.H>
#include "TBmeshWin.h"

class Meshwin {
public:
  Meshwin();
  Fl_Double_Window *winny;
  TBmeshWin *trackballwin;

class UserInterface {
};
};
#include <FL/Fl_Color_Chooser.h>
#include "VecData.h"
#include <FL/Fl_Menu_Bar.H>
#include <FL/fl_file_chooser.h>
#include "Sequence.h"
#include <FL/Fl_Choice.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>
#include "MyValueInput.h"
#include <FL/Fl_Check_Browser.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Roller.H>
#include "Colourscale.h"
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Value_Input.H>
#include "Myslider.h"
#include <FL/Fl_Counter.H>

class Controls {
public:
  Controls();
  Fl_Double_Window *window;
  Fl_Menu_Bar *mainbar;
  static Fl_Menu_Item menu_mainbar[];
private:
  void cb_Add_i(Fl_Menu_*, void*);
  static void cb_Add(Fl_Menu_*, void*);
  void cb_Read_i(Fl_Menu_*, void*);
  static void cb_Read(Fl_Menu_*, void*);
  void cb_Read1_i(Fl_Menu_*, void*);
  static void cb_Read1(Fl_Menu_*, void*);
  void cb_Read2_i(Fl_Menu_*, void*);
  static void cb_Read2(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *read_recalibrate;
private:
  void cb_Save_i(Fl_Menu_*, void*);
  static void cb_Save(Fl_Menu_*, void*);
  void cb_Read3_i(Fl_Menu_*, void*);
  static void cb_Read3(Fl_Menu_*, void*);
  void cb_Save1_i(Fl_Menu_*, void*);
  static void cb_Save1(Fl_Menu_*, void*);
  void cb_Restore_i(Fl_Menu_*, void*);
  static void cb_Restore(Fl_Menu_*, void*);
  void cb_Quit_i(Fl_Menu_*, void*);
  static void cb_Quit(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *imgmenu;
private:
  void cb_Reset_i(Fl_Menu_*, void*);
  static void cb_Reset(Fl_Menu_*, void*);
  void cb__i(Fl_Menu_*, void*);
  static void cb_(Fl_Menu_*, void*);
  void cb_item_i(Fl_Menu_*, void*);
  static void cb_item(Fl_Menu_*, void*);
  void cb_item1_i(Fl_Menu_*, void*);
  static void cb_item1(Fl_Menu_*, void*);
  void cb_item2_i(Fl_Menu_*, void*);
  static void cb_item2(Fl_Menu_*, void*);
  void cb_1_i(Fl_Menu_*, void*);
  static void cb_1(Fl_Menu_*, void*);
  void cb_item3_i(Fl_Menu_*, void*);
  static void cb_item3(Fl_Menu_*, void*);
  void cb_item4_i(Fl_Menu_*, void*);
  static void cb_item4(Fl_Menu_*, void*);
  void cb_item5_i(Fl_Menu_*, void*);
  static void cb_item5(Fl_Menu_*, void*);
  void cb_2_i(Fl_Menu_*, void*);
  static void cb_2(Fl_Menu_*, void*);
  void cb_item6_i(Fl_Menu_*, void*);
  static void cb_item6(Fl_Menu_*, void*);
  void cb_item7_i(Fl_Menu_*, void*);
  static void cb_item7(Fl_Menu_*, void*);
  void cb_item8_i(Fl_Menu_*, void*);
  static void cb_item8(Fl_Menu_*, void*);
  void cb_3_i(Fl_Menu_*, void*);
  static void cb_3(Fl_Menu_*, void*);
  void cb_item9_i(Fl_Menu_*, void*);
  static void cb_item9(Fl_Menu_*, void*);
  void cb_itema_i(Fl_Menu_*, void*);
  static void cb_itema(Fl_Menu_*, void*);
  void cb_itemb_i(Fl_Menu_*, void*);
  static void cb_itemb(Fl_Menu_*, void*);
  void cb_4_i(Fl_Menu_*, void*);
  static void cb_4(Fl_Menu_*, void*);
  void cb_itemc_i(Fl_Menu_*, void*);
  static void cb_itemc(Fl_Menu_*, void*);
  void cb_itemd_i(Fl_Menu_*, void*);
  static void cb_itemd(Fl_Menu_*, void*);
  void cb_iteme_i(Fl_Menu_*, void*);
  static void cb_iteme(Fl_Menu_*, void*);
  void cb_5_i(Fl_Menu_*, void*);
  static void cb_5(Fl_Menu_*, void*);
  void cb_itemf_i(Fl_Menu_*, void*);
  static void cb_itemf(Fl_Menu_*, void*);
  void cb_item10_i(Fl_Menu_*, void*);
  static void cb_item10(Fl_Menu_*, void*);
  void cb_item11_i(Fl_Menu_*, void*);
  static void cb_item11(Fl_Menu_*, void*);
  void cb_vertices_i(Fl_Menu_*, void*);
  static void cb_vertices(Fl_Menu_*, void*);
  void cb_cables_i(Fl_Menu_*, void*);
  static void cb_cables(Fl_Menu_*, void*);
  void cb_triangles_i(Fl_Menu_*, void*);
  static void cb_triangles(Fl_Menu_*, void*);
  void cb_surfaces_i(Fl_Menu_*, void*);
  static void cb_surfaces(Fl_Menu_*, void*);
  void cb_voxele_i(Fl_Menu_*, void*);
  static void cb_voxele(Fl_Menu_*, void*);
  void cb_White_i(Fl_Menu_*, void*);
  static void cb_White(Fl_Menu_*, void*);
  void cb_Gray_i(Fl_Menu_*, void*);
  static void cb_Gray(Fl_Menu_*, void*);
  void cb_Black_i(Fl_Menu_*, void*);
  static void cb_Black(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *revdraworder;
private:
  void cb_revdraworder_i(Fl_Menu_*, void*);
  static void cb_revdraworder(Fl_Menu_*, void*);
  void cb_Opacity_i(Fl_Menu_*, void*);
  static void cb_Opacity(Fl_Menu_*, void*);
  void cb_Clipping_i(Fl_Menu_*, void*);
  static void cb_Clipping(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *outmenu;
private:
  void cb_Image_i(Fl_Menu_*, void*);
  static void cb_Image(Fl_Menu_*, void*);
  void cb_Sequence_i(Fl_Menu_*, void*);
  static void cb_Sequence(Fl_Menu_*, void*);
  void cb_PDF_i(Fl_Menu_*, void*);
  static void cb_PDF(Fl_Menu_*, void*);
  void cb_EPS_i(Fl_Menu_*, void*);
  static void cb_EPS(Fl_Menu_*, void*);
  void cb_Visible_i(Fl_Menu_*, void*);
  static void cb_Visible(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *recordmenu;
private:
  void cb_start_i(Fl_Menu_*, void*);
  static void cb_start(Fl_Menu_*, void*);
  void cb_stop_i(Fl_Menu_*, void*);
  static void cb_stop(Fl_Menu_*, void*);
public:
  static Fl_Menu_Item *redrawbut;
private:
  void cb_redrawbut_i(Fl_Menu_*, void*);
  static void cb_redrawbut(Fl_Menu_*, void*);
public:
  Fl_Choice *datachc;
  static Fl_Menu_Item menu_datachc[];
private:
  void cb_Vertices_i(Fl_Menu_*, void*);
  static void cb_Vertices(Fl_Menu_*, void*);
  void cb_Cables_i(Fl_Menu_*, void*);
  static void cb_Cables(Fl_Menu_*, void*);
  void cb_Connections_i(Fl_Menu_*, void*);
  static void cb_Connections(Fl_Menu_*, void*);
  void cb_Triangles_i(Fl_Menu_*, void*);
  static void cb_Triangles(Fl_Menu_*, void*);
  void cb_Surfaces_i(Fl_Menu_*, void*);
  static void cb_Surfaces(Fl_Menu_*, void*);
  void cb_Tetrahedra_i(Fl_Menu_*, void*);
  static void cb_Tetrahedra(Fl_Menu_*, void*);
  void cb_Nothing_i(Fl_Menu_*, void*);
  static void cb_Nothing(Fl_Menu_*, void*);
  void cb_All_i(Fl_Menu_*, void*);
  static void cb_All(Fl_Menu_*, void*);
public:
  Fl_Light_Button *draw_axes;
private:
  void cb_draw_axes_i(Fl_Light_Button*, void*);
  static void cb_draw_axes(Fl_Light_Button*, void*);
  void cb_Voxele_i(Fl_Button*, void*);
  static void cb_Voxele(Fl_Button*, void*);
  void cb_Surfaces1_i(Fl_Check_Button*, void*);
  static void cb_Surfaces1(Fl_Check_Button*, void*);
  void cb_Tetrahedral_i(Fl_Check_Button*, void*);
  static void cb_Tetrahedral(Fl_Check_Button*, void*);
public:
  Fl_Group *regiondisp;
  Fl_Light_Button *vertbut;
private:
  void cb_vertbut_i(Fl_Light_Button*, void*);
  static void cb_vertbut(Fl_Light_Button*, void*);
public:
  Fl_Light_Button *cabbut;
private:
  void cb_cabbut_i(Fl_Light_Button*, void*);
  static void cb_cabbut(Fl_Light_Button*, void*);
public:
  Fl_Light_Button *cnnxbut;
private:
  void cb_cnnxbut_i(Fl_Light_Button*, void*);
  static void cb_cnnxbut(Fl_Light_Button*, void*);
  void cb_colour_i(Fl_Button*, void*);
  static void cb_colour(Fl_Button*, void*);
  void cb_colour1_i(Fl_Button*, void*);
  static void cb_colour1(Fl_Button*, void*);
  void cb_colour2_i(Fl_Button*, void*);
  static void cb_colour2(Fl_Button*, void*);
public:
  MyValueInput *vertstride;
private:
  void cb_vertstride_i(MyValueInput*, void*);
  static void cb_vertstride(MyValueInput*, void*);
public:
  MyValueInput *cabstridein;
private:
  void cb_cabstridein_i(MyValueInput*, void*);
  static void cb_cabstridein(MyValueInput*, void*);
public:
  MyValueInput *cnnxstridein;
private:
  void cb_cnnxstridein_i(MyValueInput*, void*);
  static void cb_cnnxstridein(MyValueInput*, void*);
public:
  Fl_Light_Button *visbut;
private:
  void cb_visbut_i(Fl_Light_Button*, void*);
  static void cb_visbut(Fl_Light_Button*, void*);
  void cb_select_i(Fl_Button*, void*);
  static void cb_select(Fl_Button*, void*);
public:
  Fl_Check_Browser *reglist;
private:
  void cb_invert_i(Fl_Button*, void*);
  static void cb_invert(Fl_Button*, void*);
public:
  Fl_Tabs *tabwidget;
  Fl_Group *hilightgrp;
  MyValueInput *tethi;
private:
  void cb_tethi_i(MyValueInput*, void*);
  static void cb_tethi(MyValueInput*, void*);
public:
  MyValueInput *elehi;
private:
  void cb_elehi_i(MyValueInput*, void*);
  static void cb_elehi(MyValueInput*, void*);
public:
  MyValueInput *cabhi;
private:
  void cb_cabhi_i(MyValueInput*, void*);
  static void cb_cabhi(MyValueInput*, void*);
public:
  MyValueInput *verthi;
private:
  void cb_verthi_i(MyValueInput*, void*);
  static void cb_verthi(MyValueInput*, void*);
public:
  MyValueInput *cnnxhi;
private:
  void cb_cnnxhi_i(MyValueInput*, void*);
  static void cb_cnnxhi(MyValueInput*, void*);
public:
  Fl_Light_Button *hilighton;
private:
  void cb_hilighton_i(Fl_Light_Button*, void*);
  static void cb_hilighton(Fl_Light_Button*, void*);
public:
  Fl_Choice *attachtype;
  static Fl_Menu_Item menu_attachtype[];
private:
  void cb_triangles1_i(Fl_Menu_*, void*);
  static void cb_triangles1(Fl_Menu_*, void*);
  void cb_Vols_i(Fl_Menu_*, void*);
  static void cb_Vols(Fl_Menu_*, void*);
  void cb_filled_i(Fl_Menu_*, void*);
  static void cb_filled(Fl_Menu_*, void*);
  void cb_nothing_i(Fl_Menu_*, void*);
  static void cb_nothing(Fl_Menu_*, void*);
  void cb_current_i(Fl_Button*, void*);
  static void cb_current(Fl_Button*, void*);
  void cb_6_i(Fl_Button*, void*);
  static void cb_6(Fl_Button*, void*);
public:
  Fl_Value_Output *vertvalout;
  Fl_Choice *hitettype;
  static Fl_Menu_Item menu_hitettype[];
private:
  void cb_Vol_i(Fl_Menu_*, void*);
  static void cb_Vol(Fl_Menu_*, void*);
  void cb_Solid_i(Fl_Menu_*, void*);
  static void cb_Solid(Fl_Menu_*, void*);
public:
  Fl_Button *pickvert;
private:
  void cb_pickvert_i(Fl_Button*, void*);
  static void cb_pickvert(Fl_Button*, void*);
  void cb_plot_i(Fl_Button*, void*);
  static void cb_plot(Fl_Button*, void*);
public:
  Fl_Group *vectorgrp;
  Fl_Light_Button *veconbut;
private:
  void cb_veconbut_i(Fl_Light_Button*, void*);
  static void cb_veconbut(Fl_Light_Button*, void*);
public:
  Fl_Roller *veclength;
private:
  void cb_veclength_i(Fl_Roller*, void*);
  static void cb_veclength(Fl_Roller*, void*);
public:
  Fl_Button *veccolor;
private:
  void cb_veccolor_i(Fl_Button*, void*);
  static void cb_veccolor(Fl_Button*, void*);
public:
  Fl_Light_Button *vecautocal;
private:
  void cb_vecautocal_i(Fl_Light_Button*, void*);
  static void cb_vecautocal(Fl_Light_Button*, void*);
public:
  Fl_Choice *veccstype;
  static Fl_Menu_Item menu_veccstype[];
private:
  void cb_7_i(Fl_Menu_*, void*);
  static void cb_7(Fl_Menu_*, void*);
  void cb_8_i(Fl_Menu_*, void*);
  static void cb_8(Fl_Menu_*, void*);
  void cb_9_i(Fl_Menu_*, void*);
  static void cb_9(Fl_Menu_*, void*);
  void cb_a_i(Fl_Menu_*, void*);
  static void cb_a(Fl_Menu_*, void*);
  void cb_b_i(Fl_Menu_*, void*);
  static void cb_b(Fl_Menu_*, void*);
  void cb_c_i(Fl_Menu_*, void*);
  static void cb_c(Fl_Menu_*, void*);
public:
  Fl_Menu_Button *veclengthdeterm;
  static Fl_Menu_Item menu_veclengthdeterm[];
private:
  void cb_Vector_i(Fl_Menu_*, void*);
  static void cb_Vector(Fl_Menu_*, void*);
  void cb_Scalar_i(Fl_Menu_*, void*);
  static void cb_Scalar(Fl_Menu_*, void*);
  void cb_Fixed_i(Fl_Menu_*, void*);
  static void cb_Fixed(Fl_Menu_*, void*);
public:
  Fl_Menu_Button *veccoldeterm;
  static Fl_Menu_Item menu_veccoldeterm[];
private:
  void cb_Vector1_i(Fl_Menu_*, void*);
  static void cb_Vector1(Fl_Menu_*, void*);
  void cb_Scalar1_i(Fl_Menu_*, void*);
  static void cb_Scalar1(Fl_Menu_*, void*);
  void cb_Fixed1_i(Fl_Menu_*, void*);
  static void cb_Fixed1(Fl_Menu_*, void*);
  void cb_optimal_i(Fl_Button*, void*);
  static void cb_optimal(Fl_Button*, void*);
public:
  Fl_Group *lightgrp;
  Fl_Value_Slider *lightx;
private:
  void cb_lightx_i(Fl_Value_Slider*, void*);
  static void cb_lightx(Fl_Value_Slider*, void*);
public:
  Fl_Value_Slider *lighty;
private:
  void cb_lighty_i(Fl_Value_Slider*, void*);
  static void cb_lighty(Fl_Value_Slider*, void*);
public:
  Fl_Value_Slider *lightz;
private:
  void cb_lightz_i(Fl_Value_Slider*, void*);
  static void cb_lightz(Fl_Value_Slider*, void*);
  void cb_reset_i(Fl_Button*, void*);
  static void cb_reset(Fl_Button*, void*);
public:
  Fl_Light_Button *showLightDir;
private:
  void cb_showLightDir_i(Fl_Light_Button*, void*);
  static void cb_showLightDir(Fl_Light_Button*, void*);
  void cb_zero_i(Fl_Button*, void*);
  static void cb_zero(Fl_Button*, void*);
  void cb_zero1_i(Fl_Button*, void*);
  static void cb_zero1(Fl_Button*, void*);
  void cb_zero2_i(Fl_Button*, void*);
  static void cb_zero2(Fl_Button*, void*);
public:
  Fl_Light_Button *illBut;
private:
  void cb_illBut_i(Fl_Light_Button*, void*);
  static void cb_illBut(Fl_Light_Button*, void*);
public:
  Fl_Value_Slider *ambientslide;
private:
  void cb_ambientslide_i(Fl_Value_Slider*, void*);
  static void cb_ambientslide(Fl_Value_Slider*, void*);
public:
  Fl_Light_Button *facetshadBut;
private:
  void cb_facetshadBut_i(Fl_Light_Button*, void*);
  static void cb_facetshadBut(Fl_Light_Button*, void*);
  void cb_fixed_i(Fl_Light_Button*, void*);
  static void cb_fixed(Fl_Light_Button*, void*);
public:
  Fl_Value_Slider *specularslide;
private:
  void cb_specularslide_i(Fl_Value_Slider*, void*);
  static void cb_specularslide(Fl_Value_Slider*, void*);
public:
  Fl_Value_Slider *diffuseslide;
private:
  void cb_diffuseslide_i(Fl_Value_Slider*, void*);
  static void cb_diffuseslide(Fl_Value_Slider*, void*);
public:
  Fl_Value_Slider *backintensityslide;
private:
  void cb_backintensityslide_i(Fl_Value_Slider*, void*);
  static void cb_backintensityslide(Fl_Value_Slider*, void*);
public:
  Fl_Light_Button *surfvisbut;
private:
  void cb_surfvisbut_i(Fl_Light_Button*, void*);
  static void cb_surfvisbut(Fl_Light_Button*, void*);
public:
  Fl_Light_Button *surfoutlinebut;
private:
  void cb_surfoutlinebut_i(Fl_Light_Button*, void*);
  static void cb_surfoutlinebut(Fl_Light_Button*, void*);
public:
  Fl_Light_Button *surffillbut;
private:
  void cb_surffillbut_i(Fl_Light_Button*, void*);
  static void cb_surffillbut(Fl_Light_Button*, void*);
  void cb_Outline_i(Fl_Button*, void*);
  static void cb_Outline(Fl_Button*, void*);
  void cb_Fill_i(Fl_Button*, void*);
  static void cb_Fill(Fl_Button*, void*);
public:
  Fl_Check_Browser *surflist;
private:
  void cb_select1_i(Fl_Button*, void*);
  static void cb_select1(Fl_Button*, void*);
  void cb_invert1_i(Fl_Button*, void*);
  static void cb_invert1(Fl_Button*, void*);
  static Fl_Menu_Item menu_delete[];
  void cb_really_i(Fl_Menu_*, void*);
  static void cb_really(Fl_Menu_*, void*);
public:
  Fl_Group *colorscalegrp;
private:
  void cb_optimal1_i(Fl_Button*, void*);
  static void cb_optimal1(Fl_Button*, void*);
public:
  Fl_Choice *cstype;
  static Fl_Menu_Item menu_cstype[];
private:
  void cb_d_i(Fl_Menu_*, void*);
  static void cb_d(Fl_Menu_*, void*);
  void cb_e_i(Fl_Menu_*, void*);
  static void cb_e(Fl_Menu_*, void*);
  void cb_f_i(Fl_Menu_*, void*);
  static void cb_f(Fl_Menu_*, void*);
  void cb_10_i(Fl_Menu_*, void*);
  static void cb_10(Fl_Menu_*, void*);
  void cb_11_i(Fl_Menu_*, void*);
  static void cb_11(Fl_Menu_*, void*);
  void cb_12_i(Fl_Menu_*, void*);
  static void cb_12(Fl_Menu_*, void*);
public:
  Fl_Value_Input *mincolval;
private:
  void cb_mincolval_i(Fl_Value_Input*, void*);
  static void cb_mincolval(Fl_Value_Input*, void*);
public:
  Fl_Value_Input *maxcolval;
private:
  void cb_maxcolval_i(Fl_Value_Input*, void*);
  static void cb_maxcolval(Fl_Value_Input*, void*);
public:
  Fl_Value_Input *numcolev;
private:
  void cb_numcolev_i(Fl_Value_Input*, void*);
  static void cb_numcolev(Fl_Value_Input*, void*);
public:
  Fl_Light_Button *autocalibratebut;
private:
  void cb_autocalibratebut_i(Fl_Light_Button*, void*);
  static void cb_autocalibratebut(Fl_Light_Button*, void*);
public:
  Fl_Group *animate_group;
  Myslider *tmslider;
private:
  void cb_tmslider_i(Myslider*, void*);
  static void cb_tmslider(Myslider*, void*);
  void cb_13_i(Fl_Button*, void*);
  static void cb_13(Fl_Button*, void*);
  void cb_14_i(Fl_Button*, void*);
  static void cb_14(Fl_Button*, void*);
public:
  Fl_Counter *frameskip;
private:
  void cb_frameskip_i(Fl_Counter*, void*);
  static void cb_frameskip(Fl_Counter*, void*);
public:
  Fl_Value_Input *animdelay;
private:
  void cb_animdelay_i(Fl_Value_Input*, void*);
  static void cb_animdelay(Fl_Value_Input*, void*);
public:
  void outputwin( TBmeshWin *w );
private:
  TBmeshWin *mwtb; 
public:
  void pickcolor(Object_t obj, const char* wintitle );
  void update_surfGUI( int s );
  void update_regionGUI( int r );
  void highlight_region();
  void set_rot( char n, int neg, int view );
  void set_tet_region( Model *model);
  bool * regselected();
  void save_state();
  void save_colour( ofstream&ofs, GLfloat *col );
  void pick_surf_color(Object_t ot, const char *winlab);
  void restore_state(const char* fname=NULL);
  void surfselected(vector<int>& lst);
  void refresh_surflist();
};
#endif

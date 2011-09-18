#ifndef TEST_S_AUX_GRIDS
#define TEST_S_AUX_GRIDS

#include "types.h"
#include <hdf5.h>

int s_aux_creates_grid();
int s_aux_gets_grid_info();
int s_aux_counts_grids();
int s_aux_writes_next_time();
int s_aux_gets_time_info();
int s_aux_reads_time();

int _s_aux_compare_info(int n, ch5s_aux_grid *expected, ch5s_aux_grid *given);
int _s_aux_check_grid_at_time(hid_t file, unsigned int grid_index,
  unsigned int time_index, unsigned int np, float *expected_points,
  unsigned int ne, unsigned int max_elem_width,
  unsigned int *expected_elements, float *expected_data);
int _s_aux_compare_time_info(int g, int t, ch5s_aux_time_step *expected, ch5s_aux_time_step *given);

#endif
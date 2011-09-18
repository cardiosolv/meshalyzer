#include "test_s_aux_grids.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <stdlib.h>

int s_aux_creates_grid() {
  OPEN_WRITE_TEST_FILE;
  
  int result = 1;
  int new_index = ch5s_aux_create_grid(file, 2., 0.5, "Label", "Time Units", "Units", "Comments");
  if (new_index < 0) {
    fprintf(stderr, "Error in create function\n");
    result = 0;
  }
  
  htri_t exists = H5Lexists(file, CH5_AUX_GROUP_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find aux grids group\n");
    result = 0;
  }
  
  hid_t main_group_id = H5Gopen(file, CH5_AUX_GROUP_NAME, H5P_DEFAULT);
  int count = ch5_nchild_count_children(main_group_id);
  if (count != 1) {
    fprintf(stderr, "Child count should be 1 but was %d\n", count);
    result = 0;
  }
  
  exists = H5Lexists(main_group_id, "Grid000000", H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find aux grid (index = 0)\n");
    result = 0;
  }
  
  hid_t grid_id = H5Gopen(main_group_id, "Grid000000", H5P_DEFAULT);
  if (grid_id < 0) {
    fprintf(stderr, "Error opening aux grid group (index = 0)\n");
    result = 0;
  }
  
  H5Gclose(grid_id);
  H5Gclose(main_group_id);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int s_aux_gets_grid_info() {
  hid_t file = fixture_create_auxgrid_hdf_file();
  int result0, result1, free_results;
  
  ch5s_aux_grid expected_0 = { 2, 0.5, 1., "Label", "Time Units", "Units", "Comments" };
  ch5s_aux_grid info_0;
  result0 = (ch5s_aux_grid_info(file, 0, &info_0) == 0);
  if (result0 == 0) fprintf(stderr, "Function failed on n=0\n");
  else if (_s_aux_compare_info(0, &expected_0, &info_0) == 0) result0 = 0;
  
  ch5s_aux_grid expected_1 = { 2, 1.25, 6.66, "AphexTwin", NULL, "RumbleFish", NULL };
  ch5s_aux_grid info_1;
  result1 = (ch5s_aux_grid_info(file, 1, &info_1) == 0);
  if (result1 == 0) fprintf(stderr, "Function failed on n=1\n");
  else if (_s_aux_compare_info(1, &expected_1, &info_1) == 0) result1 = 0;
  
  fixture_close_auxgrid_hdf_file(file);
  
  free_results  = (ch5s_aux_free_grid_info(&info_0) == 0);
  free_results &= (ch5s_aux_free_grid_info(&info_1) == 0);
  
  return result0 && result1 && free_results;
}

int _s_aux_compare_info(int n, ch5s_aux_grid *expected, ch5s_aux_grid *given) {
  int result = 1;
  char buf[50];
  sprintf(buf, "Aux grid (n=%d)", n);
  if (expected->time_steps != given->time_steps) {
    fprintf(stderr, "%s mismatch: time_steps: expected %d but given %d\n",
      buf, expected->time_steps, given->time_steps);
    result = 0;
  }
  if (expected->time_delta != given->time_delta) {
    fprintf(stderr, "%s mismatch: time_delta: expected %f but given %f\n",
      buf, expected->time_delta, given->time_delta);
    result = 0;
  }
  result &= compare_strings(buf, "label",      expected->label,      given->label);
  result &= compare_strings(buf, "time_units", expected->time_units, given->time_units);
  result &= compare_strings(buf, "units",      expected->units,      given->units);
  result &= compare_strings(buf, "comments",   expected->comments,   given->comments);
  
  return result;
}

int s_aux_counts_grids() {
  hid_t file = fixture_create_auxgrid_hdf_file();
  int count = ch5s_aux_grid_count(file);
  fixture_close_auxgrid_hdf_file(file);
  return count == 2;
}

int s_aux_writes_next_time() {
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  int grid_index = ch5s_aux_create_grid(file, 4., 1.0, "A", "B", "C", "D");
  if (grid_index < 0) {
    fprintf(stderr, "Error in create function\n");
    result = 0;
  }
  
  int status;
  
  float        time1pts[9]     = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  unsigned int time1elem_w[12] = { 0, 1, 2, 1, 1, 2, 3, 2, 1, 2, 3, 4 };
  unsigned int time1elem_r[18] = { 0, 0, 1, 2, 0, 0, 1, 0, 1, 2, 3, 0, 2, 0, 1, 2, 3, 4 };
  float        time1data[3]    = { 7, 8, 9 };
  status = ch5s_aux_write_next(file, grid_index, 3, time1pts, 3, 4, time1elem_w, time1data);
  if (status != 0) {
    fprintf(stderr, "Error in write next function (write_num=1)\n");
    result = 0;
  }
  
  float        time2pts[6]    = { 1, 2, 3, 4, 5, 6 };
  unsigned int time2elem_w[5] = { 2, 1, 2, 3, 4 };
  unsigned int time2elem_r[6] = { 2, 0, 1, 2, 3, 4 };
  status = ch5s_aux_write_next(file, grid_index, 2, time2pts, 1, 4, time2elem_w, NULL);
  if (status != 0) {
    fprintf(stderr, "Error in write next function (write_num=2)\n");
    result = 0;
  }
  
  /* READ BACK */
  result &= _s_aux_check_grid_at_time(file, grid_index, 0, 3, time1pts, 3, 4, time1elem_r, time1data);
  result &= _s_aux_check_grid_at_time(file, grid_index, 1, 2, time2pts, 1, 4, time2elem_r, NULL);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int _s_aux_check_grid_at_time(hid_t file, unsigned int grid_index,
  unsigned int time_index, unsigned int np, float *expected_points,
  unsigned int ne, unsigned int max_elem_width, unsigned int *expected_elements,
  float *expected_data)
{
  int result = 1, status;
  hid_t main_group_id = H5Gopen(file, CH5_AUX_GROUP_NAME, H5P_DEFAULT);
  if (main_group_id < 0) {
    fprintf(stderr, "Could not open main aux grid group\n");
    result = 0;
  }

  hid_t grid_id;
  status = ch5_nchild_open_child(main_group_id, grid_index, &grid_id, NULL);
  if (status != 0) {
    fprintf(stderr, "Could not open grid %d on the main aux grid group\n", grid_index);
    result = 0;
  }

  hid_t time_id;
  status = ch5_nchild_open_child(grid_id, time_index, &time_id, NULL);
  if (status != 0) {
    fprintf(stderr, "Could not open time %d of aux grid %d\n", time_index, grid_index);
    result = 0;
  }

  /* Points */
  htri_t exists = H5Lexists(time_id, CH5_AUX_POINTS_DSET_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find points dataset in time %d of aux grid %d\n", time_index, grid_index);
    result = 0;
  }
  float *read_points = (float*) malloc(sizeof(float) * np * 3);
  hid_t points_dset_id = H5Dopen(time_id, CH5_AUX_POINTS_DSET_NAME, H5P_DEFAULT);
  herr_t read_status = H5Dread(points_dset_id, H5T_NATIVE_FLOAT, H5S_ALL,
    H5S_ALL, H5P_DEFAULT, read_points);
  if (read_status < 0) {
    fprintf(stderr, "Error reading from points dataset in time %d of aux grid %d\n", time_index, grid_index);
    result = 0;
  }
  else if (!float_arrays_same(expected_points, read_points, np * 3)) {
    fprintf(stderr, "Read back points incorrect in time %d of aux grid %d\n", time_index, grid_index);
    result = 0;
  }
  free(read_points);
  H5Dclose(points_dset_id);
  
  /* Elements */
  exists = H5Lexists(time_id, CH5_AUX_ELEMS_DSET_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find elements dataset in time %d of aux grid %d\n", time_index, grid_index);
    result = 0;
  }
  unsigned int *read_elements = (unsigned int*) malloc(sizeof(int) * ne * (max_elem_width + CH5_ELEM_WIDTH_ADD));
  hid_t elem_dset_id = H5Dopen(time_id, CH5_AUX_ELEMS_DSET_NAME, H5P_DEFAULT);
  read_status = H5Dread(elem_dset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_elements);
  if (read_status < 0) {
    fprintf(stderr, "Error reading from elements dataset in time %d of aux grid %d\n", time_index, grid_index);
    result = 0;
  }
  else if (!int_arrays_same((int*)expected_elements, (int*)read_elements, ne * (max_elem_width + CH5_ELEM_WIDTH_ADD))) {
    fprintf(stderr, "Read back points incorrect in time %d of aux grid %d\n", time_index, grid_index);
    result = 0;
  }
  free(read_elements);
  H5Dclose(elem_dset_id);
  
  if (expected_data != NULL) {
    exists = H5Lexists(time_id, CH5_AUX_DATA_DSET_NAME, H5P_DEFAULT);
    if (exists < 1) {
      fprintf(stderr, "Could not find data dataset in time %d of aux grid %d\n", time_index, grid_index);
      result = 0;
    }
    float *read_data = (float*) malloc(sizeof(float) * np);
    hid_t data_dset_id = H5Dopen(time_id, CH5_AUX_DATA_DSET_NAME, H5P_DEFAULT);
    read_status = H5Dread(data_dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
      H5P_DEFAULT, read_data);
    if (read_status < 0) {
      fprintf(stderr, "Error reading from elements dataset in time %d of aux grid %d\n", time_index, grid_index);
      result = 0;
    }
    else if (!float_arrays_same(expected_data, read_data, np)) {
      fprintf(stderr, "Read back points incorrect in time %d of aux grid %d\n", time_index, grid_index);
      result = 0;
    }
    free(read_data);
    H5Dclose(data_dset_id);
  }

  H5Gclose(time_id);
  H5Gclose(grid_id);
  H5Gclose(main_group_id);
  
  return result;
}

int s_aux_gets_time_info() {
  int result = 1;
  hid_t file = fixture_create_auxgrid_hdf_file();
  
  ch5s_aux_time_step expected1 = { 3, 3, 4, 1 };
  ch5s_aux_time_step info1;
  int status = ch5s_aux_time_step_info(file, 0, 1, &info1);
  if (status != 0) {
    fprintf(stderr, "Error in time step info function (grid 0, time 1)\n");
    result = 0;
  }
  else if (!_s_aux_compare_time_info(0, 1, &expected1, &info1)) result = 0;
  
  ch5s_aux_time_step expected2 = { 2, 1, 4, 0 };
  ch5s_aux_time_step info2;
  status = ch5s_aux_time_step_info(file, 1, 0, &info2);
  if (status != 0) {
    fprintf(stderr, "Error in time step info function (grid 1, time 0)\n");
    result = 0;
  }
  else if (!_s_aux_compare_time_info(1, 0, &expected2, &info2)) result = 0;
  
  ch5s_aux_time_step expected3 = { 2, 0, 0, 0 };
  ch5s_aux_time_step info3;
  status = ch5s_aux_time_step_info(file, 1, 1, &info3);
  if (status != 0) {
    fprintf(stderr, "Error in time step info function (grid 1, time 1)\n");
    result = 0;
  }
  else if (!_s_aux_compare_time_info(1, 0, &expected3, &info3)) result = 0;
  
  fixture_close_auxgrid_hdf_file(file);
  
  return result;
}

int _s_aux_compare_time_info(int g, int t, ch5s_aux_time_step *expected,
  ch5s_aux_time_step *given)
{
  int result = 1;
  char buf[32];
  sprintf(buf, "Aux grid (n=%d) time step (n=%d)", g, t);
  if (expected->num_points != given->num_points) {
    fprintf(stderr, "%s mismatch: num_points: expected %d but given %d\n",
      buf, expected->num_points, given->num_points);
    result = 0;
  }
  if (expected->num_elements != given->num_elements) {
    fprintf(stderr, "%s mismatch: num_elements: expected %d but given %d\n",
      buf, expected->num_elements, given->num_elements);
    result = 0;
  }
  if (expected->max_element_width != given->max_element_width) {
    fprintf(stderr, "%s mismatch: max_element_width: expected %d but given %d\n",
      buf, expected->max_element_width, given->max_element_width);
    result = 0;
  }
  if (expected->has_data != given->has_data) {
    fprintf(stderr, "%s mismatch: has_data: expected %d but given %d\n",
      buf, expected->has_data, given->has_data);
    result = 0;
  }
  
  return result;
}

int s_aux_reads_time() {
  int result = 1;
  hid_t file = fixture_create_auxgrid_hdf_file();
  
  float      **expected_points    = fixture_auxgrid_points(3);
  unsigned int expected_elems[18] = { 0, 0, 1, 2, 0, 0, 1, 0, 1, 2, 3, 0, 2, 0, 1, 2, 3, 4 };
  float       *expected_data      = fixture_auxgrid_data(3);
  float        read_points[9];
  unsigned int read_elems[18];
  float        read_data[3];
  
  int status = ch5s_aux_read(file, 0, 0, read_points, read_elems, read_data);
  if (status != 0) {
    fprintf(stderr, "Error in read function\n");
    result = 0;
  }
  else {
    if (!float_arrays_same(expected_points[0], read_points, 9)) {
      fprintf(stderr, "Read-back of points incorrect (grid 0, time 0)\n");
      result = 0;
    }
    if (!int_arrays_same((int*)expected_elems, (int*)read_elems, 18)) {
      fprintf(stderr, "Read-back of elements incorrect (grid 0, time 0)\n");
      result = 0;
    }
    if (!float_arrays_same(expected_data, read_data, 3)) {
      fprintf(stderr, "Read-back of data incorrect (grid 0, time 0)\n");
      result = 0;
    }
  }
  
  status  = ch5s_aux_read(file, 0, 0, NULL, read_elems, read_data);
  status &= ch5s_aux_read(file, 0, 0, NULL, NULL, read_data);
  status &= ch5s_aux_read(file, 0, 0, NULL, NULL, NULL);
  status &= ch5s_aux_read(file, 0, 0, NULL, NULL, NULL);/* I suppose this is ok too */
  status &= ch5s_aux_read(file, 0, 0, NULL, read_elems, NULL);
  status &= ch5s_aux_read(file, 0, 0, read_points, NULL, NULL);
  status &= ch5s_aux_read(file, 0, 0, read_points, NULL, read_data);
  
  if (status != 0) {
    fprintf(stderr, "Error in read function passing NULL to various outbound data pointers\n");
    result = 0;
  }
  
  fixture_auxgrid_free_points(expected_points);
  fixture_auxgrid_free_data(expected_data);
  fixture_close_auxgrid_hdf_file(file);
  
  return result;
}

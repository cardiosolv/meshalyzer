#include "test_s_nodal.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"
#include <string.h>

int s_noda_creates_grid() {
  OPEN_WRITE_TEST_FILE;
  
  int result = 1;
  int new_index = ch5s_nodal_create_grid(file, 5, 10, 2.,0.25, CH5_DYN_PTS, "Label", "Time Units", "Units", "Comments");
  if (new_index < 0) {
    fprintf(stderr, "Error in create function.\n");
    result = 0;
  }
  
  htri_t exists = H5Lexists(file, CH5_NODA_GROUP_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find vectors main group\n");
    result = 0;
  }
  
  hid_t main_group_id = H5Gopen(file, CH5_NODA_GROUP_NAME, H5P_DEFAULT);
  int count = ch5_nchild_count_children(main_group_id);
  if (count != 1) {
    fprintf(stderr, "Child count should be 1 but was %d\n", count);
    result = 0;
  }
  
  exists = H5Lexists(main_group_id, "Grid000000", H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find grid dataset\n");
    result = 0;
  }
  
  H5Gclose(main_group_id);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int s_noda_gets_info() {
  hid_t file = fixture_create_nodal_hdf_file();
  int result0, result1, free_results;
  
  ch5s_nodal_grid expected_0 = { 3, 0.125, 3., 2, 1, CH5_SCALAR, "KGB", "seconds", "siemens", NULL };
  ch5s_nodal_grid info_0;
  result0 = (ch5s_nodal_grid_info(file, 0, &info_0) == 0);
  if (result0 == 0) fprintf(stderr, "Function failed on n=0\n");
  else if (_s_noda_compare_info(0, &expected_0, &info_0) == 0) result0 = 0;
  
  ch5s_nodal_grid expected_1 = { 6, 0.5, 2., 4, 3, CH5_DYN_PTS, NULL, NULL, NULL, "Aerodynamique" };
  ch5s_nodal_grid info_1;
  result1 = (ch5s_nodal_grid_info(file, 1, &info_1) == 0);
  if (result1 == 0) fprintf(stderr, "Function failed on n=1\n");
  else if (_s_noda_compare_info(1, &expected_1, &info_1) == 0) result1 = 0;
  
  fixture_close_nodal_hdf_file(file);
  
  free_results  = (ch5s_nodal_free_grid_info(&info_0) == 0);
  free_results &= (ch5s_nodal_free_grid_info(&info_1) == 0);
  
  return result0 && result1 && free_results;
}

int s_noda_counts_grids() {
  hid_t file = fixture_create_nodal_hdf_file();
  
  int count = ch5s_nodal_grid_count(file);
  int result = (count == 2);
  if (result == 0) fprintf(stderr, "Expected %d but given %d\n", 2, count);
  
  fixture_close_nodal_hdf_file(file);
  
  return result;
}

int _s_noda_compare_info(int n, ch5s_nodal_grid *expected, ch5s_nodal_grid *given) {
  int result = 1;
  
  if (expected->time_steps != given->time_steps) {
    fprintf(stderr, "Nodal grid (n=%d) mismatch: time_steps: expected %d but given %d\n",
      n, expected->time_steps, given->time_steps);
    result = 0;
  }
  if (expected->time_delta != given->time_delta) {
    fprintf(stderr, "Nodal grid (n=%d) mismatch: time_delta: expected %f but given %f\n",
      n, expected->time_delta, given->time_delta);
    result = 0;
  }
  if (expected->num_nodes != given->num_nodes) {
    fprintf(stderr, "Nodal grid (n=%d) mismatch: num_nodes: expected %d but given %d\n",
      n, expected->num_nodes, given->num_nodes);
    result = 0;
  }
  if (expected->node_width != given->node_width) {
    fprintf(stderr, "Nodal grid (n=%d) mismatch: node_width: expected %d but given %d\n",
      n, expected->node_width, given->node_width);
    result = 0;
  }
  if (expected->type != given->type) {
    fprintf(stderr, "Nodal grid (n=%d) mismatch: type: expected %d but given %d\n",
      n, expected->type, given->type);
    result = 0;
  }
  result &= _s_noda_compare_strings(n, "label",      expected->label,      given->label);
  result &= _s_noda_compare_strings(n, "time_units", expected->time_units, given->time_units);
  result &= _s_noda_compare_strings(n, "units",      expected->units,      given->units);
  result &= _s_noda_compare_strings(n, "comments",   expected->comments,   given->comments);
  
  return result;
}

int _s_noda_compare_strings(int n, const char *kind, const char *expected, const char *given) {
  if ((expected == NULL && given != NULL) ||
      (expected != NULL && given == NULL) ||
      (expected != NULL && given != NULL && strcmp(expected, given) != 0)) {
    fprintf(stderr, "Nodal grid (n=%d) mismatch: %s: expected \"%s\" but given \"%s\"\n",
      n, kind, expected, given);
    return 0;
  }
  return 1;
}

int s_noda_writes_data() {
  OPEN_WRITE_TEST_FILE;
  
  int result = 1;
  int grid_index = ch5s_nodal_create_grid(file, 3, 3, 3., 1.0, CH5_SCALAR, NULL, NULL, NULL, NULL);
  if (grid_index < 0) {
    fprintf(stderr, "Error creating grid\n");
    result = 0;
  }
  
  float *write_data = fixture_nodal_data(CH5_SCALAR, 3, 3);
  
  int status = 1;
  status &= (ch5s_nodal_write(file, grid_index, 0, 0, write_data)       == 0);
  status &= (ch5s_nodal_write(file, grid_index, 1, 2, (write_data + 3)) == 0);
  if (status == 0) {
    fprintf(stderr, "Error writing data\n");
    result = 0;
  }
  
  hid_t grid_id = H5Dopen(file, CH5_NODA_GROUP_PATH "/Grid000000", H5P_DEFAULT);
  if (grid_id < 0) {
    fprintf(stderr, "Grid not found\n");
    result = 0;
  }
  
  float read_data[3 * 3];
  herr_t read_status = H5Dread(grid_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_data);
  if (read_status < 0) {
    fprintf(stderr, "Data dataset read failed\n");
    result = 0;
  }
  
  if (float_arrays_same(write_data, read_data, 3 * 3) == 0) {
    fprintf(stderr, "Read-back data incorrect\n");
    result = 0;
  }
  
  H5Dclose(grid_id);
  
  fixture_free_nodal_data(write_data);
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int s_noda_error_writing_bad_time_ranges() {
  hid_t file = fixture_create_nodal_hdf_file();
  
  float dummy[50];
  int result = 1;
  result &= (ch5s_nodal_write(file, 0,  -1,  0, dummy) != 0);
  result &= (ch5s_nodal_write(file, 0,   0, -1, dummy) != 0);
  result &= (ch5s_nodal_write(file, 0,   2,  1, dummy) != 0);
  result &= (ch5s_nodal_write(file, 0,  50, 50, dummy) != 0);
  result &= (ch5s_nodal_write(file, 0,   0, 50, dummy) != 0);
  
  fixture_close_nodal_hdf_file(file);
  
  return result;
}

int s_noda_reads_data_by_time_range() {
  int result = 1;
  hid_t file = fixture_create_nodal_hdf_file();
  
  result &= _s_noda_read_and_check_data(file, CH5_SCALAR, 2, 3, 0);
  result &= _s_noda_read_and_check_data(file, CH5_DYN_PTS, 4, 6, 1);
  
  fixture_close_nodal_hdf_file(file);
  
  return result;
}

int _s_noda_read_and_check_data(hid_t file, ch5s_nodal_type type, int n,
  int t, int grid_index)
{
  int result    = 1;
  int data_size = n * t * ((type == CH5_SCALAR) ? CH5_NODA_SCALAR_NODE_WIDTH : CH5_NODA_DYNPTS_NODE_WIDTH);
  
  float *expected = fixture_nodal_data(type, n, t);
  float read[data_size];
  int status = ch5s_nodal_read(file, grid_index, 0, t-1, read);
  if (status != 0) {
    fprintf(stderr, "Read function for data failed (grid %d)\n", grid_index);
    result = 0;
  }
  else if (float_arrays_same(expected, read, data_size) == 0) {
    fprintf(stderr, "Read-back data incorrect (grid %d)\n", grid_index);
    result = 0;
  }
  fixture_free_nodal_data(expected);
  
  return result;
}

int s_noda_reads_data_by_node_index() {
  int result = 1;
  int data_size = 3 * 6;
  hid_t file = fixture_create_nodal_hdf_file();
  
  //grid 1: 4 nodes, 6 time steps
  float expected[3*6] = { 3.0, 4.0, 5.0,   15.0, 16.0, 17.0,   27.0, 28.0, 29.0,   39.0, 40.0, 41.0,   51.0, 52.0, 53.0,   63.0, 64.0, 65.0 };
  float read[data_size];
  int status = ch5s_nodal_read_time_series(file, 1, 1, read);
  if (status != 0) {
    fprintf(stderr, "Read function failed (grid %d)\n", 0);
    result = 0;
  }
  else if (float_arrays_same(expected, read, data_size) == 0) {
    fprintf(stderr, "Read-back data incorrect (grid %d)\n", 0);
    result = 0;
  }
  
  fixture_close_nodal_hdf_file(file);
  
  return result;
}

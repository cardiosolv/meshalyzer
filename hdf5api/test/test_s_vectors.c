#include "test_s_vectors.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"
#include <string.h>

int s_vect_creates_grid() {
  OPEN_WRITE_TEST_FILE;
  
  int result = 1;
  float **points = fixture_points(20);
  int new_index = ch5s_vector_create_grid(file, 20, 30, 1.5, 0.01, points[0], "Lister", "Rimmer", "Seconds", NULL, NULL);
  if (new_index < 0) {
    fprintf(stderr, "Error in create function.\n");
    result = 0;
  }
  
  htri_t exists = H5Lexists(file, CH5_VECS_GROUP_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find vectors main group\n");
    result = 0;
  }
  
  hid_t main_group_id = H5Gopen(file, CH5_VECS_GROUP_NAME, H5P_DEFAULT);
  int count = ch5_nchild_count_children(main_group_id);
  if (count != 1) {
    fprintf(stderr, "Child count should be 1 but was %d\n", count);
    result = 0;
  }
  
  exists = H5Lexists(main_group_id, "Grid000000", H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find grid (index = 0)\n");
    result = 0;
  }
  
  hid_t grid_id = H5Gopen(main_group_id, "Grid000000", H5P_DEFAULT);
  if (grid_id < 0) {
    fprintf(stderr, "Error opening grid (index = 0)\n");
    result = 0;
  }
  
  /* Points dataset */
  exists = H5Lexists(grid_id, CH5_VECS_POINTS_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find points dataset (grid index = 0)\n");
    result = 0;
  }
  
  hid_t points_dset_id = H5Dopen(grid_id, CH5_VECS_POINTS_NAME, H5P_DEFAULT);
  if (points_dset_id < 0) {
    fprintf(stderr, "Error opening points dataset (grid index = 0)\n");
    result = 0;
  }
  
  hid_t space_id = H5Dget_space(points_dset_id);
  hsize_t points_dims[2];
  hsize_t points_expected_dims[2] = { 20, 4 };
  H5Sget_simple_extent_dims(space_id, points_dims, NULL);
  
  if (!int_arrays_same((int*)points_expected_dims, (int*)points_dims, 2)) {
    fprintf(stderr, "Points dataset dimensions incorrect\n");
    result = 0;
  }
  
  float read_points[20 * 3];
  herr_t status = H5Dread(points_dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_points);
  if (status < 0) {
    fprintf(stderr, "Error reading from points dataset (grid index = 0)\n");
    result = 0;
  }
  else if (!float_arrays_same(points[0], read_points, 20 * 3)) {
    fprintf(stderr, "Read-back od points incorrect\n");
    result = 0;
  }
  
  H5Sclose(space_id);
  H5Dclose(points_dset_id);
  
  /* Vectors dataset */
  exists = H5Lexists(grid_id, CH5_VECS_VECTORS_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find vectors dataset (grid index = 0)\n");
    result = 0;
  }
  
  hid_t vecs_dset_id = H5Dopen(grid_id, CH5_VECS_VECTORS_NAME, H5P_DEFAULT);
  if (points_dset_id < 0) {
    fprintf(stderr, "Error opening vectors dataset (grid index = 0)\n");
    result = 0;
  }
  
  space_id = H5Dget_space(vecs_dset_id);
  hsize_t vecs_dims[3];
  hsize_t vecs_expected_dims[3] = { 30, 20, 4 };
  H5Sget_simple_extent_dims(space_id, vecs_dims, NULL);
  
  if (!int_arrays_same((int*)vecs_expected_dims, (int*)vecs_dims, 3)) {
    fprintf(stderr, "Vectors dataset dimensions incorrect (grid index = 0)\n");
    result = 0;
  }
  
  H5Sclose(space_id);
  H5Dclose(vecs_dset_id);
  
  
  /* A grid without a scalar label, therefore only 3 components to each vector */
  new_index = ch5s_vector_create_grid(file, 20, 30, 2., 0.01, points[0], "Lister", NULL, NULL, NULL, NULL);
  if (new_index < 0) {
    fprintf(stderr, "Error in create function.\n");
    result = 0;
  }
  
  exists = H5Lexists(main_group_id, "Grid000001", H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find grid (index = 1)\n");
    result = 0;
  }
  
  grid_id = H5Gopen(main_group_id, "Grid000001", H5P_DEFAULT);
  if (grid_id < 0) {
    fprintf(stderr, "Error opening grid (index = 1)\n");
    result = 0;
  }
  
  exists = H5Lexists(grid_id, CH5_VECS_VECTORS_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find vectors dataset (grid index = 0)\n");
    result = 0;
  }
  
  vecs_dset_id = H5Dopen(grid_id, CH5_VECS_VECTORS_NAME, H5P_DEFAULT);
  if (points_dset_id < 0) {
    fprintf(stderr, "Error opening vectors dataset (grid index = 0)\n");
    result = 0;
  }
  
  space_id = H5Dget_space(vecs_dset_id);
  hsize_t vecs2_dims[3];
  hsize_t vecs2_expected_dims[3] = { 30, 20, 3 };
  H5Sget_simple_extent_dims(space_id, vecs2_dims, NULL);
  
  if (!int_arrays_same((int*)vecs2_expected_dims, (int*)vecs2_dims, 3)) {
    fprintf(stderr, "Vectors dataset dimensions incorrect (grid index = 1)\n");
    result = 0;
  }
  
  fixture_free_points(points);
  
  H5Sclose(space_id);
  H5Dclose(vecs_dset_id);
  H5Gclose(grid_id);
  H5Gclose(main_group_id);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int s_vect_gets_info() {
  hid_t file = fixture_create_vectors_hdf_file();
  int result0, result1, result2, free_results;
  
  ch5s_vector_grid expected_0 = { 6, 1.25, 4, 4, "Moss", "Roy", "seconds", "cm^2", "DERP" };
  ch5s_vector_grid info_0;
  result0 = (ch5s_vector_grid_info(file, 0, &info_0) == 0);
  if (result0 == 0) fprintf(stderr, "Function failed on n=0\n");
  else if (_s_vect_compare_info(0, &expected_0, &info_0) == 0) result0 = 0;
  
  ch5s_vector_grid expected_1 = { 2, 0.01, 3, 3, "Lister", NULL, NULL, NULL, "HAI" };
  ch5s_vector_grid info_1;
  result1 = (ch5s_vector_grid_info(file, 1, &info_1) == 0);
  if (result1 == 0) fprintf(stderr, "Function failed on n=1\n");
  else if (_s_vect_compare_info(1, &expected_1, &info_1) == 0) result1 = 0;
  
  ch5s_vector_grid expected_2 = { 3, 2.00, 2, 4, NULL, "Rimmer", "XYZ", "hours", NULL };
  ch5s_vector_grid info_2;
  result2 = (ch5s_vector_grid_info(file, 2, &info_2) == 0);
  if (result2 == 0) fprintf(stderr, "Function failed on n=2\n");
  else if (_s_vect_compare_info(2, &expected_2, &info_2) == 0) result2 = 0;
  
  fixture_close_vectors_hdf_file(file);
  
  free_results  = (ch5s_vector_free_grid_info(&info_0) == 0);
  free_results &= (ch5s_vector_free_grid_info(&info_1) == 0);
  free_results &= (ch5s_vector_free_grid_info(&info_2) == 0);
  
  return result0 && result1 && result2 && free_results;
}

int _s_vect_compare_info(int n, ch5s_vector_grid *expected, ch5s_vector_grid *given) {
  int result = 1;
  char buf[50];
  sprintf(buf, "Vector grid (n=%d)", n);
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
  if (expected->num_vectors != given->num_vectors) {
    fprintf(stderr, "%s mismatch: num_vectors: expected %d but given %d\n",
      buf, expected->num_vectors, given->num_vectors);
    result = 0;
  }
  if (expected->num_components != given->num_components) {
    fprintf(stderr, "%s mismatch: num_components: expected %d but given %d\n",
      buf, expected->num_components, given->num_components);
    result = 0;
  }
  result &= compare_strings(buf, "label",        expected->label,        given->label);
  result &= compare_strings(buf, "scalar_label", expected->scalar_label, given->scalar_label);
  result &= compare_strings(buf, "time_units",   expected->time_units,   given->time_units);
  result &= compare_strings(buf, "units",        expected->units,        given->units);
  result &= compare_strings(buf, "comments",     expected->comments,     given->comments);
  
  return result;
}

int s_vect_counts_grids() {
  hid_t file = fixture_create_vectors_hdf_file();
  
  int count = ch5s_vector_grid_count(file);
  int result = (count == 3);
  if (result == 0) fprintf(stderr, "Expected %d but given %d\n", 3, count);
  
  fixture_close_vectors_hdf_file(file);
  
  return result;
}

int s_vect_writes_vectors() {
  OPEN_WRITE_TEST_FILE;
  
  int result = 1;
  float **points = fixture_points(3);
  int grid_index = ch5s_vector_create_grid(file, 3, 3, 2., 1.0, points[0], NULL, "OHAI", NULL, NULL, NULL);
  fixture_free_points(points);
  if (grid_index < 0) {
    fprintf(stderr, "Error creating grid\n");
    result = 0;
  }
  
  float expected[36] = {
    1.0, 2.0, 3.0, 4.0,    5.0, 6.0, 7.0, 8.0,    9.0, 1.5, 2.5, 3.5,
    4.0, 3.0, 2.0, 1.0,    8.0, 7.0, 6.0, 5.0,    3.5, 2.5, 1.5, 9.0,
    7.0, 1.0, 9.0, 3.0,    6.0, 4.0, 2.0, 0.0,    9.5, 8.0, 5.5, 1.5
  };
  
  int status = 1;
  status &= (ch5s_vector_write(file, grid_index, 0, 0, (expected +   0)) == 0);
  status &= (ch5s_vector_write(file, grid_index, 1, 2, (expected +  12)) == 0);
  if (status == 0) {
    fprintf(stderr, "Error writing data\n");
    result = 0;
  }
  
  hid_t grid_id = H5Gopen(file, CH5_VECS_GROUP_PATH "/Grid000000", H5P_DEFAULT);
  if (grid_id < 0) {
    fprintf(stderr, "Error opening grid\n");
    result = 0;
  }
  
  hid_t dset_id = H5Dopen(grid_id, CH5_VECS_VECTORS_NAME, H5P_DEFAULT);
  H5Gclose(grid_id);
  if (dset_id < 0) {
    fprintf(stderr, "Error opening vectors dataset\n");
    result = 0;
  }
  
  float read_data[3 * 3 * 4];
  herr_t read_status = H5Dread(dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_data);
  H5Dclose(dset_id);
  if (read_status < 0) {
    fprintf(stderr, "Vector grid read failed\n");
    result = 0;
  }
  
  if (float_arrays_same(expected, read_data, 3 * 3 * 4) == 0) {
    fprintf(stderr, "Read-back vectors data incorrect\n");
    result = 0;
  }
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int s_vect_error_writing_bad_time_ranges() {
  hid_t file = fixture_create_vectors_hdf_file();
  
  float dummy[50];
  int i = 0;
  while (1) {
    if ((++i && (ch5s_vector_write(file, 0, -1,  0, dummy) == 0))  ||
        (++i && (ch5s_vector_write(file, 0,  0, -1, dummy) == 0))  ||
        (++i && (ch5s_vector_write(file, 0,  2,  1, dummy) == 0))  ||
        (++i && (ch5s_vector_write(file, 0, 50, 50, dummy) == 0))  ||
        (++i && (ch5s_vector_write(file, 0,  0, 50, dummy) == 0)))
       { break; }
    else {
      i = 0;
      break;
    }
  }
  
  fixture_close_vectors_hdf_file(file);
  
  if (i != 0) {
    fprintf(stderr, "Incorrect result on attempt #%d\n", i);
    return 0;
  }
  else return 1;
}

int s_vect_reads_vectors() {
  int result = 1;
  hid_t file = fixture_create_vectors_hdf_file();
  
  for (int i = 0; i < 3; i++)
    result &= _s_vect_read_and_check_vecs(file, i);
  
  fixture_close_vectors_hdf_file(file);
  
  return result;
}

int _s_vect_read_and_check_vecs(hid_t file, int grid_index) {
  int result = 1;
  ch5s_vector_grid info;
  if (ch5s_vector_grid_info(file, grid_index, &info)) {
    fprintf(stderr, "Error getting info for grid %d\n", grid_index);
    result = 0;
  }
  ch5s_vector_free_grid_info(&info);
  
  int vector_size = info.num_vectors * info.num_components;
  
  float *expected_v = fixture_vectors(info.num_vectors, info.time_steps, info.num_components);
  float read_v[vector_size];
  
  for (int i = 0; i < info.time_steps; i++) {
    int status = ch5s_vector_read(file, grid_index, i, i, read_v);
    if (status == 1) {
      fprintf(stderr, "Read function for vectors failed (grid %d, time %d)\n", grid_index, i);
      result = 0;
    }
    else if (float_arrays_same(&expected_v[i*vector_size], read_v, vector_size) == 0) {
      fprintf(stderr, "Read-back vectors incorrect (grid %d, time %d)\n", grid_index, i);
      result = 0;
    }
  }
  fixture_free_vectors(expected_v);
  
  return result;
}

int s_vect_reads_points() {
  int result = 1;
  hid_t file = fixture_create_vectors_hdf_file();
  
  float **expected_points = fixture_points(4);
  float read_points[12];
  int status = ch5s_vector_read_points(file, 0, read_points);
  if (status != 0) {
    fprintf(stderr, "Read function for points failed (grid 0)\n");
    result = 0;
  }
  else if (!float_arrays_same(expected_points[0], read_points, 12)) {
    fprintf(stderr, "Read-back points incorrect (grid 0)\n");
    result = 0;
  }
  
  fixture_free_points(expected_points);
  fixture_close_vectors_hdf_file(file);
  
  return result;
}

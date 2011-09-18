#include "test_points.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <stdlib.h>
#include <H5LTpublic.h>

int pnts_writes_all_data() {
  OPEN_WRITE_TEST_FILE;
  float **points = fixture_points(3);
  
  int result = (ch5m_pnts_set_all(file, 3, points[0]) == 0);
  result &= _pnts_read_all_and_assert_equal(file, 3, points[0]);
  
  fixture_free_points(points);
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int pnts_resizes_on_write_all() {
  int num_points_order[2] = { 2, 4 };
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  /* Write initial set of 3 points */
  float **init_points = fixture_points(3);
  result = (ch5m_pnts_set_all(file, 3, init_points[0]) == 0);
  if (result == 0) fprintf(stderr, "Could not write points\n");
  fixture_free_points(init_points);
  
  /* Now write a set of 2 and one of 4 and check for proper resizing */
  for (int i = 0; i < 2; i++) {
    int n = num_points_order[i];
    float **points = fixture_points(n);
    
    result &= (ch5m_pnts_set_all(file, n, points[0]) == 0);
    if (result == 0) fprintf(stderr, "Could not write points\n");
    
    result &= _pnts_read_all_and_assert_equal(file, n, points[0]);

    fixture_free_points(points);
  }
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int pnts_writes_range() {
  return -1;
}

int pnts_reads_info() {
  hid_t file = fixture_create_points_hdf_file();
  ch5_dataset info;
  int result = (ch5m_pnts_get_info(file, &info) == 0);
  fixture_close_points_hdf_file(file);
  return (result == 1) && (info.count == 10) && (info.width == 3);
}

int pnts_bad_info_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  ch5_dataset info;
  int result = ch5m_pnts_get_info(file, &info);
  CLOSE_TEST_FILE;
  return result == 1;
}

int pnts_reads_all_data() {
  hid_t file = fixture_create_points_hdf_file();
  
  ch5_dataset info;
  int result = (ch5m_pnts_get_info(file, &info) == 0);
  if (result == 0) {
    fixture_close_points_hdf_file(file);
    return 0;
  }
  
  float *data = (float*) malloc(sizeof(float) * info.width * info.count);
  result = (ch5m_pnts_get_all(file, data) == 0);
  float **expected = fixture_points(10);
  
  int data_correct = float_arrays_same(data, expected[0], 30);
  
  free(data);
  fixture_free_points(expected);
  fixture_close_points_hdf_file(file);
  
  return (result == 1) && (data_correct == 1);
}

int pnts_bad_data_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  float **data = alloc_2d_float(3, 10);
  int result = (ch5m_pnts_get_all(file, data[0]) == 0);
  free_2d_float(data);
  CLOSE_TEST_FILE;
  return (result == 0);
}

int pnts_reads_range() {
  hid_t file = fixture_create_points_hdf_file();
  
  float *data = (float*) malloc(sizeof(float) * 3 * 3);
  int result = (ch5m_pnts_get_range(file, 5, 7, data) == 0);
  float **expected = fixture_points(10);
  
  /* Only check from the 5th row for 3 points worth of data (3 coords per) */
  int data_correct = float_arrays_same(data, expected[5], 9);
  
  free(data);
  fixture_free_points(expected);
  fixture_close_points_hdf_file(file);
  
  return (result == 1) && (data_correct == 1);
}

int _pnts_read_all_and_assert_equal(hid_t file, int count, float *expected) {
  int result = 1;
  
  ch5_dataset dset_info;
  result &= (ch5_gnrc_get_dset_info(file, CH5_POINTS_FULL_PATH, &dset_info) == 0);
  if ((dset_info.count != count) || (dset_info.width != 3)) {
    fprintf(stderr, "Points dataset dimensions incorrect\n");
    result = 0;
  }
  
  hid_t dset_id = H5Dopen(file, CH5_POINTS_FULL_PATH, H5P_DEFAULT);
  if (dset_id < 0) {
    fprintf(stderr, "Points dataset not found\n");
    result = 0;
  }
  
  float read_points[count * 3];
  herr_t status = H5Dread(dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_points);
  if (status < 0) {
    fprintf(stderr, "Points dataset read failed\n");
    result = 0;
  }
  
  if (float_arrays_same(expected, read_points, count * 3) == 0) {
    fprintf(stderr, "Points data incorrect\n");
    result = 0;
  }
  
  H5Dclose(dset_id);
  
  return result;
}
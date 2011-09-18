#include "test_lons.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <stdlib.h>
#include <H5LTpublic.h>

int lons_writes_all() {
  int result = 1;
  
  for (int order = 1; order <= 2; order++) {
    OPEN_WRITE_TEST_FILE;
    float **lons = fixture_lons(5, order);
  
    result &= (ch5m_lons_set_all(file, 5, order, lons[0]) == 0);
    result &= _lons_read_all_and_assert_equal(file, 5, order, lons[0]);
    
    fixture_free_lons(lons);
    CLOSE_WRITE_TEST_FILE;
  }
  
  return result;
}

int lons_resizes_on_write_all() {
  int num_lons[2]  = { 2, 4 };
  int num_order[2] = { 2, 1 };
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  /* Write initial set of 3 lons with order 1 */
  float **init_lons = fixture_lons(3, 1);
  result = (ch5m_lons_set_all(file, 3, 1, init_lons[0]) == 0);
  if (result == 0) fprintf(stderr, "Could not write lons\n");
  fixture_free_lons(init_lons);
  
  /* Now write a set of 2 and one of 4 and check for proper resizing */
  for (int i = 0; i < 2; i++) {
    int n = num_lons[i];
    int order = num_order[i];
    
    float **lons = fixture_lons(n, order);
    result &= (ch5m_lons_set_all(file, n, order, lons[0]) == 0);
    if (result == 0) fprintf(stderr, "Could not write lons\n");
    
    result &= _lons_read_all_and_assert_equal(file, n, order, lons[0]);
    
    fixture_free_points(lons);
  }
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int lons_writes_range() {
  return -1;
}

int lons_reads_info() {
  hid_t file = fixture_create_lons_hdf_file(1);
  ch5_dataset info;
  int result = (ch5m_lons_get_info(file, &info) == 0);
  fixture_close_lons_hdf_file(file);
  return (result == 1) && (info.count == 5) && (info.width == 3);
}

int lons_bad_info_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  ch5_dataset info;
  int result = (ch5m_lons_get_info(file, &info) == 0);
  CLOSE_TEST_FILE;
  return (result == 0);
}

int lons_reads_order() {
  hid_t file = fixture_create_lons_hdf_file(1);
  int order1 = ch5m_lons_get_order(file);
  fixture_close_lons_hdf_file(file);
  
  file = fixture_create_lons_hdf_file(2);
  int order2 = ch5m_lons_get_order(file);
  fixture_close_lons_hdf_file(file);
  
  return (order1 == 1) && (order2 == 2);
}

int lons_bad_order_returns_negone() {
  OPEN_INVALID_TEST_FILE;
  int order = ch5m_lons_get_order(file);
  CLOSE_TEST_FILE;
  return (order == -1);
}

int lons_reads_all_data() {
  hid_t file = fixture_create_lons_hdf_file(1);
  float **expected = fixture_lons(5, 1);
  float **read_lons = alloc_2d_float(3, 5);
  
  int result = (ch5m_lons_get_all(file, read_lons[0]) == 0);
  result &= float_arrays_same(expected[0], read_lons[0], 15);
  
  fixture_free_lons(expected);
  free_2d_float(read_lons);
  fixture_close_lons_hdf_file(file);
  
  return result;
}

int lons_reads_range() {
  hid_t file = fixture_create_lons_hdf_file(1);
  float **expected = fixture_lons(5, 1);
  float **read_lons = alloc_2d_float(3, 3);
  
  int result = (ch5m_lons_get_range(file, 2, 4, read_lons[0]) == 0);
  result &= float_arrays_same(expected[2], read_lons[0], 9);
  
  fixture_free_lons(expected);
  free_2d_float(read_lons);
  fixture_close_lons_hdf_file(file);
  
  return result;
}

int _lons_read_all_and_assert_equal(hid_t file, int count, int order,
  float *expected)
{
  int result = 1;
  
  hid_t group_id = H5Gopen(file, CH5_LONS_GROUP_NAME, H5P_DEFAULT);
  if (group_id < 0) {
    fprintf(stderr, "Lons containing group not found\n");
    result = 0;
  }
  
  hid_t dset_id = H5Dopen(group_id, CH5_LONS_DSET_NAME, H5P_DEFAULT);
  if (dset_id < 0) {
    fprintf(stderr, "Lons dataset not found\n");
    result = 0;
  }

  ch5_dataset dset_info;
  result &= (ch5_gnrc_get_dset_info(group_id, CH5_LONS_DSET_NAME, &dset_info) == 0);
  if ((dset_info.count != count) || (dset_info.width != order * 3)) {
    fprintf(stderr, "Lons dataset dimensions incorrect (order %d)\n", order);
    result = 0;
  }
  
  float *read_lons = (float*) malloc(sizeof(float) * dset_info.count * dset_info.width);
  herr_t status = H5Dread(dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_lons);
  if (status < 0) {
    fprintf(stderr, "Lons dataset read failed\n");
    result = 0;
  }
  
  if (float_arrays_same(read_lons, expected, count * order * 3) == 0) {
    fprintf(stderr, "Points data incorrect\n");
    result = 0;
  }
  
  free(read_lons);
  
  return result;
}

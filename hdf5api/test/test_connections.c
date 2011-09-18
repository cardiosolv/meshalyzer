#include "test_connections.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <H5LTpublic.h>

int conn_writes_all() {
  OPEN_WRITE_TEST_FILE;
  int **connections = fixture_connections(3);
  
  int result = (ch5m_conn_set_all(file, 3, connections[0]) == 0);
  result &= _conn_read_all_and_assert_equal(file, 3, connections[0]);
  
  fixture_free_connections(connections);
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int conn_resizes_on_write_all() {
  int num_cnnx[2] = { 2, 4 };
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  /* Write initial set of 3 points */
  int **init_cnnx = fixture_connections(3);
  result = (ch5m_conn_set_all(file, 3, init_cnnx[0]) == 0);
  if (result == 0) fprintf(stderr, "Could not write initial connections\n");
  fixture_free_connections(init_cnnx);
  
  /* Now write a set of 2 and one of 4 and check for proper resizing */
  for (int i = 0; i < 2; i++) {
    int n = num_cnnx[i];
    int **cnnx = fixture_connections(n);
    
    result &= (ch5m_conn_set_all(file, n, cnnx[0]) == 0);
    if (result == 0) fprintf(stderr, "Could not write connections\n");
    result &= _conn_read_all_and_assert_equal(file, n, cnnx[0]);
    
    fixture_free_connections(cnnx);
  }
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int conn_writes_range() {
  return -1;
}

int conn_reads_info() {
  hid_t file = fixture_create_connections_hdf_file();
  ch5_dataset info;
  int result = (ch5m_conn_get_info(file, &info) == 0);
  fixture_close_connections_hdf_file(file);
  return (result == 1) && (info.count == 10) && (info.width == 2);
}

int conn_bad_info_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  ch5_dataset info;
  int result = (ch5m_conn_get_info(file, &info) == 0);
  CLOSE_TEST_FILE;
  return result == 0;
}

int conn_reads_all_data() {
  hid_t file = fixture_create_connections_hdf_file();
  int **expected = fixture_connections(10);
  int data[20];
  
  int result = (ch5m_conn_get_all(file, data) == 0);
  result &= int_arrays_same(data, expected[0], 20);
  
  fixture_free_connections(expected);
  fixture_close_connections_hdf_file(file);
  
  return result;
}

int conn_bad_data_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  int data[6];
  int result = (ch5m_conn_get_all(file, data) == 0);
  CLOSE_TEST_FILE;
  return (result == 0);
}

int conn_reads_range() {
  return -1;
}

int _conn_read_all_and_assert_equal(hid_t file, int count, int *expected) {
  int result = 1;
  
  ch5_dataset dset_info;
  result &= (ch5_gnrc_get_dset_info(file, CH5_CONNECTIONS_FULL_PATH, &dset_info) == 0);
  if ((dset_info.count != count) || (dset_info.width != 2)) {
    fprintf(stderr, "Connections dataset dimensions incorrect\n");
    result = 0;
  }
  
  hid_t dset_id = H5Dopen(file, CH5_CONNECTIONS_FULL_PATH, H5P_DEFAULT);
  if (dset_id < 0) {
    fprintf(stderr, "Connections dataset not found\n");
    result = 0;
  }
  
  int read[count * 2];
  herr_t status = H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read);
  if (status < 0) {
    fprintf(stderr, "Connections dataset read failed\n");
    result = 0;
  }
  
  if (int_arrays_same(read, expected, count * 2) == 0) {
    fprintf(stderr, "Connections data incorrect\n");
    result = 0;
  }
  
  H5Dclose(dset_id);
  
  return result;
}
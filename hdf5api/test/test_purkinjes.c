#include "test_purkinjes.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <math.h>
#include <stdlib.h>
#include <H5LTpublic.h>

int pkje_data_comparisson() {
  ch5m_pkje_cable *set1 = fixture_purkinje_cables();
  ch5m_pkje_cable *set2 = fixture_purkinje_cables();
  
  int result = (_ch5m_pkje_compare_cable_info(&set1[0], &set1[1]) == 0) &&
               (_ch5m_pkje_compare_cable_info(&set1[0], &set1[0]) == 1) &&
               (_ch5m_pkje_compare_cable_info(&set1[0], &set2[0]) == 1);
  
  fixture_free_purkinje_cables(set1);
  fixture_free_purkinje_cables(set2);
  
  return result;
}

int pkje_reads_info() {
  hid_t file = fixture_create_purkinje_hdf_file();
  ch5_dataset info;
  int result = (ch5m_pkje_get_info(file, &info) == 0);
  fixture_close_purkinje_hdf_file(file);
  return (result == 1) && (info.count == 2) && (info.width == -1);
}

int pkje_reads_all() {
  hid_t file = fixture_create_purkinje_hdf_file();
  
  ch5_dataset info;
  ch5m_pkje_get_info(file, &info);
  
  ch5m_pkje_cable *cables = (ch5m_pkje_cable*) malloc(sizeof(ch5m_pkje_cable) * info.count);
  int result = (ch5m_pkje_get_all(file, cables) == 0);
  
  ch5m_pkje_cable *test_cables = fixture_purkinje_cables();
  
  /* Check info */
  result &= _ch5m_pkje_compare_cable_info(&test_cables[0], &cables[0]);
  result &= _ch5m_pkje_compare_cable_info(&test_cables[1], &cables[1]);
  
  /* Check nodes */
  result &= double_arrays_same(test_cables[0].nodes[0], cables[0].nodes[0], cables[0].node_count);
  result &= double_arrays_same(test_cables[1].nodes[0], cables[1].nodes[0], cables[1].node_count);
  
  /* Clean up */
  ch5m_pkje_free_cable_set(cables);
  fixture_free_purkinje_cables(test_cables);
  fixture_close_purkinje_hdf_file(file);
  return (result == 1);
}

int pkje_read_bad_file_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  
  ch5_dataset info;
  ch5m_pkje_get_info(file, &info);
  
  ch5m_pkje_cable *cables = (ch5m_pkje_cable*) malloc(sizeof(ch5m_pkje_cable) * info.count);
  int result = (ch5m_pkje_get_all(file, cables) == 0);
  free(cables);
  
  CLOSE_TEST_FILE;
  return (result == 0);
}

int pkje_writes_all() {
  ch5m_pkje_cable *cables = fixture_purkinje_cables();
  
  OPEN_WRITE_TEST_FILE;
  int result = (ch5m_pkje_set_all(file, 2, cables) == 0);
  if (result == 0) fprintf(stderr, "Error in write function.\n");
  
  /*
  * Check write results with native HDF functions (both tedious and laborious
  * but necessary to validate written data).  Getting info, however, can
  * rely on the generic dataset info functions due to their simplicity.
  */
  // Check Info - count should only be 2
  ch5_dataset dset_info;
  result &= (ch5_gnrc_get_dset_info(file, CH5_PKJE_INFO_DSET_FULL_PATH, &dset_info) == 0);
  if (dset_info.count != 2) {
    fprintf(stderr, "Info count incorrect - expected %d but got %d\n", 2, dset_info.count);
    result = 0;
  }
  
  // Read from info dataset and compare read-in data with defined cables above
  ch5m_pkje_cable read_info[2];
  hid_t dset = H5Dopen(file, CH5_PKJE_INFO_DSET_FULL_PATH, H5P_DEFAULT);
  if (dset < 0) {
    fprintf(stderr, "Info dataset not found\n");
    result = 0;
  }
  hid_t memtype = _ch5m_pkje_create_info_compound_type(0);
  int status = H5Dread(dset, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_info);
  if (status < 0) {
    fprintf(stderr, "Info dataset read failed\n");
    result = 0;
  }
  if (_ch5m_pkje_compare_cable_info(&read_info[0], &cables[0]) != 1) {
    fprintf(stderr, "Info0 read back is incorrect\n");
    result = 0;
  }
  if (_ch5m_pkje_compare_cable_info(&read_info[1], &cables[1]) != 1) {
    fprintf(stderr, "Info1 read back is incorrect\n");
    result = 0;
  }
  
  H5Tclose(memtype);
  H5Dclose(dset);
  
  // Read from offsets dataset and check values
  dset_info.count = 0;
  result &= (ch5_gnrc_get_dset_info(file, CH5_PKJE_OFFSETS_DSET_FULL_PATH, &dset_info) == 0);
  if (dset_info.count != 2) {
    fprintf(stderr, "Offsets count incorrect\n");
    result = 0;
  }
  
  dset = H5Dopen(file, CH5_PKJE_OFFSETS_DSET_FULL_PATH, H5P_DEFAULT);
  if (dset < 0) {
    fprintf(stderr, "Offsets dataset not found\n");
    result = 0;
  }
  
  int offsets[2];
  H5Dread(dset, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, offsets);
  if ((offsets[0] != 0) || (offsets[1] != 2)) {
    fprintf(stderr, "Offset data incorrect\n");
    result = 0;
  }
  
  H5Dclose(dset);
  
  // Read from nodes dataset and check values
  dset_info.count = 0;
  dset_info.width = 0;
  result &= (ch5_gnrc_get_dset_info(file, CH5_PKJE_NODES_DSET_FULL_PATH, &dset_info) == 0);
  if ((dset_info.count != 5) && (dset_info.width != 3)) {
    fprintf(stderr, "Nodes dataset dimensions incorrect\n");
    result = 0;
  }
  
  dset = H5Dopen(file, CH5_PKJE_NODES_DSET_FULL_PATH, H5P_DEFAULT);
  if (dset < 0) {
    fprintf(stderr, "Nodes dataset not found\n");
    result = 0;
  }
  
  double read_nodes[15];
  H5Dread(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_nodes);
  if (double_arrays_same(read_nodes, cables[0].nodes[0], 15) != 1) {
    fprintf(stderr, "Nodes data incorrect\n");
    result = 0;
  }
  
  H5Dclose(dset);
  
  fixture_free_purkinje_cables(cables);
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

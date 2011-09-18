#include "test_cables.h"
#include "suite_fixtures.h"
#include "suite_utils.h"
#include "ch5.h"

int cabl_writes_all() {
  OPEN_WRITE_TEST_FILE;
  int *cables = fixture_cables(5);
  
  int result = (ch5m_cabl_set_all(file, 5, cables) == 0);
  
  htri_t exists = H5Lexists(file, CH5_CABL_GROUP_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find cables main group\n");
    result = 0;
  }
  
  hid_t main_group_id = H5Gopen(file, CH5_CABL_GROUP_NAME, H5P_DEFAULT);
  exists = H5Lexists(main_group_id, CH5_CABL_DSET_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find cables dataset\n");
    result = 0;
  }
  
  hid_t dset_id = H5Dopen(main_group_id, CH5_CABL_DSET_NAME, H5P_DEFAULT);
  if (dset_id < 0) {
    fprintf(stderr, "Error opening cables dataset\n");
    result = 0;
  }
  
  ch5_dataset dset_info;
  result &= (ch5_gnrc_get_dset_info(file, CH5_CABL_DSET_FULL_PATH, &dset_info) == 0);
  if ((dset_info.count != 5) || (dset_info.width != 1)) {
    fprintf(stderr, "Cables dataset dimensions incorrect (%d,%d)\n",
      dset_info.count, dset_info.width);
    result = 0;
  }
  
  int read_cables[5];
  herr_t status = H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_cables);
  if (status < 0) {
    fprintf(stderr, "Cables reading failed\n");
    result = 0;
  }
  
  if (int_arrays_same(cables, read_cables, 5) == 0) {
    fprintf(stderr, "Cables data incorrect\n");
    result = 0;
  }
  
  H5Dclose(dset_id);
  H5Gclose(main_group_id);
  fixture_free_cables(cables);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int cabl_reads_info() {
  hid_t file = fixture_create_cables_hdf_file();
  ch5_dataset dset_info;
  int result = (ch5m_cabl_get_info(file, &dset_info) == 0);
  if ((dset_info.count != 10) || (dset_info.width != 1)) {
    fprintf(stderr, "Cables dataset dimensions incorrect (%d,%d)\n",
      dset_info.count, dset_info.width);
    result = 0;
  }
  fixture_close_cables_hdf_file(file);
  return result;
}

int cabl_reads_all() {
  hid_t file = fixture_create_cables_hdf_file();
  int *cables = fixture_cables(10);
  int read_cables[10];
  int result = (ch5m_cabl_get_all(file, read_cables) == 0);
  if (int_arrays_same(cables, read_cables, 10) == 0) {
    fprintf(stderr, "Cables data incorrect\n");
    result = 0;
  }
  fixture_free_cables(cables);
  fixture_close_cables_hdf_file(file);
  return result;
}

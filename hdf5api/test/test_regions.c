#include "test_regions.h"
#include "suite_fixtures.h"
#include "suite_utils.h"
#include "ch5.h"

int regn_writes_all() {
  OPEN_WRITE_TEST_FILE;
  int **regions = fixture_regions(5);
  
  int result = (ch5m_regn_set_all(file, 5, regions[0]) == 0);
  
  htri_t exists = H5Lexists(file, CH5_REGN_GROUP_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find regions main group\n");
    result = 0;
  }
  
  hid_t main_group_id = H5Gopen(file, CH5_REGN_GROUP_NAME, H5P_DEFAULT);
  exists = H5Lexists(main_group_id, CH5_REGN_DSET_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find regions dataset\n");
    result = 0;
  }
  
  hid_t dset_id = H5Dopen(main_group_id, CH5_REGN_DSET_NAME, H5P_DEFAULT);
  if (dset_id < 0) {
    fprintf(stderr, "Error opening regions dataset\n");
    result = 0;
  }
  
  ch5_dataset dset_info;
  result &= (ch5_gnrc_get_dset_info(file, CH5_REGN_DSET_FULL_PATH, &dset_info) == 0);
  if ((dset_info.count != 5) || (dset_info.width != 2)) {
    fprintf(stderr, "Regions dataset dimensions incorrect (%d,%d)\n",
      dset_info.count, dset_info.width);
    result = 0;
  }
  
  int read_regions[5 * 2];
  herr_t status = H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_regions);
  if (status < 0) {
    fprintf(stderr, "Regions reading failed\n");
    result = 0;
  }
  
  if (int_arrays_same(regions[0], read_regions, 5 * 2) == 0) {
    fprintf(stderr, "Regions data incorrect\n");
    result = 0;
  }
  
  H5Dclose(dset_id);
  H5Gclose(main_group_id);
  fixture_free_regions(regions);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int regn_reads_info() {
  hid_t file = fixture_create_regions_hdf_file();
  ch5_dataset dset_info;
  int result = (ch5m_regn_get_info(file, &dset_info) == 0);
  if ((dset_info.count != 10) || (dset_info.width != 2)) {
    fprintf(stderr, "Regions dataset dimensions incorrect (%d,%d)\n",
      dset_info.count, dset_info.width);
    result = 0;
  }
  fixture_close_regions_hdf_file(file);
  return result;
}

int regn_reads_all() {
  hid_t file = fixture_create_regions_hdf_file();
  int **regions = fixture_regions(10);
  int read_regions[10 * 2];
  int result = (ch5m_regn_get_all(file, read_regions) == 0);
  if (int_arrays_same(regions[0], read_regions, 10 * 2) == 0) {
    fprintf(stderr, "Regions data incorrect\n");
    result = 0;
  }
  fixture_free_regions(regions);
  fixture_close_regions_hdf_file(file);
  return result;
}

#include "test_metadata.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

int meta_writes_version() {
  ch5_version *version = fixture_metadata_version();
  OPEN_WRITE_TEST_FILE;
  
  int result = (ch5_meta_set_version(file, version) == 0);
  
  ch5_version read_version = { -1, -1, -1 };
  hid_t attr_id = H5Aopen_by_name(file, ".", CH5_META_VERSION_NAME,
    H5P_DEFAULT, H5P_DEFAULT);
  if (attr_id < 0) {
    fprintf(stderr, "Version attribute could not be found\n");
    result = 0;
  }
  
  int status = H5Aread(attr_id, H5T_NATIVE_INT, &read_version);
  if (status < 0) {
    fprintf(stderr, "Version attribute read failed\n");
    result = 0;
  }
  
  result &= (read_version.major == version->major) &&
            (read_version.minor == version->minor) &&
            (read_version.patch == version->patch);
  
  H5Aclose(attr_id);
  CLOSE_WRITE_TEST_FILE;
  free(version);
  
  return result;
}

int meta_writes_name() {
  char *name = fixture_metadata_name();
  OPEN_WRITE_TEST_FILE;
  
  int result = (ch5_meta_set_name(file, name) == 0);
  
  hid_t attr_id = H5Aopen_by_name(file, ".", CH5_META_NAME_NAME, H5P_DEFAULT,
    H5P_DEFAULT);
  if (attr_id < 0) {
    fprintf(stderr, "Name attribute could not be found\n");
    result = 0;
  }
  
  hid_t space_id = H5Aget_space(attr_id);
  hid_t type_id = H5Tcopy(H5T_C_S1);
  H5Tset_size(type_id, H5T_VARIABLE);
  
  char **read_name = (char**) calloc(sizeof(char*), 1);
  herr_t status = H5Aread(attr_id, type_id, read_name);
  if (status < 0) {
    fprintf(stderr, "Name attribute could not be read\n");
    result = 0;
  }
  else
    result &= (strcmp(read_name[0], name) == 0);
  
  H5Dvlen_reclaim(type_id, space_id, H5P_DEFAULT, read_name);
  H5Sclose(space_id);
  H5Tclose(type_id);
  H5Aclose(attr_id);
  CLOSE_WRITE_TEST_FILE;
  if (read_name != NULL) free(read_name);
  free(name);
  
  return result;
}

int meta_reads_version() {
  hid_t file = fixture_create_metadata_hdf_file();
  ch5_version *version = fixture_metadata_version();
  ch5_version read_version;
  
  int result = (ch5_meta_get_version(file, &read_version) == 0);
  result &= (read_version.major == version->major) &&
            (read_version.minor == version->minor) &&
            (read_version.patch == version->patch);
  
  if (version != NULL) free(version);
  fixture_close_metadata_hdf_file(file);
  
  return result;
}

int meta_bad_version_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  ch5_version version;
  int result = ch5_meta_get_version(file, &version);
  CLOSE_TEST_FILE;
  return result == 1;
}

int meta_reads_name() {
  hid_t file = fixture_create_metadata_hdf_file();
  char *name = fixture_metadata_name();
  
  char *read_name = NULL;
  int result = (ch5_meta_get_name(file, &read_name) == 0);
  result = (result == 1) && (read_name != NULL) && (strcmp(read_name, name) == 0);
  
  if (name != NULL)      free(name);
  if (read_name != NULL) free(read_name);
  fixture_close_metadata_hdf_file(file);
  
  return result;
}

int meta_bad_name_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  char *name = NULL;
  int result = (ch5_meta_get_name(file, &name) == 0);
  result = (result == 0) && (name == NULL);
  if (name != NULL) free(name);
  CLOSE_TEST_FILE;
  return result;
}

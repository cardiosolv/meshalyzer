#ifndef TEST_GENERICS_H
#define TEST_GENERICS_H

#include "test_generics.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <stdlib.h>
#include <string.h>
#include <hdf5.h>

int sets_type_insensitive_single_attribute() {
  OPEN_WRITE_TEST_FILE;
  hid_t group_id = ch5_gnrc_open_or_create_group(file, "Blort");
  int result = 1;
  herr_t status;
  
  /* FLOAT */
  float f_value = 0.125;
  result = ch5_gnrc_set_attr(group_id, H5T_IEEE_F32LE, "floaty", &f_value);
  
  hid_t attr_id = H5Aopen_by_name(group_id, ".", "floaty", H5P_DEFAULT, H5P_DEFAULT);
  if (attr_id < 0) {
    fprintf(stderr, "(float) Error opening attrinute after creation (not found?).\n");
    result = 0;
  }
  
  float f_readback;
  status = H5Aread(attr_id, H5T_NATIVE_FLOAT, &f_readback);
  if (status < 0) {
    fprintf(stderr, "(float) Error reading from attribute.\n");
    result = 0;
  }
  if (f_readback != f_value) {
    fprintf(stderr, "(float) Read value from attribute should be %f but was %f.\n",
      f_value, f_readback);
    result = 0;
  }
  
  H5Aclose(attr_id);
  
  
  /* INT */
  int i_value = 222;
  result = (ch5_gnrc_set_attr(group_id, H5T_STD_U32LE, "inty", &i_value) == 0);
  
  attr_id = H5Aopen_by_name(group_id, ".", "inty", H5P_DEFAULT, H5P_DEFAULT);
  if (attr_id < 0) {
    fprintf(stderr, "(int) Error opening attribute after creation (not found?).\n");
    result = 0;
  }
  
  int i_readback;
  status = H5Aread(attr_id, H5T_NATIVE_INT, &i_readback);
  if (status < 0) {
    fprintf(stderr, "(int) Error reading from attribute.\n");
    result = 0;
  }
  if (i_readback != i_value) {
    fprintf(stderr, "(int) Read value from attribute should be %d but was %d.\n", i_value, i_readback);
    result = 0;
  }
  
  H5Aclose(attr_id);
  
  
  /* STRING */
  char *s_value = "xyz";
  result = (ch5_gnrc_set_attr(group_id, H5T_C_S1, "chary", s_value) == 0);
  
  attr_id = H5Aopen_by_name(group_id, ".", "chary", H5P_DEFAULT, H5P_DEFAULT);
  if (attr_id < 0) {
    fprintf(stderr, "(char*) Error opening attribute after creation (not found?).\n");
    result = 0;
  }
  
  hid_t dspace_id = H5Aget_space(attr_id);
  
  char **s_readback = (char**) calloc(sizeof(char*), 1);
  hid_t type_id = H5Tcopy(H5T_C_S1);
  H5Tset_size(type_id, H5T_VARIABLE);
  
  status = H5Aread(attr_id, type_id, s_readback);
  if (result < 0) {
    fprintf(stderr, "(char*) Error reading from attribute.\n");
    result = 0;
  }
  
  if (strcmp(s_readback[0], s_value) != 0) {
    fprintf(stderr, "(char*) Read value from attribute should be %s but was %s.\n", s_value, s_readback[0]);
    result = 0;
  }
  
  H5Dvlen_reclaim(type_id, dspace_id, H5P_DEFAULT, s_readback);
  free(s_readback);
  H5Sclose(dspace_id);
  H5Aclose(attr_id);
  H5Gclose(group_id);
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int gets_type_insensitive_single_attribute() {
  int result = 1, status;
  hid_t file = fixture_create_generics_hdf_file();
  hid_t group_id = ch5_gnrc_open_or_create_group(file, "Group");
  
  float f_value;
  status = ch5_gnrc_get_attr(group_id, H5T_NATIVE_FLOAT, "float", &f_value);
  if (status != 0) {
    fprintf(stderr, "(float) Error reading attribute.\n");
    result = 0;
  }
  if (f_value != fixture_generics_float()) {
    fprintf(stderr, "(float) Read value from attribute should be %f but was %f.\n", fixture_generics_float(), f_value);
    result = 0;
  }
  
  int i_value;
  status = ch5_gnrc_get_attr(group_id, H5T_NATIVE_INT, "int", &i_value);
  if (status != 0) {
    fprintf(stderr, "(int) Error reading attribute.\n");
    result = 0;
  }
  if (i_value != fixture_generics_int()) {
    fprintf(stderr, "(int) Read value from attribute should be %d but was %d.\n", fixture_generics_int(), i_value);
    result = 0;
  }
  
  char *c_value = NULL;
  char *c_value_expected = fixture_generics_str();
  status = ch5_gnrc_get_attr(group_id, H5T_C_S1, "char", &c_value);
  if (status != 0) {
    fprintf(stderr, "(char*) Error reading attribute.\n");
    result = 0;
  }
  if (c_value != NULL) {
    if (strcmp(c_value, c_value_expected) != 0) {
      fprintf(stderr, "(char*) Read value from attribute should be %s but was %s.\n", c_value_expected, c_value);
      result = 0;
    }
    free(c_value);
  }
  else result = 0;
  free(c_value_expected);
  
  H5Gclose(group_id);
  fixture_close_generics_hdf_file(file);
  
  return result;
}

int nchild_generates_names() {
  const char *expected = "Prefix000321";
  char *name = ch5_nchild_gen_name("Prefix", 321, NULL);
  int result = 1;
  if (name == NULL) {
    fprintf(stderr, "Returned string was null.\n");
    return 0;
  }
  if (strcmp(name, expected) != 0) {
    fprintf(stderr, "Expected \"%s\" but got \"%s\"\n", expected, name);
    result = 0;
  }
  free(name);
  return result;
}

int nchild_creates_or_opens_container() {
  const char *name = "Ironlung";
  
  OPEN_WRITE_TEST_FILE;
  int result = 1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(file, name);
  if (container_id < 0) {
    fprintf(stderr, "Function call failed.\n");
    result = 0;
  }
  
  hid_t check_id = H5Gopen(file, name, H5P_DEFAULT);
  if (check_id < 0) {
    fprintf(stderr, "Expected to find created container but was not found.\n");
    result = 0;
  }
  if (!hdf_refer_to_same(container_id, check_id)) {
    fprintf(stderr, "Created and fetched group ids do not match.\n");
    result = 0;
  }
  
  /* Reopen and ensure that there are no errors and that the same object is
     referred to */
  hid_t reopen_id = ch5_nchild_create_or_open_container(file, name);
  if (reopen_id < 0) {
    fprintf(stderr, "Failed reopening with function.\n");
    result = 0;
  }
  if (!hdf_refer_to_same(container_id, reopen_id)) {
    fprintf(stderr, "Reopened container does not refer to original.\n");
    result = 0;
  }
  
  H5Gclose(container_id);
  H5Gclose(check_id);
  H5Gclose(reopen_id);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int nchild_counts_children() {
  int result = 1;
  hid_t file = fixture_create_surfaces_hdf_file();
  
  hid_t container_id = ch5_nchild_create_or_open_container(file,
    CH5_SURF_GROUP_PATH);
  if (container_id < 0) {
    fprintf(stderr, "Failed openeing container.\n");
    result = 0;
  }
  
  int count = ch5_nchild_count_children(container_id);
  if (count != 2) {
    fprintf(stderr, "Expected count to be 2 but got %d.\n", count);
    result = 0;
  }
  
  H5Gclose(container_id);
  
  fixture_close_surfaces_hdf_file(file);
  
  return result;
}

#endif

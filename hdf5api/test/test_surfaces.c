#include "test_surfaces.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <stdlib.h>
#include <string.h>

int surf_creates_named_surface() {
  OPEN_WRITE_TEST_FILE;
  char *name = fixture_surface_name(0);
  char *dset_name = ch5_nchild_gen_name(CH5_SURF_CHILD_PREFIX, 0);
  int surf0_idx = ch5m_surf_create(file, name, 5, 8);
  int result = (surf0_idx != -1);
  
  htri_t exists = H5Lexists(file, CH5_SURF_GROUP_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find surfaces main group\n");
    result = 0;
  }
  
  hid_t main_group_id = H5Gopen(file, CH5_SURF_GROUP_NAME, H5P_DEFAULT);
  exists = H5Lexists(main_group_id, dset_name, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find surface dataset\n");
    result = 0;
  }
  
  hid_t dset_id = H5Dopen(main_group_id, dset_name, H5P_DEFAULT);
  if (dset_id < 0) {
    fprintf(stderr, "Error opening surface dataset\n");
    result = 0;
  }
  
  hid_t attr_id = H5Aopen_by_name(dset_id, ".", CH5_SURF_NAME_ATTR,
    H5P_DEFAULT, H5P_DEFAULT);
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
  
  /* Also check that new creates increment the index value */
  int surf1_idx = ch5m_surf_create(file, "SECOND SURFACE", 10, 6);
  if (surf1_idx != 1) result = 0;
  
  H5Dvlen_reclaim(type_id, space_id, H5P_DEFAULT, read_name);
  H5Sclose(space_id);
  H5Tclose(type_id);
  H5Aclose(attr_id);
  H5Dclose(dset_id);
  H5Gclose(main_group_id);
  
  if (read_name != NULL) free(read_name);
  free(name);
  free(dset_name);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int surf_creates_unnamed_surface() {
  return -1;
}

int surf_gets_surface_count() {
  hid_t file = fixture_create_surfaces_hdf_file();
  int count = ch5m_surf_get_count(file);
  fixture_close_surfaces_hdf_file(file);
  return (count == 2);
}

int surf_opens_surface_by_index() {
  hid_t file = fixture_create_surfaces_hdf_file();
  hid_t surf_0, surf_1;
  int result  = (ch5m_surf_open(file, 0, &surf_0) == 0);
      result &= (ch5m_surf_open(file, 1, &surf_1) == 0);
  result = (result != 0) && (surf_0 >= 0) && (surf_1 >= 1) && (surf_0 != surf_1);
  ch5m_surf_close(surf_0);
  ch5m_surf_close(surf_1);
  fixture_close_surfaces_hdf_file(file);
  return result;
}

int surf_reads_name() {
  hid_t file = fixture_create_surfaces_hdf_file();
  int result = 1;
  
  for (int i = 0; i < 2; i++) {
    hid_t surf_id;
    ch5m_surf_open(file, i, &surf_id);
    char *name = NULL;
    result = (ch5m_surf_get_name(surf_id, &name) == 0);
    char *expected = fixture_surface_name(i);
    result &= (name != NULL) && (strcmp(name, expected) == 0);
    if (name != NULL) free(name);
    free(expected);
    ch5m_surf_close(surf_id);
  }
  
  fixture_close_surfaces_hdf_file(file);
  
  return result;
}

int surf_reads_info() {
  hid_t file = fixture_create_surfaces_hdf_file();
  int result = 1;
  
  for (int i = 0; i < 2; i++) {
    hid_t surf_id;
    result &= (ch5m_surf_open(file, i, &surf_id) == 0);
    if (result == 0) break;
    
    ch5_dataset info;
    result &= (ch5m_surf_get_elem_info(surf_id, &info) == 0);
    if (result == 0) {
      ch5m_surf_close(surf_id);
      break;
    }
    
    result &= (info.count == 5) &&
              (info.width == CH5_ELEM_MAX_VERTEX_COUNT + CH5_ELEM_WIDTH_ADD);
    ch5m_surf_close(surf_id);
    if (result == 0) break;
  }
  
  fixture_close_surfaces_hdf_file(file);
  
  return result;
}

int surf_reads_all_elements() {
  hid_t file = fixture_create_surfaces_hdf_file();
  int **expected = fixture_elements(5, 8);
  int result = 1;
  
  for (int i = 0; i < 2; i++) {
    hid_t surf_id;
    result &= (ch5m_surf_open(file, i, &surf_id) == 0);
    if (result == 0) break;
    
    ch5_dataset info;
    result &= (ch5m_surf_get_elem_info(surf_id, &info) == 0);
    if (result == 0) {
      fprintf(stderr, "Could not get surface info\n");
      ch5m_surf_close(surf_id);
      break;
    }
    
    int *read_elems = (int*) malloc(sizeof(int) * info.count * info.width);
    result &= (ch5m_elem_get_all_by_dset(surf_id, read_elems) == 0);
    ch5m_surf_close(surf_id);
    if (result == 0) {
      fprintf(stderr, "Read of elements failed on surface\n");
      free(read_elems);
      break;
    }
    
    result &= int_arrays_same(expected[0], read_elems,
      5 * (CH5_ELEM_MAX_VERTEX_COUNT + CH5_ELEM_WIDTH_ADD));
    
    free(read_elems);
    if (result == 0) break;
  }
  
  fixture_close_surfaces_hdf_file(file);
  fixture_free_elements(expected);
  
  return result;
}

int surf_reads_range_of_elements() {
  return -1;
}

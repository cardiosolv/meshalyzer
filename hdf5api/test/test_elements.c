#include "test_elements.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

static int _elem_read_all_and_assert_equal(hid_t file, int n, int max_width,
  int *expected);

extern char prefixes[CH5_ELEM_PRIMITIVE_COUNT][3];
extern int widths[CH5_ELEM_PRIMITIVE_COUNT];

int elem_hashing_and_width() {
  int result = 1;
  for (int i = 0; i < CH5_ELEM_PRIMITIVE_COUNT; i++) {
    ch5m_element_type elem = ch5m_elem_get_type_by_prefix(prefixes[i]);
    int width = ch5m_elem_get_width_for_type(elem);
    printf("%s:   expected_index:%d given_index:%d   expcected_width:%d given_width:%d\n", prefixes[i], i, (int)elem, widths[i], width);
    if ((int)elem != i) {
      fprintf(stderr, "Hashing mismatch on %s: expected %d but got %d\n", prefixes[i], i, (int)elem);
      result = 0;
    }
    if (width != widths[i]) {
      fprintf(stderr, "Width mismatch on %s: expected %d but got %d\n", prefixes[i], widths[i], width);
      result = 0;
    }
  }
  return result;
}

int elem_writes_all() {
  OPEN_WRITE_TEST_FILE;
  int **elements = fixture_elements(10, 6);
  
  int result = (ch5m_elem_set_all(file, 10, 6, elements[0]) == 0);
  result &= _elem_read_all_and_assert_equal(file, 10, 6, elements[0]);
  
  fixture_free_elements(elements);
  
  CLOSE_WRITE_TEST_FILE;
  return result;
}

int elem_resizes_on_write_all() {
  OPEN_WRITE_TEST_FILE;
  int **elem5_6 = fixture_elements(5, 6);
  int **elem9_4 = fixture_elements(9, 4);
  int **elem7_8 = fixture_elements(7, 8);
  
  int result = (ch5m_elem_set_all(file, 5, 6, elem5_6[0]) == 0);
  result &= (ch5m_elem_set_all(file, 7, 8, elem7_8[0]) == 0);
  result &= _elem_read_all_and_assert_equal(file, 7, 8, elem7_8[0]);
  result &= (ch5m_elem_set_all(file, 9, 4, elem9_4[0]) == 0);
  result &= _elem_read_all_and_assert_equal(file, 9, 4, elem9_4[0]);
  
  fixture_free_elements(elem5_6);
  fixture_free_elements(elem9_4);
  fixture_free_elements(elem7_8);
  
  CLOSE_WRITE_TEST_FILE;
  return result;
}

int elem_writes_range() {
  return -1;
}

int elem_reads_info() {
  hid_t file = fixture_create_elements_hdf_file();
  ch5_dataset info;
  int result = (ch5m_elem_get_info(file, &info) == 0);
  fixture_close_elements_hdf_file(file);
  return (result == 1) &&
         (info.count == 10) &&
         (info.width == CH5_ELEM_MAX_VERTEX_COUNT + CH5_ELEM_WIDTH_ADD);
}

int elem_bad_info_returns_zero() {
  OPEN_INVALID_TEST_FILE;
  ch5_dataset info;
  int result = (ch5m_elem_get_info(file, &info) == 0);
  CLOSE_TEST_FILE;
  return (result == 0);
}

int elem_reads_all() {
  hid_t file = fixture_create_elements_hdf_file();
  int total_ints = 10 * (CH5_ELEM_MAX_VERTEX_COUNT + CH5_ELEM_WIDTH_ADD);
  int read_elems[total_ints];
  int **expected = fixture_elements(10, CH5_ELEM_MAX_VERTEX_COUNT);
  
  int result = (ch5m_elem_get_all(file, read_elems) == 0);
  result &= int_arrays_same(expected[0], read_elems, total_ints);
  
  fixture_free_elements(expected);
  fixture_close_elements_hdf_file(file);
  
  return result;
}

int elem_reads_range() {
  return -1;
}

int _elem_read_all_and_assert_equal(hid_t file, int n, int max_width,
  int *expected)
{
  int result = 1;
  int total_width = max_width + CH5_ELEM_WIDTH_ADD;
  int total_ints = n * total_width;
  
  hid_t dset_id = H5Dopen(file, CH5_ELEM_DSET_FULL_PATH, H5P_DEFAULT);
  if (dset_id < 0) {
    fprintf(stderr, "Elements dataset not found\n");
    result = 0;
  }
  
  ch5_dataset dset_info;
  result &= (ch5_gnrc_get_dset_info(file, CH5_ELEM_DSET_FULL_PATH, &dset_info) == 0);
  if ((dset_info.count != n) || (dset_info.width != total_width)) {
    fprintf(stderr, "Elements dataset dimensions incorrect (%d,%d)\n",
      dset_info.count, dset_info.width);
    result = 0;
  }
  
  int read_elems[total_ints];
  herr_t status = H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, read_elems);
  if (status < 0) {
    fprintf(stderr, "Elements reading failed\n");
    result = 0;
  }
  
  if (int_arrays_same(expected, read_elems, total_ints) == 0) {
    fprintf(stderr, "Elements data incorrect\n");
    result = 0;
  }
  
  H5Dclose(dset_id);
    
  return result;
}

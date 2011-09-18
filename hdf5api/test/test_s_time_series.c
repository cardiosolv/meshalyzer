#include "test_s_time_series.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"
#include <string.h>
#include <stdlib.h>

int s_time_creates_series() {
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  int snum = ch5s_series_create(file, 10, 4, "XXX", "QUANT", "units", "comments");
  if (snum == -1) {
    fprintf(stderr, "Series creation function failed\n");
    result = 0;
  }
  else if (snum != 0) {
    fprintf(stderr, "Series creation returned unexpected value: expected %d but got %d\n", 0, snum);
    result = 0;
  }
  
  htri_t exists = H5Lexists(file, CH5_TIME_GROUP_NAME, H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find time series main group\n");
    result = 0;
  }
  
  hid_t main_group_id = H5Gopen(file, CH5_TIME_GROUP_NAME, H5P_DEFAULT);
  int count = ch5_nchild_count_children(main_group_id);
  if (count != 1) {
    fprintf(stderr, "Child count should be 1 but was %d\n", count);
    result = 0;
  }
  
  exists = H5Lexists(main_group_id, "Series000000", H5P_DEFAULT);
  if (exists < 1) {
    fprintf(stderr, "Could not find series dataset\n");
    result = 0;
  }
  
  hid_t dset_id = H5Dopen(main_group_id, "Series000000", H5P_DEFAULT);
  if (dset_id < 0) {
    fprintf(stderr, "Could not open series dataset\n");
    result = 0;
  }
  
  H5Dclose(dset_id);
  H5Gclose(main_group_id);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int s_time_gets_series_info() {
  int result = 1;
  hid_t file = fixture_create_timeseries_hdf_file();
  
  ch5s_series info;
  int status = ch5s_series_info(file, 0, &info);
  if (status == 1) {
    fprintf(stderr, "Error in function fetching series %d\n", 0);
    result = 0;
  }
  char *expected0_label = "Moderat";
  char *expected0_quant = "QUANT";
  char *expected0_units = "units";
  char *expected0_comnt = "comments";
  ch5s_series expected0 = { 2, 4, expected0_label, expected0_quant, expected0_units, expected0_comnt };
  status = _s_time_compare_info(0, &expected0, &info);
  if (status == 0) result = 0;
  status = ch5s_series_free_info(&info);
  if (status == 1) {
    fprintf(stderr, "Error freeing series %d info\n", 0);
    result = 0;
  }
  
  status = ch5s_series_info(file, 1, &info);
  if (status == 1) {
    fprintf(stderr, "Error in function fetching series %d\n", 1);
    result = 0;
  }
  char *expected1_label = "AphexTwin";
  ch5s_series expected1 = { 3, 3, expected1_label, NULL, NULL, NULL };
  status = _s_time_compare_info(1, &expected1, &info);
  if (status == 0) result = 0;
  status = ch5s_series_free_info(&info);
  if (status == 1) {
    fprintf(stderr, "Error freeing series %d info\n", 1);
    result = 0;
  }
  
  fixture_close_timeseries_hdf_file(file);
  
  return result;
}

int _s_time_compare_info(int n, ch5s_series *expected, ch5s_series *given) {
  int result = 1;
  
  if (expected->time_steps != given->time_steps) {
    fprintf(stderr, "Time series (n=%d) mismatch: time_steps: expected %d but given %d\n",
      n, expected->time_steps, given->time_steps);
    result = 0;
  }
  if (expected->num_values != given->num_values) {
    fprintf(stderr, "Time series (n=%d) mismatch: num_values: expected %d but given %d\n",
      n, expected->num_values, given->num_values);
    result = 0;
  }
  
  result &= _s_time_compare_strings(n, "label",      expected->label,      given->label);
  result &= _s_time_compare_strings(n, "quantities", expected->quantities, given->quantities);
  result &= _s_time_compare_strings(n, "units",      expected->units,      given->units);
  result &= _s_time_compare_strings(n, "comments",   expected->comments,   given->comments);
  
  return result;
}

int _s_time_compare_strings(int n, const char *kind, const char *expected, const char *given) {
  if ((expected == NULL && given != NULL) ||
      (expected != NULL && given == NULL) ||
      (expected != NULL && given != NULL && strcmp(expected, given) != 0)) {
    fprintf(stderr, "Time series (n=%d) mismatch: %s: expected \"%s\" but given \"%s\"\n",
      n, kind, expected, given);
    return 0;
  }
  return 1;
}

int s_time_counts_series() {
  int result = 1;
  
  OPEN_WRITE_TEST_FILE;
  int empty_count = ch5s_series_count(file);
  if (empty_count == -1){
    fprintf(stderr, "Error in function (empty file)\n");
    result = 0;
  }
  else if (empty_count != 0) {
    fprintf(stderr, "Incorrect count on empty file, expected 0 but got %d\n", empty_count);
    result = 0;
  }
  CLOSE_WRITE_TEST_FILE;
  
  hid_t fixfile = fixture_create_timeseries_hdf_file();
  int fix_count = ch5s_series_count(file);
  if (fix_count == -1){
    fprintf(stderr, "Error in function (fixture file)\n");
    result = 0;
  }
  else if (fix_count != 2) {
    fprintf(stderr, "Incorrect count on fixture file, expected 2 but got %d\n", fix_count);
    result = 0;
  }
  fixture_close_timeseries_hdf_file(fixfile);
  
  return result;
}

int s_time_writes_data() {
  int result = 1, status;
  
  OPEN_WRITE_TEST_FILE;
  
  /* WRITE STAGE */
  status = ch5s_series_create(file, 3, 3, NULL, NULL, NULL, NULL);
  if (status == -1) {
    fprintf(stderr, "Error creating series 1\n");
    result = 0;
  }
  status = ch5s_series_create(file, 3, 4, NULL, NULL, NULL, NULL);
  if (status == -1) {
    fprintf(stderr, "Error creating series 2\n");
    result = 0;
  }
  
  double *data1 = fixture_timeseries(3, 4);
  status  = 1;
  status &= (ch5s_series_write(file, 0, 0, 1, data1)         == 0);
  status &= (ch5s_series_write(file, 0, 2, 2, &data1[2 * 4]) == 0);
  if (status == 0) {
    fprintf(stderr, "Error in write data function (series id 0)\n");
    result = 0;
  }
  
  double *data2 = fixture_timeseries(3, 5);
  status  = 1;
  status &= (ch5s_series_write(file, 1, 0, 0, data2)         == 0);
  status &= (ch5s_series_write(file, 1, 1, 2, &data2[1 * 5]) == 0);
  if (status == 0) {
    fprintf(stderr, "Error in write data function (series id 1)\n");
    result = 0;
  }
  
  /* READ-BACK STAGE */
  /* Series 0 */
  hid_t dset1_id = H5Dopen(file, "/time_series/Series000000", H5P_DEFAULT);
  if (dset1_id < 0) {
    fprintf(stderr, "Could not find series dataset (series id 0)\n");
    result = 0;
  }
  
  double *read1 = (double*) malloc(sizeof(double) * 3 * 4);
  herr_t read_status = H5Dread(dset1_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read1);
  if (read_status < 0) {
    fprintf(stderr, "Error reading from dataset (series id 0)\n");
    result = 0;
  }
  
  if (!double_arrays_same(data1, read1, 3 * 4)) {
    fprintf(stderr, "Read-back data does not match (series id 0)\n");
    result = 0;
  }
  
  /* Series 1 */
  hid_t dset2_id = H5Dopen(file, "/time_series/Series000001", H5P_DEFAULT);
  if (dset2_id < 0) {
    fprintf(stderr, "Could not find series dataset (series id 1)\n");
    result = 0;
  }
  
  double *read2 = (double*) malloc(sizeof(double) * 3 * 5);
  read_status = H5Dread(dset2_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, read2);
  if (read_status < 0) {
    fprintf(stderr, "Error reading from dataset (series id 1)\n");
    result = 0;
  }
  
  if (!double_arrays_same(data2, read2, 3 * 5)) {
    fprintf(stderr, "Read-back data does not match (series id 1)\n");
    result = 0;
  }
  
  fixture_free_timeseries(data1);
  fixture_free_timeseries(data2);
  free(read1);
  free(read2);
  H5Dclose(dset1_id);
  H5Dclose(dset2_id);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int s_time_reads_data() {
  int result = 1;
  hid_t file = fixture_create_timeseries_hdf_file();
  
  double *read1    = (double*) malloc(sizeof(double) * 5);
  double *read2    = (double*) malloc(sizeof(double) * 5);
  double *expected = fixture_timeseries(2, 5);
  
  int status = ch5s_series_read(file, 0, 0, 0, read1);
  if (status == 1) {
    fprintf(stderr, "Error in read function\n");
    result = 0;
  }
  else if (!double_arrays_same(expected, read1, 5)) {
    fprintf(stderr, "Read-back data incorrect t=0\n");
    result = 0;
  }
  
  status = ch5s_series_read(file, 0, 1, 1, read2);
  if (status == 1) {
    fprintf(stderr, "Error in read function\n");
    result = 0;
  }
  else if (!double_arrays_same((expected + 5), read2, 5)) {
    fprintf(stderr, "Read-back data incorrect t=1\n");
    result = 0;
  }
  
  free(read1);
  free(read2);
  fixture_free_timeseries(expected);
  fixture_close_timeseries_hdf_file(file);
  
  return result;
}

int s_time_reads_data_by_value() {
  int result = 1;
  hid_t file = fixture_create_timeseries_hdf_file();
  
  double *read1       = (double*) malloc(sizeof(double) * 2 * 1);
  double *read2       = (double*) malloc(sizeof(double) * 2 * 2);
  double expected1[2] = { 0, 5 };
  double expected2[4] = { 2, 3, 7, 8 };
  
  int status = ch5s_series_read_values(file, 0, 0, 0, read1);
  if (status == 1) {
    fprintf(stderr, "Error in read function\n");
    result = 0;
  }
  else if (!double_arrays_same(expected1, read1, 2)) {
    fprintf(stderr, "Read-back data incorrect n=0\n");
    result = 0;
  }
  
  status = ch5s_series_read_values(file, 0, 2, 3, read2);
  if (status == 1) {
    fprintf(stderr, "Error in read function\n");
    result = 0;
  }
  else if (!double_arrays_same(expected2, read2, 4)) {
    fprintf(stderr, "Read-back data incorrect n=2..3\n");
    result = 0;
  }
  
  free(read1);
  free(read2);
  fixture_close_timeseries_hdf_file(file);
  
  return result;
}

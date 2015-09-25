#include "test_text.h"
#include "suite_utils.h"
#include "suite_fixtures.h"
#include "ch5.h"

#include <stdlib.h>
#include <string.h>

int text_creates_new() {
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  int index = ch5_text_create(file, "Label");
  if (index == -1) {
    fprintf(stderr, "Error in create function\n");
    result = 0;
  }
  
  hid_t container_id = H5Gopen(file, CH5_TEXT_GROUP_NAME, H5P_DEFAULT);
  if (container_id < 0) {
    fprintf(stderr, "Error openeing text container\n");
    result = 0;
  }
  
  hid_t text_id = H5Dopen(container_id, CH5_TEXT_FILE_NAME_PREFIX "0", H5P_DEFAULT);
  if (text_id < 0) {
    fprintf(stderr, "Error openeing text file\n");
    result = 0;
  }
  
  H5Dclose(text_id);
  H5Gclose(container_id);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int text_gets_info() {
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  int index = ch5_text_create(file, "Label");
  if (index == -1) {
    fprintf(stderr, "Error in create function\n");
    result = 0;
  }
  
  ch5_text_file expcected_info = { .size=0, .storage=0,.label="Label" };
  ch5_text_file read_info;
  int status = ch5_text_info(file, 0, &read_info);
  if (status != 0) {
    fprintf(stderr, "Error retrieving file info\n");
    result = 0;
  }
  else if (!_text_compare_info(&expcected_info, &read_info)) result = 0;
  else if (ch5_text_free_info(&read_info) != 0) {
    fprintf(stderr, "Error freeing file info\n");
    result = 0;
  }
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int text_appends() {
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  int index = ch5_text_create(file, "Label");
  if (index == -1) {
    fprintf(stderr, "Error in create function\n");
    result = 0;
  }
  
  char *text1         = "ABCD";
  char *text2         = "1234";
  char *expected_text = "ABCD1234";
  int status = ch5_text_append(file, index, text1);
  if (status != 0) {
    fprintf(stderr, "Error first in append function\n");
    result = 0;
  }
  status = ch5_text_append(file, index, text2);
  if (status != 0) {
    fprintf(stderr, "Error second in append function\n");
    result = 0;
  }
  
  hid_t container_id = H5Gopen(file, CH5_TEXT_GROUP_NAME, H5P_DEFAULT);
  if (container_id < 0) {
    fprintf(stderr, "Error openeing text container\n");
    result = 0;
  }
  
  hid_t text_id = H5Dopen(container_id, CH5_TEXT_FILE_NAME_PREFIX "0", H5P_DEFAULT);
  if (text_id < 0) {
    fprintf(stderr, "Error openeing text file\n");
    result = 0;
  }
  
  hsize_t dims[1];
  hid_t space_id = H5Dget_space(text_id);
  status = H5Sget_simple_extent_dims(space_id, dims, NULL);
  if (status < 0) {
    fprintf(stderr, "Error getting text dataset dimensions\n");
    result = 0;
  }
  else if ((unsigned int)dims[0] != strlen(expected_text)) {
    fprintf(stderr, "Wrong dimensions on text dataset, expected %d but got %d\n", (unsigned int)strlen(expected_text), (unsigned int)dims[0]);
    result = 0;
  }
  
  char *read_text = (char*) calloc(sizeof(char), dims[0] + 1);
  status = H5Dread(text_id, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, read_text);
  if (status < 0) {
    fprintf(stderr, "Error reading from text dataset\n");
    result = 0;
  }
  else if (strcmp(expected_text, read_text) != 0) {
    fprintf(stderr, "Incorrect string read-back, expected \"%s\" but got \"%s\"\n", expected_text, read_text);
    result = 0;
  }
  if (read_text != NULL) free(read_text);
  
  H5Sclose(space_id);
  H5Dclose(text_id);
  H5Gclose(container_id);
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

/* Depends on the success of the ch5_text_append method for brevity */
int text_reads() {
  int result = 1;
  OPEN_WRITE_TEST_FILE;
  
  int index = ch5_text_create(file, "Label");
  if (index == -1) {
    fprintf(stderr, "Error in create function\n");
    result = 0;
  }
  
  char *expected_text = "If you optimize everything,\nyou will always be unhappy.";
  int expected_len = strlen(expected_text);
  int status = ch5_text_append(file, index, expected_text);
  if (status != 0) {
    fprintf(stderr, "Error in append function\n");
    result = 0;
  }
  
  char **read_back; 
  status = ch5_text_read(file, index, &read_back);
  if (status != 0) {
    fprintf(stderr, "Error in read function\n");
    result = 0;
  }
  else if (strcmp(expected_text, read_back[0]) != 0) {
    fprintf(stderr, "Read-back mismatch, expected \"%s\" but got \"%s\"\n", expected_text, *read_back);
    result = 0;
  }
  
  if (read_back != NULL) {
    char *ptr = read_back[0];
    while( ptr ) 
      free( ptr++ );
    free(read_back);
  }
  
  CLOSE_WRITE_TEST_FILE;
  
  return result;
}

int _text_compare_info(ch5_text_file *expected, ch5_text_file *given) {
  int result = 1;
  
  if (expected->size != given->size) {
    fprintf(stderr, "Text info mismatch: size: expected %d but given %d\n",
      expected->size, given->size);
    result = 0;
  }
  if (strcmp(expected->label, given->label) != 0) {
    fprintf(stderr, "Text info mismatch: label: expected %s but given %s\n",
      expected->label, given->label);
    result = 0;
  }
  
  return result;
}

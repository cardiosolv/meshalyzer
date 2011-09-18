#include "suite_utils.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <hdf5.h>

#define R_PASS_LABEL  "PASS"
#define R_FAIL_LABEL  "FAIL"
#define R_STUB_LABEL  "STUB"
#define R_PASS_COLOUR "\x1B[1m\x1B[32m"
#define R_FAIL_COLOUR "\x1B[1m\x1B[31m"
#define R_STUB_COLOUR "\x1B[1m\x1B[36m"
#define R_CLEAR_CTRL  "\x1B[0m"

#define R_PASS R_PASS_COLOUR R_PASS_LABEL R_CLEAR_CTRL
#define R_FAIL R_FAIL_COLOUR R_FAIL_LABEL R_CLEAR_CTRL
#define R_STUB R_STUB_COLOUR R_STUB_LABEL R_CLEAR_CTRL

static int passed_tests = 0, stub_tests = 0, failed_tests = 0;

void run_test(char const *name, int (*test_fn)(void)) {
  int result = test_fn();
  switch (result) {
    case 1:
      passed_tests += 1;
      break;
    
    case 0:
      failed_tests += 1;
      break;
    
    case -1:
      stub_tests += 1;
      break;
  }
  print_test_result(name, result);
}

void print_test_result(char const *name, int result) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  
  int name_width = w.ws_col - 3 - 5;
  char *out_name = pad_str_right(name, name_width, '.');
  char *label;
  switch (result) {
  case 0:
    label = R_FAIL;
    break;
  case 1:
    label = R_PASS;
    break;
  case -1:
    label = R_STUB;
    break;
  }
  printf(" + %s %s\n", out_name, label);
  
  free(out_name);
}

void print_result_summary() {
  printf("   %s: %4d\n", R_PASS, passed_tests);
  printf("   %s: %4d\n", R_FAIL, failed_tests);
  printf("   %s: %4d\n", R_STUB, stub_tests);
}

void print_title(char const *text) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int orig_len = strlen(text);
  char *padded = (char*) calloc(sizeof(char), orig_len + 5);
  padded[0] = padded[1] = '=';
  padded[2] = ' ';
  strncpy(&padded[3], text, orig_len);
  padded[orig_len + 3] = ' ';
  char *full_text = pad_str_right(padded, w.ws_col, '=');
  printf("\n\x1B[33m%s\x1B[0m\n", full_text);
  free(padded);
  free(full_text);
}

char* pad_str_right(char const *text, int width, char expand_fill) {
  char *out = (char*) calloc(sizeof(char), width + 1);
  int text_len = strlen(text);
  int copy_len = (text_len < width) ? text_len : width;
  strncpy(out, text, copy_len);
  if (copy_len < width) {
    for (int i = copy_len; i < width; i++) out[i] = expand_fill;
  }
  return out;
}

float** alloc_2d_float(int cols, int rows) {
  float*  data = (float*)  calloc(sizeof(float), cols * rows);
  float** arr  = (float**) malloc(sizeof(float*) * rows);
  for (int i = 0; i < rows; i++) arr[i] = data + (i * cols);
  return arr;
}

void free_2d_float(float **arr) {
  if (arr == NULL) return;
  if (arr[0] != NULL) free(arr[0]);
  free(arr);
}

int** alloc_2d_int(int cols, int rows) {
  int*  data = (int*)  calloc(sizeof(int), cols * rows);
  int** arr  = (int**) malloc(sizeof(int*) * rows);
  for (int i = 0; i < rows; i++) arr[i] = data + (i * cols);
  return arr;
}

void free_2d_int(int **arr) {
  if (arr == NULL) return;
  if (arr[0] != NULL) free(arr[0]);
  free(arr);
}

int float_equality(float *a, float *b, float error) {
  return (fabs(*a - *b) < error);
}

int float_arrays_same(float *a, float *b, int n) {
  for (int i = 0; i < n; i++)
    if (!float_equality(&a[i], &b[i], FLOAT_ERROR)) return 0;
  return 1;
}

int double_equality(double a, double b, double error) {
  return (fabs(a - b) < error);
}

int double_arrays_same(double *a, double *b, int n) {
  for (int i = 0; i < n; i++)
    if (!double_equality(a[i], b[i], DOUBLE_ERROR)) return 0;
  return 1;
}

int int_arrays_same(int *a, int *b, int n) {
  for (int i = 0; i < n; i++)
    if (a[i] != b[i]) return 0;
  return 1;
}

int hdf_refer_to_same(hid_t a, hid_t b) {
  H5O_info_t a_info, b_info;
  H5Oget_info(a, &a_info);
  H5Oget_info(b, &b_info);
  if (a_info.addr == b_info.addr) return 1;
  return 0;
}

int compare_strings(const char *prefix, const char *kind,
  const char *expected, const char *given)
{
  if ((expected == NULL && given != NULL) ||
      (expected != NULL && given == NULL) ||
      (expected != NULL && given != NULL && strcmp(expected, given) != 0)) {
    fprintf(stderr, "%s mismatch: %s: expected \"%s\" but given \"%s\"\n",
      prefix, kind, expected, given);
    return 0;
  }
  return 1;
}

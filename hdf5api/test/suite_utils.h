#ifndef SUITE_UTILS_H
#define SUITE_UTILS_H

#include <hdf5.h>

#define FLOAT_ERROR  0.00001
#define DOUBLE_ERROR 0.0000001

#define VALID_FILE   "test/data/test_valid.h5"
#define INVALID_FILE "test/data/test_invalid.h5"
#define WRITE_FILE   "test/data/test_write.h5"

#define OPEN_TEST_FILE(path) \
hid_t file; \
int _file_result = ch5_open(path, &file); \
if (_file_result != 0) return 0

#define OPEN_INVALID_TEST_FILE OPEN_TEST_FILE(INVALID_FILE)

#define OPEN_WRITE_TEST_FILE \
hid_t file = H5Fcreate(WRITE_FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT); \
if (file < 0) return 0

#define CLOSE_TEST_FILE ch5_close(file)
#define CLOSE_WRITE_TEST_FILE H5Fclose(file); \
remove(WRITE_FILE)

void    run_test(char const *name, int (*test_fn)(void));
void    print_test_result(char const *name, int result);
void    print_result_summary();
void    print_title(char const *text);
char*   pad_str_right(char const *text, int width, char expand_fill);
float** alloc_2d_float(int cols, int rows);
void    free_2d_float(float** arr);
int**   alloc_2d_int(int cols, int rows);
void    free_2d_int(int **arr);
int     float_equality(float *a, float *b, float error);
int     float_arrays_same(float *a, float *b, int n);
int     double_equality(double a, double b, double error);
int     double_arrays_same(double *a, double *b, int n);
int     int_arrays_same(int *a, int *b, int n);
int     hdf_refer_to_same(hid_t a, hid_t b);
int     compare_strings(const char *prefix, const char *kind,
          const char *expected, const char *given);

#endif

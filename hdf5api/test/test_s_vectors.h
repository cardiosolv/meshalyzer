#ifndef TEST_S_VECTORS_H
#define TEST_S_VECTORS_H

#include "types.h"
#include <hdf5.h>

int s_vect_creates_grid();
int s_vect_gets_info();
int s_vect_counts_grids();
int s_vect_writes_vectors();
int s_vect_error_writing_bad_time_ranges();
int s_vect_reads_vectors();
int s_vect_reads_points();

int _s_vect_compare_info(int n, ch5s_vector_grid *expected, ch5s_vector_grid *given);
int _s_vect_read_and_check_vecs(hid_t file, int grid_index);

#endif

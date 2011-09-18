#ifndef TEST_S_NODAL_H
#define TEST_S_NODAL_H

#include "types.h"
#include <hdf5.h>

int s_noda_creates_grid();
int s_noda_gets_info();
int s_noda_counts_grids();
int s_noda_writes_data();
int s_noda_error_writing_bad_time_ranges();
int s_noda_reads_data_by_time_range();
int s_noda_reads_data_by_node_index();

int _s_noda_read_and_check_data(hid_t file, ch5s_nodal_type type, int n, int t, int grid_index);
int _s_noda_compare_info(int n, ch5s_nodal_grid *expected, ch5s_nodal_grid *given);
int _s_noda_compare_strings(int n, const char *kind, const char *expected, const char *given);

#endif

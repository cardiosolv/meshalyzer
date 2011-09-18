#ifndef TEST_POINTS_H
#define TEST_POINTS_H

#include <hdf5.h>

int pnts_writes_all_data();
int pnts_resizes_on_write_all();
int pnts_writes_range();
int pnts_reads_info();
int pnts_bad_info_returns_zero();
int pnts_reads_all_data();
int pnts_bad_data_returns_zero();
int pnts_reads_range();
int _pnts_read_all_and_assert_equal(hid_t file, int count, float *expected);

#endif

#ifndef TEST_LONS_H
#define TEST_LONS_H

#include <hdf5.h>

int lons_writes_all();
int lons_resizes_on_write_all();
int lons_writes_range();
int lons_reads_info();
int lons_bad_info_returns_zero();
int lons_reads_order();
int lons_bad_order_returns_negone();
int lons_reads_all_data();
int lons_reads_range();
int _lons_read_all_and_assert_equal(hid_t file, int count, int order, float *expected);

#endif

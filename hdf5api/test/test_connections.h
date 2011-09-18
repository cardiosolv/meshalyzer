#ifndef TEST_CONNECTIONS_H
#define TEST_CONNECTIONS_H

#include <hdf5.h>

int conn_writes_all();
int conn_resizes_on_write_all();
int conn_writes_range();
int conn_reads_info();
int conn_bad_info_returns_zero();
int conn_reads_all_data();
int conn_bad_data_returns_zero();
int conn_reads_range();
int _conn_read_all_and_assert_equal(hid_t file, int count, int *expected);

#endif

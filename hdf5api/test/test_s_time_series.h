#ifndef TEST_S_TIMESERIES_H
#define TEST_S_TIMESERIES_H

#include "types.h"
#include <hdf5.h>

int s_time_creates_series();
int s_time_gets_series_info();
int s_time_counts_series();
int s_time_writes_data();
int s_time_reads_data();
int s_time_reads_data_by_value();

int _s_time_compare_info(int n, ch5s_series *expected, ch5s_series *given);
int _s_time_compare_strings(int n, const char *kind, const char *expected, const char *given);

#endif

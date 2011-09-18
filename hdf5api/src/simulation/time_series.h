/**
* \file time_series.h
* \brief Time series data for simulations
*/

#ifndef _CH5S_TIMESERIES_H
#define _CH5S_TIMESERIES_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
#define CH5_TIME_GROUP_NAME    "time_series"
#define CH5_TIME_GROUP_PATH    "/" CH5_TIME_GROUP_NAME
#define CH5_TIME_SERIES_PREFIX "Series"
#define CH5_TIME_QUANT_ATTR    "q"
#define CH5_TIME_ORIENT_TIME   1
#define CH5_TIME_ORIENT_VALUE  2
/// @endcond

int ch5s_series_create(
  hid_t hdf_file,
  unsigned int t,
  unsigned int n,
  const char *label,
  const char *quantities,
  const char *units,
  const char *comments
);

int ch5s_series_info(
  hid_t hdf_file,
  unsigned int series_index,
  ch5s_series *info
);

int ch5s_series_free_info(ch5s_series *info);

int ch5s_series_count(hid_t hdf_file);

int ch5s_series_write(
  hid_t hdf_file,
  unsigned int series_index,
  unsigned int from_time,
  unsigned int to_time,
  double *in
);

int ch5s_series_read(
  hid_t hdf_file,
  unsigned int series_index,
  unsigned int from_time,
  unsigned int to_time,
  double *out
);

int ch5s_series_read_values(
  hid_t hdf_file,
  unsigned int series_index,
  unsigned int from_val,
  unsigned int to_val,
  double *out
);

/// @cond INTERNAL

int _ch5s_series_read_write_general(
  hid_t hdf_file,
  unsigned int series_index,
  int rw_id,
  int orient,
  unsigned int from_time,
  unsigned int to_time,
  double *inout
);

// @endcond

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

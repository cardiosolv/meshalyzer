/**
* \file nodal.h
* \brief Nodal data for simulations
*/

#ifndef _CH5S_NODAL_H
#define _CH5S_NODAL_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
#define CH5_NODA_GROUP_NAME        "nodal"
#define CH5_NODA_GROUP_PATH        "/" CH5_NODA_GROUP_NAME
#define CH5_NODA_GRID_NAME_PREFIX  "Grid"
#define CH5_NODA_TYPE_ATTR         "type"
#define CH5_NODA_SCALAR_NODE_WIDTH 1
#define CH5_NODA_DYNPTS_NODE_WIDTH 3
/// @endcond

int ch5s_nodal_create_grid(
  hid_t hdf_file,
  unsigned int n,
  unsigned int t,
  float t000,
  float time_delta,
  ch5s_nodal_type type,
  const char *label,
  const char *time_units,
  const char *units,
  const char *comments
);

int ch5s_nodal_grid_info(
  hid_t hdf_file,
  unsigned int grid_index,
  struct ch5s_nodal_grid *info
);

int ch5s_nodal_free_grid_info(ch5s_nodal_grid *info);

int ch5s_nodal_grid_count(hid_t hdf_file);

int ch5s_nodal_write(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int from_time,
  unsigned int to_time,
  float *in
);

int ch5s_nodal_read(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int from_time,
  unsigned int to_time,
  float *out
);

int ch5s_nodal_read_time_series(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int node_index,
  float *out
);

/// @cond INTERNAL

int _ch5s_nodal_read_write_general(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int from_time,
  unsigned int to_time,
  int rw_id,
  float *inout
);

/// @endcond

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

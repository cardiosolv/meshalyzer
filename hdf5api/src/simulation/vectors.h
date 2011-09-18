/**
* \file vectors.h
* \brief Vector grid data for simulations
*/

#ifndef _CH5S_VECTORS_H
#define _CH5S_VECTORS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
#define CH5_VECS_GROUP_NAME       "vector"
#define CH5_VECS_GROUP_PATH       "/" CH5_VECS_GROUP_NAME
#define CH5_VECS_GRID_NAME_PREFIX "Grid"
#define CH5_VECS_POINTS_NAME      "points"
#define CH5_VECS_VECTORS_NAME     "vectors"
#define CH5_VECS_SCALAR_ATTR      "scalar_label"
#define CH5_VECS_BASE_COMPONENTS  3
/// @endcond

int ch5s_vector_grid_count(hid_t hdf_file);

int ch5s_vector_create_grid(
  hid_t hdf_file,
  unsigned int n,
  unsigned int t,
  float t0,
  float time_delta,
  float *points,
  const char *label,
  const char *scalar_label,
  const char *time_units,
  const char *units,
  const char *comments
);

int ch5s_vector_grid_info(
  hid_t hdf_file,
  unsigned int grid_index,
  ch5s_vector_grid *info
);

int ch5s_vector_free_grid_info(ch5s_vector_grid *info);

int ch5s_vector_write(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int from_time,
  unsigned int to_time,
  float *in
);

int ch5s_vector_write_all(
  hid_t hdf_file,
  unsigned int grid_index,
  float *in
);

int ch5s_vector_read(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int from_time,
  unsigned int to_time,
  float *out
);

int ch5s_vector_read_points(
  hid_t hdf_file,
  unsigned int grid_index,
  float *out
);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

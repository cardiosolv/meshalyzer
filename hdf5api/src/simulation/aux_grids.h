/**
* \file aux_grids.h
* \brief Auxilliary grid data for simulations
*/

#ifndef _CH5S_AUX_GRIDS_H
#define _CH5S_AUX_GRIDS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
#define CH5_AUX_GROUP_NAME       "aux_grids"
#define CH5_AUX_POINTS_DSET_NAME "points"
#define CH5_AUX_ELEMS_DSET_NAME  "elements"
#define CH5_AUX_DATA_DSET_NAME   "data"
#define CH5_AUX_GRID_NAME_PREFIX "Grid"
#define CH5_AUX_TIME_NAME_PREFIX "Time"
/// @endcond

int ch5s_aux_create_grid(
  hid_t hdf_file,
  float t0,
  float time_delta,
  const char *label,
  const char *time_units,
  const char *units,
  const char *comments
);

int ch5s_aux_grid_info(
  hid_t hdf_file,
  unsigned int grid_index,
  ch5s_aux_grid *info
);

int ch5s_aux_free_grid_info(ch5s_aux_grid *info);

int ch5s_aux_grid_count(hid_t hdf_file);

int ch5s_aux_write_next(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int np,
  float *points,
  unsigned int ne,
  unsigned int max_elem_width,
  unsigned int *elements,
  float *data
);

int ch5s_aux_time_step_info(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int time_index,
  ch5s_aux_time_step *info
);

int ch5s_aux_read(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int time_index,
  float *points,
  unsigned int *elements,
  float *data
);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

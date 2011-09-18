/**
* \file generics.h
* \brief Generic functions used throughout the API
*/

#ifndef _CH5M_GENERICS_H
#define _CH5M_GENERICS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "types.h"

#include <hdf5.h>

/// @cond INTERNAL
#define CH5_NCHILD_ZEROPAD 6
#define CH5_QUOTE_(x) #x
#define CH5_QUOTE(x) CH5_QUOTE_(x)

#define SET_ATTR(grid, type, name, ptr) \
result = ch5_gnrc_set_attr(grid, type,name, ptr); \
if (result == 1) { \
  H5Dclose(grid); \
  return -1; \
}

#define SET_NON_NULL_ATTR(grid, type, name, value, ptr) \
if (value != NULL) { \
  SET_ATTR(grid, type, name, ptr) \
}

#define GET_ATTR(grid, type, name, ptr) \
result = ch5_gnrc_get_attr(grid, type, name, ptr); \
if (result != 0) { \
  H5Dclose(grid_id); \
  return 1; \
}

#define GET_STRING_ATTR(grid, name, ptr) \
result = ch5_gnrc_get_attr(grid, H5T_C_S1, name, ptr); \
if      (result == 2) *(ptr) = NULL; \
else if (result == 1) { \
  H5Dclose(grid_id); \
  return 1; \
}
/// @endcond

int ch5_gnrc_get_dset_info(
  hid_t file_id,
  const char *path,
  ch5_dataset *out_info
);

int ch5_gnrc_get_attr(
  hid_t parent_id,
  hid_t type_id,
  const char *name,
  void *data
);

int ch5_gnrc_set_attr(
  hid_t parent_id,
  hid_t type_id,
  const char *name,
  void *data
);

int ch5_gnrc_get_all(
  hid_t file_id,
  const char *path,
  hid_t memtype,
  void *out_data
);

int ch5_gnrc_get_range(
  hid_t file_id,
  const char *path,
  hid_t memtype,
  int start,
  int end,
  int width,
  void *out_data
);

hid_t ch5_gnrc_open_or_create_group(hid_t parent_id, const char *name);

hid_t ch5_gnrc_open_or_create_dset(
  hid_t parent_id,
  const char *name,
  hid_t type_id,
  int rank,
  hsize_t *dims
);

hid_t ch5_gnrc_open_or_create_chunked_dset(
  hid_t parent_id,
  const char *name,
  hid_t type_id,
  int rank,
  hsize_t *dims,
  hsize_t *max_dims,
  hsize_t *chunk_dims,
  int gzip_level
);

char* ch5_nchild_gen_name(const char *prefix, int n);

hid_t ch5_nchild_create_or_open_container(hid_t hdf_file, const char *name);

int ch5_nchild_count_children(hid_t container_id);

int ch5_nchild_open_child(
  hid_t container_id,
  int n,
  hid_t *child_id,
  H5O_type_t *child_type /* Can be NULL */
);

/// @cond INTERNAL

herr_t _ch5_nchild_iter_callback(
  hid_t container_id,
  const char *name,
  const H5L_info_t *info,
  void *op_data
);

/*
* Internal structure used as a persistent data source while iterating through
* surfaces when opening a surface by numeric id.  See _surf_iter_callback.
*/
struct _ch5_nchild_iter_data {
  int        target_index;
  hid_t      child_id;
  H5O_type_t child_type;
};
typedef struct _ch5_nchild_iter_data _ch5_nchild_iter_data;

/// @endcond

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

#include "nodal.h"
#include "../generics.h"
#include "../types.h"

#include <stdlib.h>

/**
* \brief Creates a new nodal grid
* \note Grids of type \p CH5_SCALAR will contain only one value per node
*       whereas grids of type \p CH5_DYN_PTS contain three values per node.
* \note Groups are named as \p Grid###### starting at 0
* \param[in] hdf_file   The HDF file reference id
* \param[in] n          The number of nodes in the grid
* \param[in] t          The number of time intervals
* \param[in] t0         The initial time
* \param[in] time_delta The duration of each time interval
* \param[in] type       The #ch5s_nodal_type this grid represents
* \param[in] label      (Optional) A label for the grid (NULL for none)
* \param[in] time_units (Optional) The time units for the data in the grid (NULL for none)
* \param[in] units      (Optional) The units for the data in the grid (NULL for none)
* \param[in] comments   (Optional) Additional comments about the grid data (NULL for none)
* \returns The numeric index for the newly created grid
* \retval -1 Failure
*/
int ch5s_nodal_create_grid(hid_t hdf_file, unsigned int n, unsigned int t,
  float t0, float time_delta, ch5s_nodal_type type, const char *label,
  const char *time_units, const char *units, const char *comments)
{
  if (type != CH5_SCALAR && type != CH5_DYN_PTS) return -1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_NODA_GROUP_NAME);
  if (container_id < 0) return -1;
  
  int grid_num = ch5_nchild_count_children(container_id);
  char *gen_name = ch5_nchild_gen_name(CH5_NODA_GRID_NAME_PREFIX, grid_num);
  
  hid_t grid_id = ch5_gnrc_open_or_create_dset(container_id, gen_name,
    H5T_IEEE_F32LE, 3,
    (hsize_t[3]){ t, n, (type == CH5_SCALAR) ? CH5_NODA_SCALAR_NODE_WIDTH : CH5_NODA_DYNPTS_NODE_WIDTH });
  free(gen_name);
  H5Gclose(container_id);
  if (grid_id < 0) return -1;
  
  int result;
  SET_ATTR(grid_id, H5T_IEEE_F32LE, CH5_DELTA_T_ATTR, &time_delta);
  SET_ATTR(grid_id, H5T_IEEE_F32LE, CH5_T0_ATTR, &t0       );
  SET_ATTR(grid_id, H5T_STD_U32LE,  CH5_NODA_TYPE_ATTR,    &type);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_LABEL_ATTR,      label,      (char*)label);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_TIME_UNITS_ATTR, time_units, (char*)time_units);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_UNITS_ATTR,      label,      (char*)units);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_COMMENTS_ATTR,   comments,   (char*)comments);
  
  H5Dclose(grid_id);
  
  return grid_num;
}

/**
* \brief Retrieves information about a nodal grid given a valid index
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the nodal grid to get info about
* \param[out] info       Pointer to the ch5s_nodal_grid struct to populate
*                        with the retrieved info
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_nodal_grid_info(hid_t hdf_file, unsigned int grid_index,
  ch5s_nodal_grid *info)
{
  /* NULL string pointers to ensure values even with failed results */
  info->label    = NULL;
  info->units    = NULL;
  info->comments = NULL;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_NODA_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int result = ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL);
  H5Gclose(container_id);
  if (result != 0) return 1;
  
  hid_t space_id = H5Dget_space(grid_id);
  int ndims = H5Sget_simple_extent_ndims(space_id);
  hsize_t dims[ndims];
  result = H5Sget_simple_extent_dims(space_id, dims, NULL);
  H5Sclose(space_id);
  if (result != ndims) {
    H5Dclose(grid_id);
    return 1;
  }
  
  info->time_steps = dims[0];
  info->num_nodes  = dims[1];
  info->node_width = dims[2];
  
  GET_ATTR(grid_id, H5T_NATIVE_FLOAT, CH5_DELTA_T_ATTR,   &(info->time_delta));
  GET_ATTR(grid_id, H5T_NATIVE_FLOAT, CH5_T0_ATTR,        &(info->t0));
  GET_ATTR(grid_id, H5T_NATIVE_INT,   CH5_NODA_TYPE_ATTR, &(info->type));
  GET_STRING_ATTR(grid_id, CH5_LABEL_ATTR,      &(info->label));
  GET_STRING_ATTR(grid_id, CH5_TIME_UNITS_ATTR, &(info->time_units));
  GET_STRING_ATTR(grid_id, CH5_UNITS_ATTR,      &(info->units));
  GET_STRING_ATTR(grid_id, CH5_COMMENTS_ATTR,   &(info->comments));
  
  H5Dclose(grid_id);
  
  return 0;
}

/**
* \brief Frees any allocated memory used in #ch5s_nodal_grid structs
* \param[in] info A pointer to the #ch5s_nodal_grid struct to free
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_nodal_free_grid_info(ch5s_nodal_grid *info) {
  if (info == NULL) return 1;
  if (info->label      != NULL) free((char*)info->label);
  if (info->time_units != NULL) free((char*)info->time_units);
  if (info->units      != NULL) free((char*)info->units);
  if (info->comments   != NULL) free((char*)info->comments);
  return 0;
}

/**
* \brief Counts the nodal grids currently defined
* \param[in] hdf_file The HDF file reference id
* \returns Number of nodal grids
* \retval -1 Failure
*/
int ch5s_nodal_grid_count(hid_t hdf_file) {
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_NODA_GROUP_NAME);
  if (container_id < 0) return -1;
  int count = ch5_nchild_count_children(container_id);
  H5Gclose(container_id);
  return count;
}

/**
* \brief Writes nodal data to a grid over a time range
*
* Size for \p in is <tt>sizeof(float) * (to_time - from_time + 1) * n * w</tt>
* and is a 3D array in the form:
*
* <tt>in[time(0..to_time-from_time)][node(0..n)][value(0..w-1)]</tt>
*
* Where \p n is the number of nodes represented in the grid and \w is the
* number of values per node (3 for grids of type CH5_DYN_PTS and 1 for grids
* of type CH5_SCALAR).
*
* \note \p in must be a contiguous set of memory, the format described here is
*       for visualization only - the user is responsible for supplying the
*       data using the correct offsets
*
* \param[in] hdf_file   The HDF file reference id
* \param[in] grid_index The index of the nodal grid to write to
* \param[in] from_time  The time index to begin writing at (inclusive)
* \param[in] to_time    The time index to end writing at (inclusive)
* \param[in] in         The nodal data to be written (see function description
*                       for expected format)
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_nodal_write(hid_t hdf_file, unsigned int grid_index,
  unsigned int from_time, unsigned int to_time, float *in)
{
  return _ch5s_nodal_read_write_general(hdf_file, grid_index, from_time,
    to_time, CH5_WRITE, in);
}

/**
* \brief Reads nodal data from a grid over a time range
*
* Size for \p out is <tt>sizeof(float) * (to_time - from_time + 1) * n * w</tt>
* and is a 3D array in the form:
*
* <tt>out[time(0..to_time-from_time)][node(0..n-1)][value(0..w-1)]</tt>
*
* Where \p n is the number of nodes represented in the grid and \w is the
* number of values per node (3 for grids of type CH5_DYN_PTS and 1 for grids
* of type CH5_SCALAR).
*
* \note \p out must be a contiguous set of memory, the format described here is
*       for visualization only - the user is responsible for calculating the
*       correct offsets to extract the data
*
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the nodal grid to read from
* \param[in]  from_time  The time index to begin reading from (inclusive)
* \param[in]  to_time    The time index to end reading from (inclusive)
* \param[out] out        A pointer to an array of floats of the size described
*                        in the function description
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_nodal_read(hid_t hdf_file, unsigned int grid_index,
  unsigned int from_time, unsigned int to_time, float *out)
{
  return _ch5s_nodal_read_write_general(hdf_file, grid_index, from_time,
    to_time, CH5_READ, out);
}

/**
* \brief Reads nodal data for a single node spanning the duration of the grid
*
* Size for \p out is <tt>sizeof(float) * t * w</tt> and is a 2D array in the form:
*
* <tt>out[time(0..t-1)][value(0..w-1)]</tt>
*
* Where \p t is the number of time steps in the grid and \w is the
* number of values per node (3 for grids of type CH5_DYN_PTS and 1 for grids
* of type CH5_SCALAR).
*
* \note \p out must be a contiguous set of memory, the format described here is
*       for visualization only - the user is responsible for calculating the
*       correct offsets to extract the data
*
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the nodal grid to read from
* \param[in]  from_time  The time index to begin reading from (inclusive)
* \param[in]  to_time    The time index to end reading from (inclusive)
* \param[out] out        A pointer to an array of floats of the size described
*                        in the function description
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_nodal_read_time_series(hid_t hdf_file, unsigned int grid_index,
  unsigned int node_index, float *out)
{
  if (out == NULL) return 1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_NODA_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int result = (ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL) == 0);
  H5Gclose(container_id);
  if (result == 0) return 1;
  
  ch5s_nodal_grid info;
  ch5s_nodal_grid_info(hdf_file, grid_index, &info);
  ch5s_nodal_free_grid_info(&info);/* don't need the strings anyway */
  
  if (node_index >= info.num_nodes) {/* invalid node index */
    H5Dclose(grid_id);
    return 1;
  }
  
  int node_width = (info.type == CH5_SCALAR) ? CH5_NODA_SCALAR_NODE_WIDTH : CH5_NODA_DYNPTS_NODE_WIDTH;
  
  hid_t space_id = H5Dget_space(grid_id);
  herr_t status = H5Sselect_hyperslab(space_id, H5S_SELECT_SET,
    (hsize_t[3]){ 0,               node_index, 0          }, NULL,
    (hsize_t[3]){ info.time_steps, node_index, node_width }, NULL);
  if (status < 0) {
    H5Sclose(space_id);
    H5Dclose(grid_id);
    return 1;
  }
  
  hid_t memspace_id = H5Screate_simple(3,
    (hsize_t[3]){ info.time_steps, 1, node_width }, NULL);
  
  result = H5Dread(grid_id, H5T_NATIVE_FLOAT, memspace_id, space_id, H5P_DEFAULT, out);
  
  H5Sclose(space_id);
  H5Sclose(memspace_id);
  H5Dclose(grid_id);
  
  if (result < 0) return 1;
  
  return 0;
}

/// @cond INTERNAL

int _ch5s_nodal_read_write_general(hid_t hdf_file, unsigned int grid_index,
  unsigned int from_time, unsigned int to_time, int rw_id, float *inout)
{
  if (rw_id != CH5_READ && rw_id != CH5_WRITE) return 1;
  if (from_time > to_time)                     return 1;
  if (from_time < 0 || to_time < 0)            return 1;
  if (inout == NULL)                           return 1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_NODA_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int result = (ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL) == 0);
  H5Gclose(container_id);
  if (result == 0) return 1;
  
  ch5s_nodal_grid info;
  ch5s_nodal_grid_info(hdf_file, grid_index, &info);
  ch5s_nodal_free_grid_info(&info);/* don't need the strings anyway */
  
  if (from_time >= info.time_steps || to_time >= info.time_steps) {
    H5Dclose(grid_id);
    return 1;
  }
  
  int node_width = (info.type == CH5_SCALAR) ? CH5_NODA_SCALAR_NODE_WIDTH : CH5_NODA_DYNPTS_NODE_WIDTH;
  
  hid_t space_id = H5Dget_space(grid_id);
  herr_t status = H5Sselect_hyperslab(space_id, H5S_SELECT_SET,
    (hsize_t[3]){ from_time,           0,              0          }, NULL,
    (hsize_t[3]){ to_time-from_time+1, info.num_nodes, node_width }, NULL);
  if (status < 0) {
    H5Sclose(space_id);
    H5Dclose(grid_id);
    return 1;
  }
  
  hid_t memspace_id = H5Screate_simple(3,
    (hsize_t[3]){ to_time-from_time+1, info.num_nodes, node_width }, NULL);
    
  switch (rw_id) {
    case CH5_READ:
      result = H5Dread(grid_id, H5T_NATIVE_FLOAT, memspace_id, space_id, H5P_DEFAULT, inout);
      break;
    
    case CH5_WRITE:
      result = H5Dwrite(grid_id, H5T_IEEE_F32LE, memspace_id, space_id, H5P_DEFAULT, inout);
      break;
  }
  
  H5Sclose(space_id);
  H5Sclose(memspace_id);
  H5Dclose(grid_id);
  
  if (result < 0) return 1;
  
  return 0;
}

/// @endcond

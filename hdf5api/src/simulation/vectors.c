#include "vectors.h"
#include "../generics.h"

#include <stdlib.h>

/// @cond INTERNAL
static int _ch5s_vector_read_write_general(
  hid_t hdf_file,
  unsigned int grid_index,
  unsigned int from_time,
  unsigned int to_time,
  int rw_id,
  float *inout
);
/// @endcond

/**
* \brief Counts the vector grids currently defined
* \param[in] hdf_file The HDF file reference id
* \returns Number of vector grids
* \retval -1 Failure
*/
int ch5s_vector_grid_count(hid_t hdf_file) {
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_VECS_GROUP_NAME);
  if (container_id < 0) return -1;
  int count = ch5_nchild_count_children(container_id);
  H5Gclose(container_id);
  return count;
}

/**
* \brief Creates a new vector grid of n vectors over t time intervals
* \details This will create a group containing two datasets: vectors and data
* \note When \p scalar_label is not specified (<tt>NULL</tt> is passed) then
*       the dataset created will have 3 components per vector.  However, if
*       the \scalar_label argument is not \p NULL the dataset will have 4
*       components per vector allocated.  The \p num_components property
*       of #ch5s_vector_grid contains this information.
* \note Groups are named as \p Grid###### starting at 0
* \param[in] hdf_file     The HDF file reference id
* \param[in] n            The number of vectors in the grid
* \param[in] t            The number of time intervals
* \param[in] t0           The initial time
* \param[in] time_delta   The duration of each time interval
* \param[in] points       A pointer to an array of points where the number of
*                         points is equal to \p n
* \param[in] label        (Optional) A label for the grid (NULL for none)
* \param[in] scalar_label (Optional) A label for the scalar value (NULL for none)
* \param[in] time_units   (Optional) The time units for the data in the grid (NULL for none)
* \param[in] units        (Optional) The units for the data in the grid (NULL for none)
* \param[in] comments     (Optional) Additional comments about the grid data (NULL for none)
* \returns The numeric index for the newly created grid
* \retval -1 Failure
*/
int ch5s_vector_create_grid(hid_t hdf_file, unsigned int n, unsigned int t,
  float t0,  float time_delta, float *points, const char* label,
  const char* scalar_label,
  const char *time_units, const char *units, const char *comments)
{
  if (n < 1 || t < 1) return -1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_VECS_GROUP_NAME);
  if (container_id < 0) return -1;
  
  int grid_num = ch5_nchild_count_children(container_id);
  char *gen_name = ch5_nchild_gen_name(CH5_VECS_GRID_NAME_PREFIX, grid_num, label);
  
  int vector_components = CH5_VECS_BASE_COMPONENTS;
  if (scalar_label != NULL) vector_components += 1;
  
  hid_t grid_id = ch5_nchild_create_or_open_container(container_id, gen_name);
  H5Gclose(container_id);
  if (grid_id < 0) return -1;
  
  hid_t pnts_dset_id = ch5_gnrc_open_or_create_dset(grid_id,
    CH5_VECS_POINTS_NAME, H5T_IEEE_F32LE, 2, (hsize_t[2]){ n, 3 });
  if (pnts_dset_id < 0) {
    H5Gclose(grid_id);
    return -1;
  }
  herr_t write_status = H5Dwrite(pnts_dset_id, H5T_NATIVE_FLOAT, H5S_ALL,
    H5S_ALL, H5P_DEFAULT, points);
  H5Dclose(pnts_dset_id);
  if (write_status < 0) {
    H5Gclose(grid_id);
    return -1;
  }
  
  hid_t vecs_dset_id = ch5_gnrc_open_or_create_dset(grid_id,
    CH5_VECS_VECTORS_NAME, H5T_IEEE_F32LE, 3,
    (hsize_t[3]){ t, n, vector_components });
  free(gen_name);
  if (vecs_dset_id < 0) {
    H5Gclose(grid_id);
    return -1;
  }
  H5Dclose(vecs_dset_id);
  
  SET_ATTR(grid_id, H5T_NATIVE_UINT32, CH5_T_ATTR,       (int[]){0} );
  SET_ATTR(grid_id, H5T_IEEE_F32LE,    CH5_DELTA_T_ATTR, &time_delta);
  SET_ATTR(grid_id, H5T_IEEE_F32LE,    CH5_T0_ATTR,      &t0        );
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_LABEL_ATTR,       label,        (char*)label       );
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_VECS_SCALAR_ATTR, scalar_label, (char*)scalar_label);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_TIME_UNITS_ATTR,  time_units,   (char*)time_units  );
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_UNITS_ATTR,       units,        (char*)units       );
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_COMMENTS_ATTR,    comments,     (char*)comments    );
  
  H5Gclose(grid_id);
  
  return grid_num;
}

/**
* \brief Retrieves information about a vector grid given a valid grid index
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the grid to get info about
* \param[out] info       Pointer to the #ch5s_vector_grid struct to populate
*                        with the retrieved info
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_vector_grid_info(hid_t hdf_file, unsigned int grid_index,
  ch5s_vector_grid *info)
{
  /* NULL these string pointers to ensure their value even with failed results */
  info->label        = NULL;
  info->scalar_label = NULL;
  info->units        = NULL;
  info->comments     = NULL;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_VECS_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int result = (ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL) == 0);
  H5Gclose(container_id);
  if (result == 0) return 1;
  
  hid_t vecs_dset_id = H5Dopen(grid_id, CH5_VECS_VECTORS_NAME, H5P_DEFAULT);
  if (vecs_dset_id < 0) {
    H5Gclose(grid_id);
    return 1;
  }
  
  hid_t space_id = H5Dget_space(vecs_dset_id);
  int ndims = H5Sget_simple_extent_ndims(space_id);
  hsize_t dims[ndims];
  result = H5Sget_simple_extent_dims(space_id, dims, NULL);
  H5Sclose(space_id);
  H5Dclose(vecs_dset_id);
  if (result != ndims) {
    H5Gclose(grid_id);
    return 1;
  }
  
  info->max_time_steps = dims[0];
  info->num_vectors    = dims[1];
  info->num_components = dims[2];
  
  GET_ATTR( grid_id, H5T_NATIVE_UINT32, CH5_T_ATTR,       &(info->time_steps));
  GET_ATTR( grid_id, H5T_NATIVE_FLOAT,  CH5_DELTA_T_ATTR, &(info->time_delta));
  GET_STRING_ATTR(grid_id, CH5_LABEL_ATTR,       &(info->label));
  GET_STRING_ATTR(grid_id, CH5_VECS_SCALAR_ATTR, &(info->scalar_label));
  GET_STRING_ATTR(grid_id, CH5_TIME_UNITS_ATTR,  &(info->time_units));
  GET_STRING_ATTR(grid_id, CH5_T0_ATTR,          &(info->time_units));
  GET_STRING_ATTR(grid_id, CH5_UNITS_ATTR,       &(info->units));
  GET_STRING_ATTR(grid_id, CH5_COMMENTS_ATTR,    &(info->comments));
  
  H5Gclose(grid_id);
  
  return 0;
}

/**
* \brief Frees any allocated memory used in #ch5s_vector_grid structs
* \param[in] info A pointer to the #ch5s_vector_grid struct to free
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_vector_free_grid_info(ch5s_vector_grid *info) {
  if (info == NULL) return 1;
  if (info->label        != NULL) free((char*)info->label);
  if (info->scalar_label != NULL) free((char*)info->scalar_label);
  if (info->time_units   != NULL) free((char*)info->time_units);
  if (info->units        != NULL) free((char*)info->units);
  if (info->comments     != NULL) free((char*)info->comments);
  return 0;
}

/**
* \brief Writes vector data to the grid over a given time span
*
* Size for \p in is <tt>sizeof(float) * n * (to_time - from_time + 1) * 4</tt>
* and is a 3D array in the form:
*
* <tt>in[time(0..(to_time - from_time))][vector(0..n-1)][x(0),y(1),z(2),d(3)]</tt>
*
* Where \p n is the number of vectors represented in the grid.
*
* \note \p in must be a contiguous set of memory, the format described here is
*       for visualization only - the user is responsible for supplying the
*       data using the correct offsets
*
* \param[in] hdf_file   The HDF file reference id
* \param[in] grid_index The index of the grid to write to
* \param[in] from_time  The time index to begin writing at (inclusive)
* \param[in] to_time    The time index to end writing at (inclusive)
* \param[in] in         The vector data to be written (see function
*                       description for expected format)
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_vector_write(hid_t hdf_file, unsigned int grid_index,
  unsigned int from_time, unsigned int to_time, float *in)
{
  return _ch5s_vector_read_write_general(hdf_file, grid_index, from_time,
    to_time, CH5_WRITE, in);
}

/**
* \brief Writes all vector data over entire time span
*
* Size for \p in is <tt>sizeof(float) * t * n * 4</tt> where \p t is the
* number of time steps and \p n is the number of vectors.
*
* \param[in] hdf_file   The HDF file reference id
* \param[in] grid_index The index of the grid to write to
* \param[in] in         The vector data to be written (see function
*                       description for expected format)
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_vector_write_all(hid_t hdf_file, unsigned int grid_index, float *in) {
  if (in == NULL) return 1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_VECS_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int result = (ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL) == 0);
  H5Gclose(container_id);
  if ( !result ) return 1;
  
  hid_t vecs_dset_id = H5Dopen(grid_id, CH5_VECS_VECTORS_NAME, H5P_DEFAULT);
  H5Gclose(grid_id);
  if (vecs_dset_id < 0) return 1;
  
  herr_t status = H5Dwrite(vecs_dset_id, H5T_IEEE_F32LE, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, in);
  H5Dclose(vecs_dset_id);

  ch5s_vector_grid vg_info;
  ch5s_vector_grid_info( hdf_file, grid_index, &vg_info );
  SET_ATTR( grid_id, H5T_NATIVE_UINT32, CH5_T_ATTR, &vg_info.max_time_steps );
  ch5s_vector_free_grid_info( &vg_info );

  return status < 0;
}


/**
* \brief Reads vector data from the grid over a given time span
*
* Size for \p out is <tt>sizeof(float) * n * (to_time - from_time + 1) * 4</tt>
* and is a 3D array in the form:
*
* <tt>out[time(0..(to_time - from_time))][vector(0..n-1)][x(0),y(1),z(2),d(3)]</tt>
*
* Where \p n is the number of vectors represented in the grid.
*
* \note \p out must be a contiguous set of memory, the format described here is
*       for visualization only - the user is responsible for calculating the
*       correct offsets to extract the data
*
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the grid to read from
* \param[in]  from_time  The time index to begin reading from (inclusive)
* \param[in]  to_time    The time index to end reading from (inclusive)
* \param[out] out        A pointer to an array of floats of the size described
*                        in the function description
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_vector_read(hid_t hdf_file, unsigned int grid_index,
  unsigned int from_time, unsigned int to_time, float *out)
{
  return _ch5s_vector_read_write_general(hdf_file, grid_index, from_time,
    to_time, CH5_READ, out);
}

/**
* \brief Reads the entire set for the given grid
*
* The size of the data is <tt>sizeof(float) * num_points * 3</tt>
*
* \p num_points equals the number of vectors per time step which can be
* obtained from #ch5s_vector_grid_info
*
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the grid to read from
* \param[out] out        A pointer to an array of floats of the size described
*                        in the function description
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5s_vector_read_points(hid_t hdf_file, unsigned int grid_index,
  float *out)
{
  if (out == NULL) return 1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_VECS_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int result = (ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL) == 0);
  H5Gclose(container_id);
  if (result == 0) return 1;
  
  hid_t pnts_dset_id = H5Dopen(grid_id, CH5_VECS_POINTS_NAME, H5P_DEFAULT);
  H5Gclose(grid_id);
  if (pnts_dset_id < 0) return 1;
  
  herr_t status = H5Dread(pnts_dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, out);
  H5Dclose(pnts_dset_id);
  if (status < 0) return 1;
  
  return 0;
}

/// @cond INTERNAL

/*
* Generally read or write data to vector grids. set_id designates which
* dataset to write to (either CH5_VECS_VECS_DSET_ID or CH5_VECS_DATA_DSET_ID)
* and rw_id indicates whether the operation is read or write (CH5_READ
* or CH5_WRITE).
*/
int _ch5s_vector_read_write_general(hid_t hdf_file, unsigned int grid_index,
  unsigned int from_time, unsigned int to_time, int rw_id, float *inout)
{
  if (rw_id != CH5_READ && rw_id != CH5_WRITE) return 1;
  if (from_time > to_time)                       return 1;
  if (from_time < 0 || to_time < 0)              return 1;
  if (inout == NULL)                           return 1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_VECS_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int result = (ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL) == 0);
  H5Gclose(container_id);
  if (result == 0) return 1;
  
  hid_t vecs_dset_id = H5Dopen(grid_id, CH5_VECS_VECTORS_NAME, H5P_DEFAULT);
  H5Gclose(grid_id);
  if (vecs_dset_id < 0) return 1;
  
  ch5s_vector_grid info;
  ch5s_vector_grid_info(hdf_file, grid_index, &info);
  ch5s_vector_free_grid_info(&info);/* don't need the strings */
  
  if( to_time >= info.time_steps ) {
    if( rw_id==CH5_READ || to_time>=info.max_time_steps ) { 
      H5Dclose(grid_id);
      return 1;
    } else {
      unsigned int t = to_time+1;
      SET_ATTR(grid_index, H5T_NATIVE_UINT32, CH5_T_ATTR, &t);
    }
  } 
  
  int count = to_time - from_time + 1;
  hid_t space_id = H5Dget_space(vecs_dset_id);
  herr_t status = H5Sselect_hyperslab(space_id, H5S_SELECT_SET,
    (hsize_t[3]){ from_time, 0,                0                   }, NULL,
    (hsize_t[3]){ count,     info.num_vectors, info.num_components }, NULL);
  if (status < 0) {
    H5Sclose(space_id);
    H5Dclose(vecs_dset_id);
    return 1;
  }
  
  hid_t plist_id = H5P_DEFAULT;
#ifdef HAVE_MPIIO
  plist_id = H5Pcreate(H5P_DATASET_XFER);
  H5Pset_dxpl_mpio(plist_id, H5FD_MPIO_INDEPENDENT);
#endif

  hid_t memspace_id = H5Screate_simple(3,
    (hsize_t[3]){ count, info.num_vectors, info.num_components }, NULL);
  
  switch (rw_id) {
    case CH5_READ:
      result = H5Dread(vecs_dset_id, H5T_NATIVE_FLOAT, memspace_id, space_id, plist_id, inout);
      break;
    
    case CH5_WRITE:
      result = H5Dwrite(vecs_dset_id, H5T_IEEE_F32LE, memspace_id, space_id, plist_id, inout);
      break;
  }
  
  H5Sclose(space_id);
  H5Sclose(memspace_id);
  H5Dclose(vecs_dset_id);
  
  return (result < 0);
}

/// @endcond

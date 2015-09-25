#include "time_series.h"
#include "../generics.h"
#include "../types.h"

#include <stdlib.h>
#include <stdio.h>

/**
* \brief Creates a new time series
* \note The dimensions of the dataset created will be t &times; n+1 due to the
*       first set of values denoting the time index for each time step.
* \param[in] hdf_file   The HDF file reference id
* \param[in] t          The number of time intervals
* \param[in] n          The number of values per timestep
* \param[in] label      (Optional) Label for the series
* \param[in] quantities (Optional) Label for the quantities in the series
* \param[in] units      (Optional) Label describing the units used on the series
* \param[in] comments   (Optional) Label for general comments about the series
* \returns Index of newly created series
* \retval -1 Failure
*/
int ch5s_series_create(hid_t hdf_file, unsigned int t, unsigned int n,
  const char *label, const char *quantities, const char *units,
  const char *comments)
{
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file, CH5_TIME_GROUP_NAME);
  if (container_id < 0) return -1;
  
  int series_num = ch5_nchild_count_children(container_id);
  if (series_num == -1) {
    H5Gclose(container_id);
    return -1;
  }
  
  char* series_name = ch5_nchild_gen_name(CH5_TIME_SERIES_PREFIX, series_num,label);
  hid_t dset_id = ch5_gnrc_open_or_create_dset(container_id, series_name,
    H5T_IEEE_F64LE, 2, (hsize_t[2]){ t, n+1 });
  free(series_name);
  H5Gclose(container_id);
  if (dset_id < 0) return -1;
  
  int result = 0;
  if (label != NULL) {
    result = ch5_gnrc_set_attr(dset_id, H5T_C_S1, CH5_LABEL_ATTR, (char*)label);
    if (result == 1) {
      H5Dclose(dset_id);
      return -1;
    }
  }
  
  if (quantities != NULL) {
    result = ch5_gnrc_set_attr(dset_id, H5T_C_S1, CH5_TIME_QUANT_ATTR, (char*)quantities);
    if (result == 1) {
      H5Dclose(dset_id);
      return -1;
    }
  }
  
  if (units != NULL) {
    result = ch5_gnrc_set_attr(dset_id, H5T_C_S1, CH5_UNITS_ATTR, (char*)units);
    if (result == 1) {
      H5Dclose(dset_id);
      return -1;
    }
  }
  
  if (comments != NULL) {
    result = ch5_gnrc_set_attr(dset_id, H5T_C_S1, CH5_COMMENTS_ATTR, (char*)comments);
    if (result == 1) {
      H5Dclose(dset_id);
      return -1;
    }
  }
  
  H5Dclose(dset_id);
  
  return series_num;
}

/**
* \brief Retrieves information about a given time series
* \note Any #ch5s_series struct populated with this function should be cleaned
*       up with #ch5s_series_free_info
* \note The \p num_values property populated in the struct reflects only the
*       number of actual values represented and does not account for the first
*       element of each time step holding the time index value.
* \param[in]  hdf_file     The HDF file reference id
* \param[in]  series_index The index of the series to query
* \param[out] info         A pointer to a #ch5s_series struct to store the
*                          retrieved information
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
* \sa ch5s_series_free_info
*/
int ch5s_series_info(hid_t hdf_file, unsigned int series_index,
  ch5s_series *info)
{
  info->label      = NULL;
  info->quantities = NULL;
  info->units      = NULL;
  info->comments   = NULL;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_TIME_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t series_id;
  int result = ch5_nchild_open_child(container_id, series_index, &series_id, NULL);
  H5Gclose(container_id);
  if (result == 1) return 1;
  
  hid_t space_id = H5Dget_space(series_id);
  int ndims = H5Sget_simple_extent_ndims(space_id);
  hsize_t dims[ndims];
  result = H5Sget_simple_extent_dims(space_id, dims, NULL);
  H5Sclose(space_id);
  if (result != ndims) {
    H5Dclose(series_id);
    return 1;
  }
  
  info->time_steps = dims[0];
  info->num_values = dims[1] - 1;/* Account for extra column of time indices */
  
  result = ch5_gnrc_get_attr(series_id, H5T_C_S1, CH5_LABEL_ATTR,
    &(info->label));
  if (result == 2) info->label = NULL;
  if (result == 1) {
    H5Dclose(series_id);
    return 1;
  }
  
  result = ch5_gnrc_get_attr(series_id, H5T_C_S1, CH5_TIME_QUANT_ATTR,
    &(info->quantities));
  if (result == 2) info->quantities = NULL;
  if (result == 1) {
    H5Dclose(series_id);
    return 1;
  }
  
  result = ch5_gnrc_get_attr(series_id, H5T_C_S1, CH5_UNITS_ATTR,
    &(info->units));
  if (result == 2) info->units = NULL;
  if (result == 1) {
    H5Dclose(series_id);
    return 1;
  }
  
  result = ch5_gnrc_get_attr(series_id, H5T_C_S1, CH5_COMMENTS_ATTR,
    &(info->comments));
  if (result == 2) info->comments = NULL;
  H5Dclose(series_id);
  if (result == 1) return 1;
  
  return 0;
}

/**
* \brief Frees any allocated memory in ch5s_series structs populated by
*        #ch5s_series_info
* \param[in] info A pointer to a #ch5s_series struct
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
* \sa ch5s_series_info
*/ 
int ch5s_series_free_info(ch5s_series *info) {
  if (info == NULL) return 1;
  if (info->label      != NULL) free((char*)info->label);
  if (info->quantities != NULL) free((char*)info->quantities);
  if (info->units      != NULL) free((char*)info->units);
  if (info->comments   != NULL) free((char*)info->comments);
  return 0;
}

/**
* \brief Returns the number of time series defined in the given file
* \param[in] hdf_file The HDF file reference id
* \returns Time series count
* \retval -1 Failure
*/
int ch5s_series_count(hid_t hdf_file) {
  htri_t exists = H5Lexists(hdf_file, CH5_TIME_GROUP_NAME, H5P_DEFAULT);
  if (exists < 1) return 0;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_TIME_GROUP_NAME);
  if (container_id < 0) return -1;
  int count = ch5_nchild_count_children(container_id);
  H5Gclose(container_id);
  
  return count;
}

/**
* \brief Writes all values over a given range of time.
*
* \p in should be a 2D array of \p doubles of size
* <tt>sizeof(double) * (to_time - from_time + 1) * (n+1)</tt> in the form:
*
* <tt>in[time_index(from_time..to_time-from_time-1)][value_index(0..n)]</tt>
*
* Where \p n is the number of values per time step.
*
* \warning Note that the first value of each time step is the time index of
*          that step.
*
* \param[in] hdf_file     The HDF file reference id
* \param[in] series_index The index of the series to query
* \param[in] from_time    The time step to begin writing to (inclusive)
* \param[in] to_time      The time step to stop writing at (inclusive)
* \param[in] in           The data to be written
* \returns Status code
* \retval 0 Success
* \retval 1 Failure     
*/
int ch5s_series_write(hid_t hdf_file, unsigned int series_index,
  unsigned int from_time, unsigned int to_time, double *in)
{
  return _ch5s_series_read_write_general(hdf_file, series_index, CH5_WRITE,
    CH5_TIME_ORIENT_TIME, from_time, to_time, in);
}

/**
* \brief Reads all values over a given range of time.
*
* \p out should be a 2D array of \p doubles of size
* <tt>sizeof(double) * (to_time - from_time + 1) * (n+1)</tt> in the form:
*
* <tt>out[time_index(from_time..to_time-from_time-1)][value_index(0..n)]</tt>
*
* Where \p n is the number of values per time step.
*
* \warning Note that the first value of each time step is the time index of
*          that step.
*
* \param[in]  hdf_file     The HDF file reference id
* \param[in]  series_index The index of the series to query
* \param[in]  from_time    The time step to begin reading to (inclusive)
* \param[in]  to_time      The time step to stop reading at (inclusive)
* \param[out] out          Pointer to an array to store the data being read
* \returns Status code
* \retval 0 Success
* \retval 1 Failure     
*/
int ch5s_series_read(hid_t hdf_file, unsigned int series_index,
  unsigned int from_time, unsigned int to_time, double *out)
{
  return _ch5s_series_read_write_general(hdf_file, series_index, CH5_READ,
    CH5_TIME_ORIENT_TIME, from_time, to_time, out);
}

/**
* \brief Reads all values over a time for a given range value indexes
*
* \p out should be a 2D array of \p doubles of size
* <tt>sizeof(double) * (to_val - from_val + 1) * t</tt> in the form:
*
* <tt>out[time_index(0..t-1)][value_index(from_val..to_val-from_val-1)]</tt>
*
* Where \p t is the number of time steps in the series.
*
* \warning Note that the first value of each time step is the time index of
*          that step.
*
* \param[in]  hdf_file     The HDF file reference id
* \param[in]  series_index The index of the series to query
* \param[in]  from_val     The value index to begin reading to (inclusive)
* \param[in]  to_val       The value index to stop reading at (inclusive)
* \param[out] out          Pointer to an array to store the data being read
* \returns Status code
* \retval 0 Success
* \retval 1 Failure     
*/
int ch5s_series_read_values(hid_t hdf_file, unsigned int series_index,
  unsigned int from_val, unsigned int to_val, double *out)
{
  return _ch5s_series_read_write_general(hdf_file, series_index, CH5_READ,
    CH5_TIME_ORIENT_VALUE, from_val, to_val, out);
}

/// @cond INTERNAL

int _ch5s_series_read_write_general(hid_t hdf_file, unsigned int series_index,
  int rw_id, int orient, unsigned int from, unsigned int to, double *inout)
{
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_TIME_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t series_id;
  int result = ch5_nchild_open_child(container_id, series_index, &series_id, NULL);
  H5Gclose(container_id);
  if (result == 1) return 1;
  
  int count = to - from + 1;
  
  ch5s_series info;
  ch5s_series_info(hdf_file, series_index, &info);
  ch5s_series_free_info(&info);
  
  hid_t memspace_id, filespace_id;
  herr_t status;
  if (orient == CH5_TIME_ORIENT_TIME) {
    if (to >= info.time_steps) return 1;
  
    filespace_id = H5Dget_space(series_id);
    status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET,
      (hsize_t[2]){ from,  0                 }, NULL,
      (hsize_t[2]){ count, info.num_values+1 }, NULL);
    if (status < 0) {
      H5Dclose(series_id);
      return 1;
    }
  
    memspace_id = H5Screate_simple(2, (hsize_t[2]){ count, info.num_values+1 }, NULL);
  }
  else if (orient == CH5_TIME_ORIENT_VALUE) {
    if (to >= info.num_values) return 1;
  
    filespace_id = H5Dget_space(series_id);
    status = H5Sselect_hyperslab(filespace_id, H5S_SELECT_SET,
      (hsize_t[2]){ 0,               from  }, NULL,
      (hsize_t[2]){ info.time_steps, count }, NULL);
    if (status < 0) {
      H5Dclose(series_id);
      return 1;
    }
  
    memspace_id = H5Screate_simple(2, (hsize_t[2]){ info.time_steps, count }, NULL);
  }
  
  hid_t rw_result;
  switch (rw_id) {
    case CH5_READ:
      rw_result = H5Dread(series_id, H5T_NATIVE_DOUBLE, memspace_id, filespace_id, H5P_DEFAULT, inout);
      break;
    
    case CH5_WRITE:
      rw_result = H5Dwrite(series_id, H5T_NATIVE_DOUBLE, memspace_id, filespace_id, H5P_DEFAULT, inout);
      break;
  }
  
  H5Sclose(memspace_id);
  H5Sclose(filespace_id);
  H5Dclose(series_id);
  
  return (rw_result < 0);
}

/// @endcond

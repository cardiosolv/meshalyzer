#include "aux_grids.h"
#include "../generics.h"
#include "../model/elements.h"

#include <stdlib.h>
#include <string.h>

/**
* \brief Creates a new auiliary grid and returns it's index
* \param[in] hdf_file   The HDF file reference id
* \param[in] t0         The initial time
* \param[in] time_delta The duration of each time interval
* \param[in] label      (Optional) A label for the grid (NULL for none)
* \param[in] time_units (Optional) The units for time on the grid (NULL for none)
* \param[in] units      (Optional) The units for the data in the grid (NULL for none)
* \param[in] comments   (Optional) Additional comments about the grid data (NULL for none)
* \returns Newly created aux grid index
* \retval -1 Failure
*/
int ch5s_aux_create_grid(hid_t hdf_file, float t0, float time_delta, 
        const char *label,
  const char *time_units, const char *units, const char *comments)
{
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_AUX_GROUP_NAME);
  if (container_id < 0) return -1;
  
  int   grid_num = ch5_nchild_count_children(container_id);
  char *gen_name = ch5_nchild_gen_name(CH5_AUX_GRID_NAME_PREFIX, grid_num, label);
  
  hid_t grid_id = ch5_nchild_create_or_open_container(container_id, gen_name);
  H5Gclose(container_id);
  if (grid_id < 0) {
    H5Gclose(grid_id);
    return -1;
  }
  
  SET_ATTR(grid_id, H5T_IEEE_F32LE, CH5_DELTA_T_ATTR, &time_delta);
  SET_ATTR(grid_id, H5T_IEEE_F32LE, CH5_T0_ATTR, &t0);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_LABEL_ATTR,      label,      (char*)label);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_TIME_UNITS_ATTR, time_units, (char*)time_units);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_UNITS_ATTR,      units,      (char*)units);
  SET_NON_NULL_ATTR(grid_id, H5T_C_S1, CH5_COMMENTS_ATTR,   comments,   (char*)comments);
  
  free( gen_name );  
  H5Gclose(grid_id);
  
  return grid_num;
}

/**
* \brief Populates a #ch5s_aux_grid struct with info about a grid at a given
*        index
* \note The populated #ch5s_aux_grid struct should be freed with the related
*       function #ch5s_aux_free_grid_info
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the grid to query
* \param[out] info       A pointer to a #ch5s_aux_grid struct to store the
*                        grid information.
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5s_aux_free_grid_info
*/
int ch5s_aux_grid_info(hid_t hdf_file, unsigned int grid_index,
  ch5s_aux_grid *info)
{
  /* NULL these string pointers to ensure their value even with failed results */
  info->label      = NULL;
  info->time_units = NULL;
  info->units      = NULL;
  info->comments   = NULL;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_AUX_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int result = (ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL) == 0);
  H5Gclose(container_id);
  if (result == 0) return 1;
  
  info->time_steps = ch5_nchild_count_children(grid_id);
  
  GET_ATTR(grid_id, H5T_NATIVE_FLOAT, CH5_DELTA_T_ATTR, &(info->time_delta));
  GET_ATTR(grid_id, H5T_NATIVE_FLOAT, CH5_T0_ATTR, &(info->t0));
  GET_STRING_ATTR(grid_id, CH5_LABEL_ATTR,      &(info->label));
  GET_STRING_ATTR(grid_id, CH5_TIME_UNITS_ATTR, &(info->time_units));
  GET_STRING_ATTR(grid_id, CH5_UNITS_ATTR,      &(info->units));
  GET_STRING_ATTR(grid_id, CH5_COMMENTS_ATTR,   &(info->comments));
  
  return 0;
}

/**
* \brief Frees any allocated memory used by a #ch5s_aux_grid struct
* \param[in] info A pointer to a #ch5s_aux_grid struct that needs freeing
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5s_aux_grid_info
*/
int ch5s_aux_free_grid_info(ch5s_aux_grid *info) {
  if (info == NULL) return 1;
  if (info->label      != NULL) free((char*)info->label);
  if (info->time_units != NULL) free((char*)info->time_units);
  if (info->units      != NULL) free((char*)info->units);
  if (info->comments   != NULL) free((char*)info->comments);
  return 0;
}

/**
* \brief Counts the auxiliary grids currently defined
* \param[in] hdf_file The HDF file reference id
* \returns Number of auxiliary grids
* \retval -1 Failure
*/
int ch5s_aux_grid_count(hid_t hdf_file) {
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_AUX_GROUP_NAME);
  if (container_id < 0) return -1;
  int count = ch5_nchild_count_children(container_id);
  H5Gclose(container_id);
  return count;
}

/**
* \brief Writes a whole timestep of data to a grid
*
* This function writes up to three datasets simultaneously.  The formats for
* each type of data are as follows:
*
* Points: size should be <tt>sizeof(float) * np * 3</tt> stored in a single
*         contiguous array.
*
* Elements: the inbound size for this will be variable but should also be
*           stored in a single contiguous array.  See the note about the
*           element data format expected here.
*
* Data: size should be <tt>sizeof(float) * np</tt> stored in a contiguous
*       array.  That is, if data exists, there is one float per point.
*
* \note Element data is not padded and should be provided in the form:
*       <tt>[elem_type, node_0, node_1, ..., node_n, elem_type, node_0, node_1, ... etc.]</tt>
*       but will be padded to the \p max_elem_width size for uniform storage
*       in the HDF file.
*
* \param[in] hdf_file       The HDF file reference id
* \param[in] grid_index     The index of the grid to query
* \param[in] np             The number of points to be written
* \param[in] points         A pointer to the points data to be written
* \param[in] ne             The number of elements to be written.  Can be 0
*                           in which case max_elem_width and elements are
*                           ignored.
* \param[in] max_elem_width The maximum number of nodes for a single element
*                           in the elements data to be written
* \param[in] elements       A pointer to the elements data to be written (see
*                           the note regarding the inbound element data format)
* \param[in] data           (Optional) A pointer to data to be written (pass
*                           NULL to omit)
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5s_aux_write_next(hid_t hdf_file, unsigned int grid_index,
  unsigned int np, float *points, unsigned int ne,
  unsigned int max_elem_width, unsigned int *elements, float *data)
{
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_AUX_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int status = ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL);
  H5Gclose(container_id);
  if (status != 0) return 1;
  
  int count = ch5_nchild_count_children(grid_id);
  char *time_name = ch5_nchild_gen_name(CH5_AUX_TIME_NAME_PREFIX, count, NULL);
  hid_t time_id = ch5_gnrc_open_or_create_group(grid_id, time_name);
  free(time_name);
  H5Gclose(grid_id);
  if (time_id < 0) return 1;
  
  /* Points */
  hid_t points_dset_id = ch5_gnrc_open_or_create_dset(time_id,
    CH5_AUX_POINTS_DSET_NAME, H5T_IEEE_F32LE, 2, (hsize_t[2]){ np, 3 });
  if (points_dset_id < 0) {
    H5Gclose(time_id);
    return 1;
  }
  herr_t write_result = H5Dwrite(points_dset_id, H5T_NATIVE_FLOAT, H5S_ALL,
    H5S_ALL, H5P_DEFAULT, points);
  H5Dclose(points_dset_id);
  if (write_result < 0) {
    H5Gclose(time_id);
    return 1;
  }
  
  /* Elements */
  if (ne > 0) {
    int padded_size = max_elem_width + CH5_ELEM_WIDTH_ADD;
    int *padded_elements = (int*) calloc(sizeof(int), padded_size * ne);
    int elem_index = 0, i, n;
    for (i = 0; i < ne; i++) {
      padded_elements[i * padded_size] = elements[elem_index];
      int elem_size = ch5m_elem_get_width_for_type(elements[elem_index]);
      for (n = 0; n < elem_size; n++)
        padded_elements[i * padded_size + CH5_ELEM_WIDTH_ADD + n] = elements[elem_index + n + 1];
      elem_index += (n+1);
    }
  
    hid_t elems_dset_id = ch5_gnrc_open_or_create_dset(time_id,
      CH5_AUX_ELEMS_DSET_NAME, H5T_STD_U32LE, 2,
      (hsize_t[2]){ ne, (max_elem_width + CH5_ELEM_WIDTH_ADD) });
    if (elems_dset_id < 0) {
      free(padded_elements);
      H5Gclose(time_id);
      return 1;
    }
    write_result = H5Dwrite(elems_dset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL,
      H5P_DEFAULT, padded_elements);
    free(padded_elements);
    H5Dclose(elems_dset_id);
    if (write_result < 0) {
      H5Gclose(time_id);
      return 1;
    }
  }
  
  /* Data */
  if (data != NULL) {
    hid_t data_dset_id = ch5_gnrc_open_or_create_dset(time_id,
      CH5_AUX_DATA_DSET_NAME, H5T_IEEE_F32LE, 1, (hsize_t[1]){ np });
    if (data_dset_id < 0) {
      H5Gclose(time_id);
      return 1;
    }
    write_result = H5Dwrite(data_dset_id, H5T_NATIVE_FLOAT, H5S_ALL,
      H5S_ALL, H5P_DEFAULT, data);
    H5Dclose(data_dset_id);
    if (write_result < 0) {
      H5Gclose(time_id);
      return 1;
    }
  }
  
  H5Gclose(time_id);
  
  return 0;
}

/**
* \brief Fetches information about a given time step in a given grid
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the grid to query
* \param[in]  time_index The time index at which to query
* \param[out] info       A pointer to a #ch5s_aux_time_step struct to populate
*                        with info
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5s_aux_time_step_info(hid_t hdf_file, unsigned int grid_index,
  unsigned int time_index, ch5s_aux_time_step *info)
{
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_AUX_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int status = ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL);
  H5Gclose(container_id);
  if (status != 0) return 1;
  
  hid_t time_id;
  status = ch5_nchild_open_child(grid_id, time_index, &time_id, NULL);
  H5Gclose(grid_id);
  if (status != 0) return 1;
  
  ch5_dataset dset_info;
  status = ch5_gnrc_get_dset_info(time_id, CH5_AUX_POINTS_DSET_NAME,
    &dset_info);
  if (status != 0) {
    H5Gclose(time_id);
    return 1;
  }
  info->num_points = dset_info.count;
  
  htri_t exists = H5Lexists(time_id, CH5_AUX_ELEMS_DSET_NAME, H5P_DEFAULT);
  if (exists >= 1) {
    status = ch5_gnrc_get_dset_info(time_id, CH5_AUX_ELEMS_DSET_NAME,
      &dset_info);
    if (status != 0) {
      H5Gclose(time_id);
      return 1;
    }
    info->num_elements      = dset_info.count;
    info->max_element_width = dset_info.width - CH5_ELEM_WIDTH_ADD;
  }
  else {
    info->num_elements      = 0;
    info->max_element_width = 0;
  }
  
  exists = H5Lexists(time_id, CH5_AUX_DATA_DSET_NAME, H5P_DEFAULT);
  info->has_data = (exists >= 1);
  
  H5Gclose(time_id);
  
  return 0;
}

/**
* \brief Reads data for a given time step
*
* In order to know how much to space to allocate, one should query the
* information of the time step first; see #ch5s_aux_time_step_info for this.
*
* Size of points is <tt>sizeof(float) * 3 * num_points</tt>
*
* Size of elements is <tt>sizeof(unsigned int) * num_elements * max_element_width</tt>
*
* Size of data is <tt>sizeof(float) * num_points</tt> (one data value for each
* point defined).
*
* \note The format for the elements read is the *padded* form where each
*       element is padded with 0 up to \p max_element_width + 2
* \param[in]  hdf_file   The HDF file reference id
* \param[in]  grid_index The index of the grid to read from
* \param[in]  time_index The time index at which to read from
* \param[out] points     (Optional) A pointer to an array of floats large
*                        enough to hold the all the points defined for this
*                        time step.  Passing NULL will skip reading points.
* \param[out] elements   (Optional) A pointer to an array of uints large
*                        enough to hold the all the elements defined for this
*                        time step.  Passing NULL will skip reading elements.
* \param[out] data       (Optional) A pointer to an array of floats large
*                        enough to hold the all the data defined for this
*                        time step.  Passing NULL will skip reading data.
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5s_aux_read(hid_t hdf_file, unsigned int grid_index,
  unsigned int time_index, float *points, unsigned int *elements, float *data)
{
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_AUX_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t grid_id;
  int status = ch5_nchild_open_child(container_id, grid_index, &grid_id, NULL);
  H5Gclose(container_id);
  if (status != 0) return 1;
  
  hid_t time_id;
  status = ch5_nchild_open_child(grid_id, time_index, &time_id, NULL);
  H5Gclose(grid_id);
  if (status != 0) return 1;
  
  herr_t read_status;
  hid_t dset_id;
  /* Points */
  if (points != NULL) {
    dset_id = H5Dopen(time_id, CH5_AUX_POINTS_DSET_NAME, H5P_DEFAULT);
    if (dset_id < 0) {
      H5Gclose(time_id);
      return 1;
    }
    read_status = H5Dread(dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
      H5P_DEFAULT, points);
    H5Dclose(dset_id);
    if (read_status < 0) {
      H5Gclose(time_id);
      return 1;
    }
  }
  
  /* Elements */
  if (elements != NULL) {
    dset_id = H5Dopen(time_id, CH5_AUX_ELEMS_DSET_NAME, H5P_DEFAULT);
    if (dset_id < 0) {
      H5Gclose(time_id);
      return 1;
    }
    read_status = H5Dread(dset_id, H5T_NATIVE_UINT, H5S_ALL, H5S_ALL,
      H5P_DEFAULT, elements);
    H5Dclose(dset_id);
    if (read_status < 0) {
      H5Gclose(time_id);
      return 1;
    }
  }
  
  /* Data */
  if (data != NULL) {
    dset_id = H5Dopen(time_id, CH5_AUX_DATA_DSET_NAME, H5P_DEFAULT);
    if (dset_id < 0) {
      H5Gclose(time_id);
      return 1;
    }
    read_status = H5Dread(dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
      H5P_DEFAULT, data);
    H5Dclose(dset_id);
    if (read_status < 0) {
      H5Gclose(time_id);
      return 1;
    }
  }
  
  H5Gclose(time_id);
  
  return 0;
}

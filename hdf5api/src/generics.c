#include "generics.h"

#include <stdlib.h>
#include <string.h>
#include <H5LTpublic.h>

/**
* \brief Retrieves 2D dataset information
*
* Sets the #ch5_dataset count to the total number of entries and width
* to the number of values per entry.  This function is wrapped by many other
* functions specific to various datasets.
*
* \param[in]  file_id  The HDF file reference id
* \param[in]  path     HDF path to the dataset (relative to #file_id)
* \param[out] out_info Pointer to #ch5_dataset struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5_gnrc_get_dset_info(hid_t file_id, const char *path,
  ch5_dataset *out_info)
{
  int rank;
  herr_t result = H5LTget_dataset_ndims(file_id, path, &rank);
  if (result < 0) return 1;

  hsize_t dimensions[rank];
  result = H5LTget_dataset_info(file_id, path, dimensions, NULL, NULL);
  if (result < 0) return 1;

  out_info->count = dimensions[0];
  out_info->width = (rank > 1) ? dimensions[1] : 1;

  return 0;
}

/**
* \brief Fetches a single-value attribute of a given type
* \warning When getting \p H5T_C_S1 (a c-string) you must supply a NULL
*          pointer to the data argument as this function will allocate the
*          memory required to store the string.
* \note Please refer to the HDF5 documentation for details on values to pass
*       for \p type_id.  These should all use the \p H5T_NATIVE_ prefix with
*       the exception of \p H5T_C_S1
* \param[in]  parent_id An HDF reference ID to the group or dataset that holds
*                       the attribute being fetched
* \param[in]  type_id   An HDF reference ID to the memory type to use when
*                       fetching the attribute value
* \param[in]  name      The name of the attribute to fetch
* \param[out] data      A pointer to the container for the fetched data (only
*                       populated on success)
* \returns Status code
* \retval 2 Attribute not found
* \retval 1 Failure
* \retval 0 Success
*/
int ch5_gnrc_get_attr(hid_t parent_id, hid_t type_id, const char *name,
  void *data)
{
  if (name == NULL) return 1;
  int exists = H5Aexists_by_name(parent_id, ".", name, H5P_DEFAULT);
  if      (exists ==  0) return 2;
  else if (exists == -1) return 1;
  
  int result = 0;
  
  hid_t attr_id = H5Aopen_by_name(parent_id, ".", name, H5P_DEFAULT,
    H5P_DEFAULT);
  if (attr_id < 0) return 1;
  
  hid_t dspace_id = H5Aget_space(attr_id);
  if (dspace_id < 0) {
    H5Aclose(attr_id);
    return 1;
  }
  
  if (H5Tequal(type_id, H5T_C_S1) > 0) {
    type_id = H5Tcopy(H5T_C_S1);
    H5Tset_size(type_id, H5T_VARIABLE);
    /* From this point on type_id is a variable string and will not be
       evaluated as equal to H5T_C_S1 */
    char **read = (char**) calloc(sizeof(char*), 1);
    herr_t result = H5Aread(attr_id, type_id, read);
    H5Tclose(type_id);
    if (result < 0) {
      free(read);
      result = 1;
    }
    else {
      *((char**)data) = (char*) calloc(sizeof(char), strlen(read[0]) + 1);
      strcpy(*((char**)data), read[0]);
      H5Dvlen_reclaim(type_id, dspace_id, H5P_DEFAULT, read);
      free(read);
    }
  }
  else {
    herr_t status = H5Aread(attr_id, type_id, data);
    if (status < 0) result = 1;
  }
  
  H5Aclose(attr_id);
  H5Sclose(dspace_id);
  
  return result;
}

/**
* \brief Sets a single-value attribute of a given type
* \warning Setting an attribute that already exists (by name) will cause the
*          existing attribute and it's type definition to be overwritten.
* \note Please refer to the HDF5 documentation for details on values to pass
*       for \p type_id.  For example: \p H5T_IEEE_F32LE \p H5T_C_S1
*       \p H5T_STD_U32LE etc.
* \param[in] parent_id An HDF reference ID to the group or dataset that holds
*                      the attribute being set
* \param[in] type_id   An HDF reference ID to the memory type to use when
*                      setting the attribute value
* \param[in] name      The name of the attribute to set
* \param[in] data      A pointer to the data to be set on the attribute
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5_gnrc_set_attr(hid_t parent_id, hid_t type_id, const char *name,
  void *data)
{
  if (name == NULL) return 1;
  
  herr_t status;
  if (H5Aexists_by_name(parent_id, ".", name, H5P_DEFAULT) > 0) {
    status = H5Adelete_by_name(parent_id, ".", name, H5P_DEFAULT);
    if (status < 0) return 1;
  }
  
  hid_t space_id = H5Screate_simple(1, (hsize_t[1]){ 1 }, NULL);
  if (space_id < 0) return 1;
  
  if (H5Tequal(type_id, H5T_C_S1) > 0) {
    type_id = H5Tcopy(H5T_C_S1);
    H5Tset_size(type_id, H5T_VARIABLE);
    /* From this point on type_id is a variable string and will not be
       evaluated as equal to H5T_C_S1 */
  }
  
  hid_t attr_id = H5Acreate_by_name(parent_id, ".", name, type_id, space_id,
    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  if (attr_id < 0) {
    H5Sclose(space_id);
    return 0;
  }
  
  if (H5Tis_variable_str(type_id)) {
    const char *cstrdata[1] = { (char*)data };
    status = H5Awrite(attr_id, type_id, cstrdata);
  }
  else
    status = H5Awrite(attr_id, type_id, data);
  
  H5Sclose(space_id);
  H5Aclose(attr_id);
  
  return (status < 0);
}

/**
* \brief Fetches an entire dataset's data into out_data
* \pre \p out_data is expected to be a contiguous array of size
*      <tt>sizeof(TYPE) * count * width</tt> where \p TYPE is the native type
*      that matches the HDF5 memtype passed to the function and \p width and
*      \p count correspond to the values returned from
*      #ch5_gnrc_get_dset_info (or equivalent) for that dataset.
* \note In all cases, supplying a \p memtype of \p H5T_NATIVE_* is strongly
*       recommended.
* \param[in]  file_id  The HDF file reference id
* \param[in]  path     HDF path to the dataset (relative to #file_id)
* \param[in]  memtype  An HDF type identifier describing the output (memory)
*                      type
* \param[out] out_data A pointer to a contiguous array of a type matching
*                      \p memtype
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5_gnrc_get_all(hid_t file_id, const char *path, hid_t memtype,
  void *out_data)
{
  hid_t set_id = H5Dopen(file_id, path, H5P_DEFAULT);
  if (set_id < 0) return 1;
  
  herr_t result = H5Dread(set_id, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, out_data);
  
  H5Dclose(set_id);
  
  return (result < 0);
}

/**
* \brief Fetches a range of data rows from a dataset into out_data.
* \pre \p out_data is expected to be a contiguous array of size
*      <tt>sizeof(TYPE) * width * (end - start + 1)</tt> where \p TYPE is the
*      native type that matches the HDF5 memtype passed to the function and
*      \p width corresponds to the value returned from
*      #ch5_gnrc_get_dset_info (or equivalent) for that dataset.
* \note If a \p width is specified that is less than the dataset's width, the
*       data returned will be the columns from <tt>0...width</tt> from the set.
* \note In all cases, supplying a \p memtype of \p H5T_NATIVE_* is strongly
*       recommended.
* \param[in]  file_id  The HDF file reference id
* \param[in]  path     HDF path to the dataset (relative to #file_id)
* \param[in]  memtype  An HDF type identifier describing the output (memory)
*                      type
* \param[in]  start    The 0-based index to begin fetching at (inclusive)
* \param[in]  end      The 0-based index to stop fetching at (inclusive)
* \param[in]  width    The width of each row to be fetched
* \param[out] out_data A pointer to a contiguous array of a type matching
*                      #memtype
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5_gnrc_get_range(hid_t file_id, const char *path, hid_t memtype,
  int start, int end, int width, void *out_data)
{
  hid_t set_id = H5Dopen(file_id, path, H5P_DEFAULT);
  if (set_id < 0) return 1;
  
  hid_t space_id = H5Dget_space(set_id);
  herr_t result = H5Sselect_hyperslab(space_id, H5S_SELECT_SET,
    (hsize_t[2]){ start, 0 }, NULL, (hsize_t[2]){ end-start+1, width }, NULL);
  if (result < 0) {
    H5Dclose(set_id);
    H5Sclose(space_id);
    return 1;
  }
  
  hid_t memspace_id = H5Screate_simple(2, (hsize_t[2]){ width, end-start+1 },
    NULL);
  result = H5Dread(set_id, memtype, memspace_id, space_id,
    H5P_DEFAULT, out_data);
  
  H5Sclose(memspace_id);
  H5Sclose(space_id);
  H5Dclose(set_id);
  
  return (result < 0);
}

/**
* \brief Opens or creates (if non-existing) a group
* \param[in] parent_id The immediate parent of the group (file or group only)
* \param[in] name      The name for the group (used for lookup too)
* \returns HDF group reference id (lt 0 is failure/error)
*/
hid_t ch5_gnrc_open_or_create_group(hid_t parent_id, const char *name) {
  hid_t group_id;
  htri_t exists = H5Lexists(parent_id, name, H5P_DEFAULT);
  
  if (exists == 1)
    group_id = H5Gopen(parent_id, name, H5P_DEFAULT);
  else
    group_id = H5Gcreate(parent_id, name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    
  return group_id;
}

/**
* \brief Opens or creates (if non-existing) a fixed-size dataset
* \note Fixed-size datasets cannot be compressed with gzip filters
* \param[in] parent_id   The immediate parent of the dataset (file or group only)
* \param[in] name        The name for the dataset to create or open
* \param[in] type_id     The HDF file datatype for the dataset
* \param[in] rank        The number of dimensions for the dataset
* \param[in] dims        An array of dimension sizes whose count matches the
*                        \p rank argument
* \returns HDF dataset reference id (lt 0 is failure/error)
*/
hid_t ch5_gnrc_open_or_create_dset(hid_t parent_id, const char *name,
  hid_t type_id, int rank, hsize_t *dims)
{
  hid_t dset_id;
  herr_t exists = H5LTfind_dataset(parent_id, name);
  if (exists == 1)
    dset_id = H5Dopen(parent_id, name, H5P_DEFAULT);
  else {
    hid_t space_id = H5Screate_simple(rank, dims, NULL);
    
    dset_id = H5Dcreate(parent_id, name, type_id, space_id, H5P_DEFAULT,
      H5P_DEFAULT, H5P_DEFAULT);
    
    H5Sclose(space_id);
  }
  
  return dset_id;
}

/**
* \brief Opens or creates (if non-existing) a rank-2 chunked dataset
* \warning If count_max or width_max is specified and is smaller than the
*          corresponding chunk_* values, the chunk_* values will be clamped to
*          the count or width max.  Beware of creating chunked datasets
*          without an unlimited dimension as resizing becomes impossible.
* \param[in] parent_id   The immediate parent of the dataset (file or group only)
* \param[in] name        The name for the dataset to create or open
* \param[in] type_id     The HDF file datatype for the dataset
* \param[in] rank        The number of dimensions for the dataset
* \param[in] dims        An array of dimension sizes whose count matches the
*                        \p rank argument
* \param[in] max_dims    An array of the maximum dimension sizes whose count
*                        matches the \p rank argument (H5S_UNLIMITED allowed)
* \param[in] chunk_dims  An array of chunk sizes for each dimension whose
*                        count matches the \p rank argument
* \param[in] gzip_level  The level of compression to use (0 or less will cause
*                        the gzip property to be omitted entirely)
* \returns HDF dataset reference id (lt 0 is failure/error)
*/
hid_t ch5_gnrc_open_or_create_chunked_dset(hid_t parent_id, const char *name,
  hid_t type_id, int rank, hsize_t *dims, hsize_t *max_dims,
  hsize_t *chunk_dims, int gzip_level)
{
  hid_t dset_id;
  herr_t exists = H5LTfind_dataset(parent_id, name);
  if (exists == 1) {/* Dataset exists, resize to accomodate incoming data */
    dset_id = H5Dopen(parent_id, name, H5P_DEFAULT);
    if (dset_id < 0) return dset_id;
    herr_t status = H5Dset_extent(dset_id, dims);
    if (status < 0) {
      H5Dclose(dset_id);
      return -1;
    }
  }
  else {/* Create the dataset for the incoming data */
    /* Ensure that chunk sizes don't exceed the maximums (HDF does not allow this) */
    for (int i = 0; i < rank; i++) {
      if ((max_dims[i] != H5S_UNLIMITED) && (max_dims[i] < chunk_dims[i]))
        chunk_dims[i] = max_dims[i];
    }
    
    hid_t space_id = H5Screate_simple(rank, dims, max_dims);
    hid_t property_id = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(property_id, rank, chunk_dims);
    if (gzip_level > 0)
      H5Pset_deflate(property_id, gzip_level);
    
    dset_id = H5Dcreate(parent_id, name, type_id, space_id, H5P_DEFAULT,
      property_id, H5P_DEFAULT);
    
    H5Pclose(property_id);
    H5Sclose(space_id);
  }
  
  return dset_id;
}

/**
* \brief Generates a dataset or group name for a child in the form
*        Prefix000000. (The prefix with a 6 digit zeropadded number).
* \param[in] prefix The name prefix
* \param[in] n      The number of the child
* \param[in] label  use this nuame if supplied    
*/
char* ch5_nchild_gen_name(const char *prefix, int n, const char *label ) {
  if( label )
    return strdup(label);
  int new_total = strlen(prefix) + CH5_NCHILD_ZEROPAD + 1;
  char *name = (char*) malloc(sizeof(char) * new_total);
  sprintf(name, "%s%0" CH5_QUOTE(CH5_NCHILD_ZEROPAD) "d", prefix, n);
  return name;
}

/**
* \brief Creates an n-child "compatible" group with the given name
* \note Groups that are not created with this function have undefined
*       behaviour when used with other \p ch5_nchild_ functions.
* \param[in] hdf_file The HDF file reference ID (can be a file or group)
* \param[in] name     The name for the container
* \returns The HDF reference ID for the newly created container
* \retval Negative values indicate error
*/
hid_t ch5_nchild_create_or_open_container(hid_t hdf_file, const char *name) {
  hid_t group_id;
  
  htri_t exists = H5Lexists(hdf_file, name, H5P_DEFAULT);
  if (exists < 1) {
    hid_t prop_id = H5Pcreate(H5P_GROUP_CREATE);
    H5Pset_link_creation_order(prop_id,
      H5P_CRT_ORDER_TRACKED | H5P_CRT_ORDER_INDEXED );
    group_id = H5Gcreate(hdf_file, name, H5P_DEFAULT, prop_id, H5P_DEFAULT);
    H5Pclose(prop_id);
  }
  else
    group_id = H5Gopen(hdf_file, name, H5P_DEFAULT);
  
  return group_id;
}

/**
* \brief Counts the number of children in the given nchild container
* \param[in] container_id The HDF reference ID for the nchild container
* \returns The number of children defined in the container
* \retval -1 Failure
*/
int ch5_nchild_count_children(hid_t container_id) {
  H5G_info_t info;
  herr_t result = H5Gget_info(container_id, &info);
  if (result < 0) return -1;
  return info.nlinks;
}

/**
* \brief Opens the n-th child in a given container
* \param[in]  container_id The HDF reference index for the container to look
*                          for the child within
* \param[in]  n            The index of the child to open
* \param[out] child_id     A pointer to an HDF reference index that refers to
*                          the opened child (on success)
* \param[out] child_type   An optionally NULL pointer to an H50_type_t which
*                          indicates whether the child is a group or dataset
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5_nchild_open_child(hid_t container_id, int n, hid_t *child_id, H5O_type_t *child_type) {
  _ch5_nchild_iter_data iter_data = { .target_index=n };
  herr_t result = H5Literate(container_id, H5_INDEX_CRT_ORDER, H5_ITER_INC,
    NULL, _ch5_nchild_iter_callback, &iter_data);
  if (result <= 0) return 1;/* Not found or error */
  
  *child_id = iter_data.child_id;
  if (child_type != NULL) *child_type = iter_data.child_type;
  
  return 0;
}

/// @cond INTERNAL

/*
* Callback method used when iterating over n-children.
*/
herr_t _ch5_nchild_iter_callback(hid_t container_id, const char *name,
  const H5L_info_t *info, void *op_data)
{
  if (info->corder_valid == 1) {
    _ch5_nchild_iter_data *iter_data = (_ch5_nchild_iter_data*)op_data;
    if (info->corder == iter_data->target_index) {
      H5O_info_t info;
      herr_t result = H5Oget_info_by_name(container_id, name, &info, H5P_DEFAULT);
      if (result < 0) return -1;
      
      switch (info.type) {
        case H5O_TYPE_GROUP:
          iter_data->child_id = H5Gopen(container_id, name, H5P_DEFAULT);
          break;
        
        case H5O_TYPE_DATASET:
          iter_data->child_id = H5Dopen(container_id, name, H5P_DEFAULT);
          break;
        
        default:
          return -1;/* Only useful if a group or dataset */
      }
      
      iter_data->child_type = info.type;
      return 1;/* Found */
    }
  }
  else return -1;/* Creation orders are invalid, can't reliably fetch surface */
  
  return 0;/* Continue on next child */
}

/// @endcond

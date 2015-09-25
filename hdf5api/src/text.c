#include "text.h"
#include "generics.h"

#include <stdlib.h>
#include <string.h>

/**
* \brief Creates a new text file
* \param[in] hdf_file     The HDF file reference id
* \param[in] label        (Optional) A label for the text file (pass NULL for no label)
* \returns The numeric index for the newly created text file
* \retval -1 Failure
*/
int ch5_text_create(hid_t hdf_file, const char *label) {
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_TEXT_GROUP_NAME);
  if (container_id < 0) return -1;
  
  int file_count = ch5_nchild_count_children(container_id);
  char file_name[1024];
  if( !label )
    sprintf(file_name, "%s%d", CH5_TEXT_FILE_NAME_PREFIX, file_count);
  else
    strncpy( file_name, label, 1024 );
  
  hid_t filetype = H5Tcopy (H5T_C_S1);
  H5Tset_size (filetype, H5T_VARIABLE);

  hid_t text_id = ch5_gnrc_open_or_create_chunked_dset(container_id,
    file_name, filetype, 1, (hsize_t[1]){ 0 }, (hsize_t[1]){ H5S_UNLIMITED },
    (hsize_t[1]){ CH5_TEXT_CHUNK_SIZE }, 0);
  H5Gclose(container_id);
  if (text_id < 0) return -1;
  
  if (label != NULL) {
    int result = ch5_gnrc_set_attr(text_id, H5T_C_S1, CH5_TEXT_LABEL_ATTR,
      (char*)label);
    if (result == 1) {
      H5Dclose(text_id);
      return -1;
    }
  }
  
  H5Dclose(text_id);
  
  return file_count;
}

/**
* \brief Retrieves information about a text file given a text file index
* \param[in]  hdf_file The HDF file reference id
* \param[in]  index    The index of the text file to get info about
* \param[out] info     Pointer to the #ch5s_text_file struct to populate
*                      with the retrieved info
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5_text_info(hid_t hdf_file, unsigned int index, ch5_text_file *info) {
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_TEXT_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t text_id;
  int status = ch5_nchild_open_child(container_id, index, &text_id, NULL);
  H5Gclose(container_id);
  if (status != 0) return 1;
  
  status = ch5_gnrc_get_attr(text_id, H5T_C_S1, CH5_TEXT_LABEL_ATTR,
    &(info->label));
  if (status != 0) return 1;
  
  hsize_t dims[1];
  hid_t space_id = H5Dget_space(text_id);
  H5Sget_simple_extent_dims(space_id, dims, NULL);
  info->storage = H5Dget_storage_size(text_id);
  H5Sclose(space_id);
  info->size = dims[0];
  
  H5Dclose(text_id);
  
  return 0;
}

/**
* \brief Frees any allocated memory used in #ch5s_text_file structs
* \param[in] info A pointer to the #ch5s_text_file struct to free
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5_text_free_info(ch5_text_file *info) {
  if (info == NULL) return 1;
  if (info->label != NULL) free((char*)info->label);
  return 0;
}

/**
* \brief Appends a string to a text file
* \param[in] hdf_file   The HDF file reference id
* \param[in] index The index of the file to append to
* \param[in] in    A pointer to the string to append
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
*/
int ch5_text_append(hid_t hdf_file, unsigned int index, const char *in) {
  if (in == NULL)     return 1;
  int len = strlen(in);
  if (len < 1) return 1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_TEXT_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t text_id;
  int status = ch5_nchild_open_child(container_id, index, &text_id, NULL);
  H5Gclose(container_id);
  if (status != 0) return 1;
  
  hid_t space_id = H5Dget_space(text_id);
  hsize_t dims[1];
  status = H5Sget_simple_extent_dims(space_id, dims, NULL);
  if (status < 0) {
    H5Sclose(space_id);
    H5Dclose(text_id);
    return 1;
  }
  
  /* Resize and reload the dataset space */
  status = H5Dset_extent(text_id, (hsize_t[1]){ dims[0] + 1 });
  if (status < 0) return 1;
  space_id = H5Dget_space(text_id);
  
  /* Select new range in hyperslab and write */
  status = H5Sselect_hyperslab(space_id, H5S_SELECT_SET,
    (hsize_t[1]){ dims[0] }, NULL, (hsize_t[1]){ 1 }, NULL);
  if (status < 0) {
    H5Sclose(space_id);
    H5Dclose(text_id);
    return 1;
  }
  hid_t memspace_id = H5Screate_simple(1, (hsize_t[1]){ 1 }, NULL);

  hid_t memtype = H5Tcopy (H5T_C_S1);
  status = H5Tset_size (memtype, H5T_VARIABLE);

  status = H5Dwrite(text_id, memtype, memspace_id, space_id,
    H5P_DEFAULT, &in);

  H5Sclose(memspace_id);
  H5Sclose(space_id);
  H5Dclose(text_id);
  
  return (status < 0);
}


/**
* \brief Reads back the entire contents of a text file
* 
* Where <tt>info.size</tt> is the size reported from a #ch5_text_file struct.
*
* \param[in]   hdf_file The HDF file reference id
* \param[in]   index    The index of the file to read from
* \param[out]  out      A pointer to a pointer of strings (to be allocated)
*
* \post memory for out is dynamically allocated
* \note *out is a NULL terminated list
*                    
* \retval  0 Success
* \retval -1 Failure
*/
int ch5_text_read(hid_t hdf_file, unsigned int index, char ***out) {
  if (out == NULL) return 1;
  
  hid_t container_id = ch5_nchild_create_or_open_container(hdf_file,
    CH5_TEXT_GROUP_NAME);
  if (container_id < 0) return 1;
  
  hid_t text_id;
  int status = ch5_nchild_open_child(container_id, index, &text_id, NULL);
  H5Gclose(container_id);
  if (status != 0) return 1;
  
  hid_t space_id = H5Dget_space(text_id);
  hsize_t dim;
  status = H5Sget_simple_extent_dims(space_id, &dim, NULL);
  H5Sclose(space_id);
  if (status < 0) {
    H5Dclose(text_id);
    return 1;
  }
  *out = (char **) calloc ( dim+1, sizeof(char *) );
  
  hid_t memtype = H5Tcopy (H5T_C_S1);
  status = H5Tset_size (memtype, H5T_VARIABLE);

  status = H5Dread(text_id, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, *out);
  H5Dclose(text_id);
  if (status < 0) return 1;
  
  return 0;
}

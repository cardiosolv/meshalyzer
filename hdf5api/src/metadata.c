#include "metadata.h"
#include "generics.h"

#include <stdlib.h>
#include <string.h>

/**
* \brief Retrieves the version metadata for the file
* \param[in]  file_id The HDF file reference id
* \param[out] v       Pointer to a #ch5_version struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5_meta_get_version(hid_t file_id, ch5_version *v) {
  hid_t attr_id = H5Aopen_by_name(file_id, "/", CH5_META_VERSION_NAME,
    H5P_DEFAULT, H5P_DEFAULT);
  if (attr_id < 0) return 1;
  
  herr_t result = H5Aread(attr_id, H5T_NATIVE_INT, v);
  
  H5Aclose(attr_id);
  
  return (result < 0);
}

/**
* \brief Set the version metadata for the file
* \param[in] file_id The HDF file reference id
* \param[in] v       Pointer to a #ch5_version struct to write
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5_meta_set_version(hid_t file_id, ch5_version *v) {
  herr_t status;
  
  if (H5Aexists_by_name(file_id, ".", CH5_META_VERSION_NAME, H5P_DEFAULT) > 0) {
    status = H5Adelete_by_name(file_id, ".", CH5_META_VERSION_NAME,
      H5P_DEFAULT);
    if (status < 0) return 1;
  }
    
  hid_t space_id = H5Screate_simple(1, (hsize_t[1]){ 3 }, NULL);
  if (space_id < 0) return 1;
  
  hid_t attr_id = H5Acreate_by_name(file_id, ".", CH5_META_VERSION_NAME,
    H5T_STD_U16LE, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);  
  if (attr_id < 0) {
    H5Sclose(space_id);
    return 1;
  }
  
  status = H5Awrite(attr_id, H5T_NATIVE_INT, v);
  
  H5Sclose(space_id);
  H5Aclose(attr_id);
  
  return (status < 0);
}

/**
* \brief Retrieves the name metadata
* \param[in]  file_id The HDF file reference id
* \param[out] name A pointer to a string pointer: <tt>&(char*)</tt>
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
* \warning The caller is responsible for freeing the returned pointer 
*/
int ch5_meta_get_name(hid_t file_id, char **name) {
  return !(ch5_gnrc_get_attr(file_id, H5T_C_S1, CH5_META_NAME_NAME, name) == 0);
}

/**
* \brief Sets the name metadata
* \param[in]  file_id The HDF file reference id
* \param[out] name A string pointer
* \returns Status code
* \retval 0 Success
* \retval 1 Failure
* \warning The caller is responsible for freeing the returned pointer 
*/
int ch5_meta_set_name(hid_t file_id, const char *name) {
  return !(ch5_gnrc_set_attr(file_id, H5T_C_S1, CH5_META_NAME_NAME, (void*)name) == 0);
}

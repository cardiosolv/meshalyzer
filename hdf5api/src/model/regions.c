#include "regions.h"
#include "../generics.h"

/**
* \brief Writes an entire set of region definitions to the file overwriting
*        any existing regions data.
* \param[in] file_id The HDF file reference id
* \param[in] n       Number of regions to write
* \param[in] points  The regions data in a contiguous array
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_regn_set_all(hid_t file_id, int n, int *regions) {
  hid_t group_id = ch5_gnrc_open_or_create_group(file_id,
    CH5_REGN_GROUP_NAME);
  if (group_id < 0) return 1;
  
  hid_t dset_id = ch5_gnrc_open_or_create_chunked_dset(group_id,
    CH5_REGN_DSET_NAME, H5T_STD_U32LE, 2,
    (hsize_t[2]){ n, 2 },
    (hsize_t[2]){ H5S_UNLIMITED, 2 },
    (hsize_t[2]){ CH5_REGN_CHUNK_SIZE, 2 }, 1);
  if (dset_id < 0) {
    H5Gclose(group_id);
    return 1;
  }
  
  herr_t status = H5Dwrite(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, regions);
  
  H5Dclose(dset_id);
  
  return (status < 0);
}

/**
* \brief Retrieves regions dataset information.
* \param[in]  file_id  The HDF file reference id
* \param[out] out_info Pointer to a #ch5_dataset struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_dset_info
*/
int ch5m_regn_get_info(hid_t file_id, ch5_dataset *out_info) {
  return ch5_gnrc_get_dset_info(file_id, CH5_REGN_DSET_FULL_PATH, out_info);
}

/**
* \brief Reads an entire set of region definitions
* \param[in] file_id The HDF file reference id
* \param[in] points  Pointer to a contiguous array of integers to store regions
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_regn_get_all(hid_t file_id, int *out_regions) {
  return ch5_gnrc_get_all(file_id, CH5_REGN_DSET_FULL_PATH, H5T_NATIVE_INT,
    (void*)out_regions);
}

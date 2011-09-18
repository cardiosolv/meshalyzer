#include "lons.h"
#include "../generics.h"

#include <H5LTpublic.h>

/**
* \brief Writes an entire set of lons overwriting any existing data
* \param[in] file_id  The HDF file reference id
* \param[in] n        Number of points to write
* \param[in] order    The "order" of the lons being written (1 or 2) which
*                     means that an order of 1 will have 3 floats per lon
*                     and an order of 2 means that there will be 6 floats
* \param[in] in_data  The lons data in a contiguous array
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_lons_set_all(hid_t file_id, int n, int order, float *in_data) {
  if (!(order == 1 || order == 2)) {
    fprintf(stderr, "Order must be either 1 or 2\n");
    return 1;
  }
  
  int width = order * 3;
  
  hid_t group_id = ch5_gnrc_open_or_create_group(file_id, CH5_LONS_GROUP_NAME);
  if (group_id < 0) return 1;
  
  hid_t dset_id = ch5_gnrc_open_or_create_chunked_dset(group_id,
    CH5_LONS_DSET_NAME, H5T_IEEE_F32LE, 2,
    (hsize_t[2]){ n, width },
    (hsize_t[2]){ H5S_UNLIMITED, 6 },
    (hsize_t[2]){ CH5_LONS_CHUNK_SIZE, 6 }, 1);
  if (dset_id < 0) {
    H5Gclose(group_id);
    return 1;
  }
  
  herr_t status = H5Dwrite(dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, in_data);
  
  H5Dclose(dset_id);
  H5Gclose(group_id);
  
  return (status < 0);
}

/**
* \brief Retrieves dimension information about the lons dataset
* \param[in]  file_id  The HDF file reference id
* \param[out] out_info Pointer to a #ch5_dataset struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_dset_info
*/
int ch5m_lons_get_info(hid_t file_id, ch5_dataset *out_info) {
  return ch5_gnrc_get_dset_info(file_id, CH5_LONS_FULL_PATH, out_info);
}

/**
* \brief Returns the order of the lons dataset
*
* The order specifies whether there are 1 or 2 vectors stored for each entry
* in the dataset.  The width of the main lons dataset will be 3 when the order
* is 1 and 6 when the order is 2.
*
* \param[in] file_id The HDF file reference id
* \returns Order of the lons dataset (either 1 or 2)
* \retval -1 Failure
*/
int ch5m_lons_get_order(hid_t file_id) {
  ch5_dataset info;
  int result = (ch5m_lons_get_info(file_id, &info) == 0);
  if (result == 0) return -1;
  
  switch (info.width) {
    case 3:
      return 1;
    
    case 6:
      return 2;
    
    default:
      fprintf(stderr, "Bad width for lons dataset encountered: got %d but expected 3 or 6\n", info.width);
      return -1;
  }
}

/**
* \brief  Fetches the entire dataset's data
*
* \p out_data is expected to be a contiguous array of size
* <tt>sizeof(float) * count * width</tt> where width and count correspond to
* the values returned from #ch5m_lons_get_info.
*
* \param[in]  file_id  The HDF file reference id
* \param[out] out_data Pointer to a contiguous array of integers
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_all
*/
int ch5m_lons_get_all(hid_t file_id, float *out_data) {
  return ch5_gnrc_get_all(file_id, CH5_LONS_FULL_PATH, H5T_NATIVE_FLOAT,
    (void*)out_data);
}

/**
* \brief Fetches a range of data from the dataset
*
* Similar to #ch5m_lons_get_all with the exception that \p out_data should be
* of size <tt>sizeof(float) * width * (end - start + 1)</tt>.
*
* \param[in]  file_id  The HDF file reference id
* \param[in]  start    The 0-based index to begin fetching at (inclusive)
* \param[in]  end      The 0-based index to stop fetching at (inclusive)
* \param[out] out_data A pointer to a contiguous array of floats
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_range
*/
int ch5m_lons_get_range(hid_t file_id, int start, int end, float *out_data) {
  return ch5_gnrc_get_range(file_id, CH5_LONS_FULL_PATH, H5T_NATIVE_FLOAT,
    start, end, 3, (void*)out_data);
}

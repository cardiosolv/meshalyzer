#include "points.h"
#include "../generics.h"

#include <H5LTpublic.h>

/**
* \brief Writes an entire set of points to the file overwriting any existing
*        points data.
* \param[in] file_id The HDF file reference id
* \param[in] n       Number of points to write
* \param[in] points  The points data in a contiguous array
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_pnts_set_all(hid_t file_id, int n, float *points) {
  hid_t group_id = ch5_gnrc_open_or_create_group(file_id,
    CH5_POINTS_GROUP_NAME);
  if (group_id < 0) return 1;
  
  hid_t dset_id = ch5_gnrc_open_or_create_chunked_dset(group_id,
    CH5_POINTS_DSET_NAME, H5T_IEEE_F32LE, 2,
    (hsize_t[2]){ n, 3 },
    (hsize_t[2]){ H5S_UNLIMITED, 3 },
    (hsize_t[2]){ CH5_POINTS_CHUNK_SIZE, 3 }, 1);
  if (dset_id < 0) {
    H5Gclose(group_id);
    return 1;
  }
  
  herr_t status = H5Dwrite(dset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, points);
  
  H5Dclose(dset_id);
  
  return (status < 0);
}

/**
* \brief Retrieves points dataset information.
* \param[in]  file_id  The HDF file reference id
* \param[out] out_info Pointer to a #ch5_dataset struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_dset_info
*/
int ch5m_pnts_get_info(hid_t file_id, ch5_dataset *out_info) {
  return ch5_gnrc_get_dset_info(file_id, CH5_POINTS_FULL_PATH, out_info);
}

/**
* \brief Fetches the entire dataset's data
*
* \p out_data is expected to be a contiguous array of size
* <tt>sizeof(float) * count * width</tt> where \p width and \p count
* correspond to the values returned from #ch5m_pnts_get_info.
*
* \param[in]  file_id  The HDF file reference id
* \param[out] out_data Pointer to a contiguous array of floats
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_all
*/
int ch5m_pnts_get_all(hid_t file_id, float *out_data) {
  return ch5_gnrc_get_all(file_id, CH5_POINTS_FULL_PATH, H5T_NATIVE_FLOAT,
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
int ch5m_pnts_get_range(hid_t file_id, int start, int end, float *out_data) {
  return ch5_gnrc_get_range(file_id, CH5_POINTS_FULL_PATH, H5T_NATIVE_FLOAT,
    start, end, 3, (void*)out_data);
}

#include "connections.h"
#include "../generics.h"

/**
* \brief Retrieves connections dataset information
* \param[in]  file_id  The HDF file reference id
* \param[out] out_info Pointer to #ch5_dataset struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_dset_info
*/
int ch5m_conn_get_info(hid_t file_id, ch5_dataset *out_info) {
  return ch5_gnrc_get_dset_info(file_id, CH5_CONNECTIONS_FULL_PATH, out_info);
}

/**
* \brief Fetches the entire set of data from the connections dataset
*
* \p out_data is expected to be a contiguous array of size
* <tt>sizeof(int) * count * width</tt> where \p width and \p count
* correspond to the values returned from #ch5m_conn_get_info.  See the
* parameter definitions for inbound connections in #ch5m_conn_set_all for
* details on the format of this data.
*
* \param[in]  file_id  The HDF file reference id
* \param[out] out_data Pointer to a contiguous array of int
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5m_conn_set_all, ch5_gnrc_get_all
*/
int ch5m_conn_get_all(hid_t file_id, int *out_data) {
  return ch5_gnrc_get_all(file_id, CH5_CONNECTIONS_FULL_PATH, H5T_NATIVE_INT,
    out_data);
}

/**
* \brief Writes a full set of connection data
* \param[in] file_id The HDF file reference id
* \param[in] n       The number of connections defined
* \param[in] cables  A pointer to an array of integers of size n * 2 where for
*                    each connection i, element i*2 is the starting vertex
*                    and elements i*2 + 1 is the ending vertex.
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_conn_set_all(hid_t file_id, int n, int *connections) {
  hid_t group_id = ch5_gnrc_open_or_create_group(file_id,
    CH5_CONNECTIONS_GROUP_NAME);
  if (group_id < 0) return 1;
  
  hid_t dset_id = ch5_gnrc_open_or_create_chunked_dset(group_id,
    CH5_CONNECTIONS_DSET_NAME, H5T_STD_U32LE, 2,
    (hsize_t[2]){ n, 2 },
    (hsize_t[2]){ H5S_UNLIMITED, 2 },
    (hsize_t[2]){ CH5_CONNECTIONS_CHUNK_SIZE, 2 }, 1);
  if (dset_id < 0) {
    H5Gclose(group_id);
    return 1;
  }
  
  herr_t status = H5Dwrite(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, connections);
  
  H5Dclose(dset_id);
  
  return (status < 0);
}

/*
* TODO write tests for this function and complete it's implementation
*/
int ch5m_conn_get_range(hid_t file_id, int start, int end, int *out_data) {
  return 0;
}
#include "file.h"

/**
* \brief Opens an HDF file at the given path
* \param[in]  path    Path to the HDF file to open
* \param[out] file_id Pointer to an hid_t
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_close
*/
int ch5_open(const char *path, hid_t *file_id) {
  hid_t id = H5Fopen(path, H5F_ACC_RDWR, H5P_DEFAULT);
  if (id < 0) return 1;
  *file_id = id;
  return 0;
}

/**
* \brief Creates a new HDF file at the given path
* \note Will overwrite/truncate an existing file at the given path
* \param[in]  path    Path to the HDF file to create
* \param[out] file_id Pointer to an hid_t
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_close
*/
int ch5_create(const char *path, hid_t *file_id) {
  hid_t id = H5Fcreate(path, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (id < 0) return 1;
  *file_id = id;
  return 0;
}

/**
* \brief Closes ab HDF5 file given an HDF identifier
* \param[in] file_id The HDF file reference id
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_open
*/
int ch5_close(hid_t file_id) {
  herr_t result = H5Fclose(file_id);
  return (result < 0);
}

#include "elements.h"
#include "../generics.h"
#include "../utils.h"

#include <stdlib.h>
#include <string.h>
#include <H5LTpublic.h>


/// @cond INTERNAL

static int _ch5m_elem_hash_prefix(const char *prefix);
static void _ch5m_elem_build_prefix_hash();

/// @endcond

/**
* \var char prefixes[CH5_ELEM_PRIMITIVE_COUNT][3]
* \brief Lookup table of prefixes.
* \details Indexes match with the subsequent widths and dataset names defined
*          #widths and #dset_names.  The order here is defined by the
*          ch5m_element_type enum and any change in order here should be
*          reflected both in the ch5m_element_type enum and the other lookup
*          tables defined here.
*/
char prefixes[CH5_ELEM_PRIMITIVE_COUNT][3] =
  { "Ln", "Tr", "Qd", "Tt", "Py", "Pr", "Hx" };

static int *prefix_hash;

/*
* Lookup table of the widths of primitives sans-region data (that is, only the
* number of nodes required to describe them). The order here is defined by the
* ch5m_element_type enum and any change in order here should be reflected both
* in the ch5m_element_type enum and the other lookup tables defined here.
*/
int widths[CH5_ELEM_PRIMITIVE_COUNT] = { 2, 3, 4, 4, 5, 6, 8 };

/**
* \brief Writes an entire set of element data to the global elements dataset
* \param[in] file_id   The HDF file reference id
* \param[in] n         Total number of elements to be stored
* \param[in] max_width Should be the number of vertices in the most complex
*                      primitive to be stored and should not exceed
*                      \p CH5_ELEM_MAX_VERTEX_COUNT
* \param[in] in_elem   A contiguous array of integers
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5m_elem_set_all_on_dset
*/
int ch5m_elem_set_all(hid_t file_id, int n, int max_width, int *in_elem) {
  hid_t dset_id = ch5m_elem_create_or_open_dset(file_id, CH5_ELEM_DSET_NAME,
    n, max_width);
  if (dset_id < 0) return 1;
  
  int result = ch5m_elem_set_all_on_dset(dset_id, n, max_width, in_elem);
  
  H5Dclose(dset_id);
  
  return result;
}

/**
* \brief Writes data directly to the given dataset
* \param[in] file_id   The HDF file reference id
* \param[in] n         Total number of elements to be stored
* \param[in] max_width Should be the number of vertices in the most complex
*                      primitive to be stored and should not exceed
*                      \p CH5_ELEM_MAX_VERTEX_COUNT
* \param[in] in_elem   A contiguous array of integers
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5m_elem_set_all
*/
int ch5m_elem_set_all_on_dset(hid_t dset_id, int n, int max_width, int *in_elem) {
  if (max_width > CH5_ELEM_MAX_VERTEX_COUNT) return 1;
  herr_t status = H5Dset_extent(dset_id,
    (hsize_t[2]){ n, max_width + CH5_ELEM_WIDTH_ADD });
  if (status < 0) return 1;
  herr_t result = H5Dwrite(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, in_elem);
  return (result < 0);
}

/**
* \brief Retrieves dimension information about the global elements dataset
* \param[in]  file_id  The HDF file reference id
* \param[out] out_info Pointer to a #ch5_dataset struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_dset_info
*/
int ch5m_elem_get_info(hid_t file_id, ch5_dataset *out_info) {
  return ch5_gnrc_get_dset_info(file_id, CH5_ELEM_DSET_FULL_PATH, out_info);
}

/**
* \brief Fetches the entire set of global elements
*
* \p out_elem is expected to be a contiguous array of integers of size
* <tt>sizeof(int) * count * width</tt> where \p width and \p count correspond
* to the values returned from #ch5m_elem_get_info.
*
* \param[in]  file_id  The HDF file reference id
* \param[out] out_elem Pointer to a contiguous array of ints
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5m_elem_get_all_by_dset
*/
int ch5m_elem_get_all(hid_t file_id, int *out_elem) {
  hid_t dset_id = H5Dopen(file_id, CH5_ELEM_DSET_FULL_PATH, H5P_DEFAULT);
  if (dset_id < 0) return 1;
  int result = ch5m_elem_get_all_by_dset(dset_id, out_elem);
  H5Dclose(dset_id);
  return result;
}

/**
* \brief Fetches the entire set of elements from a given dataset
* \note See the #ch5m_elem_get_all documentation for data format details
* \param[in]  file_id  The HDF file reference id
* \param[out] out_elem Pointer to a contiguous array of ints
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5m_elem_get_all
*/
int ch5m_elem_get_all_by_dset(hid_t dset_id, int *out_elem) {
  hsize_t dims[1];
  herr_t status = H5LTget_dataset_info(dset_id, ".", dims, NULL, NULL);
  if (status < 0) return 1;
  
  int result = H5Dread(dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, out_elem);
  
  return (result < 0);
}

/**
* \brief Returns an element's type index by a two-letter prefix
* \note Prefixes are defined in the Meshalyzer manual and can be found in the
*       definition for <tt>static char prefixes</tt>
* \param[in] prefix A two-char string containing the element prefix
* \returns Element type index (0+)
* \retval -1 Failure
*/
ch5m_element_type ch5m_elem_get_type_by_prefix(const char *prefix) {
  if (prefix_hash == NULL) _ch5m_elem_build_prefix_hash();
  return prefix_hash[_ch5m_elem_hash_prefix(prefix)];
}

/**
* \brief Returns the number of vertices for a given element type
* \param[in] type Numeric index of the element type
* \returns Number of vertices for the given element type (0+)
* \retval -1 Failure
*/
int ch5m_elem_get_width_for_type(ch5m_element_type type) {
  if ((type < 0) || (type >= CH5_ELEM_PRIMITIVE_COUNT)) return -1;
  return widths[type];
}

/**
* \brief Convenience method for creating or openening element datasets
*
* Attempts to open the given dataset by name and if it does not exist creates
* it with the given parameters.  If the dataset exists and \p n is not the
* same as the current count of elements in the dataset, it will be resized to
* the new \p n value.
*
* \param[in] parent_id The HDF id for the dataset container (file or group only)
* \param[in] name      Name for the dataset
* \param[in] n         Number of elements to be stored
* \param[in] max_width Defines the maximum number of vertices in the most
*                      complex element being stored (see #ch5m_elem_set_all)
* \returns An HDF dataset reference id (lt 0 is failure/error)
*/
hid_t ch5m_elem_create_or_open_dset(hid_t parent_id, const char *name, int n,
  int max_width)
{
  return ch5_gnrc_open_or_create_chunked_dset(parent_id, name, H5T_STD_U32LE, 2,
    (hsize_t[2]){ n, max_width + CH5_ELEM_WIDTH_ADD },
    (hsize_t[2]){ H5S_UNLIMITED, H5S_UNLIMITED },
    (hsize_t[2]){ CH5_ELEM_CHUNK_SIZE, CH5_ELEM_MAX_VERTEX_COUNT + CH5_ELEM_WIDTH_ADD }, 1);
}

/// @cond INTERNAL

int _ch5m_elem_hash_prefix(const char *prefix) {
  return ((prefix[0] ^ (prefix[1] >> 1)) & 0xf) - 3;
}

void _ch5m_elem_build_prefix_hash() {
  prefix_hash = (int*) malloc(sizeof(int) * 13);
  for (int i = 0; i < CH5_ELEM_PRIMITIVE_COUNT; i++)
    prefix_hash[_ch5m_elem_hash_prefix(prefixes[i])] = i;
}

/// @endcond

#include "surfaces.h"
#include "elements.h"
#include "../generics.h"

#include <stdlib.h>

/**
* \brief Creates a surface with the given parameters
* \param[in] file_id   The HDF file reference id
* \param[in] in_name   (Optional) The name for the surface, NULL for none
* \param[in] count     The number of elements to be written
* \param[in] max_width Defines the maximum number of vertices in the most
*                      complex element being stored (see #ch5m_elem_set_all)
* \returns The newly created surface's index number
* \retval -1 Failure
*/
int ch5m_surf_create(hid_t file_id, char *in_name, int count, int max_width) {
  hid_t group_id = ch5_nchild_create_or_open_container(file_id, CH5_SURF_GROUP_NAME);
  if (group_id < 0) return -1;
  
  int surf_count = ch5m_surf_get_count(file_id);
  if (surf_count == -1) {
    H5Gclose(group_id);
    return -1;
  }
  
  int retval = -1;
  char *dset_name = ch5_nchild_gen_name(CH5_SURF_CHILD_PREFIX, surf_count);
  hid_t dset_id = ch5m_elem_create_or_open_dset(group_id, dset_name, count, max_width);
  if (dset_id >= 0) {
    if (in_name != NULL) ch5_gnrc_set_attr(dset_id, H5T_C_S1, CH5_SURF_NAME_ATTR, in_name);
    retval = surf_count;
  }
  
  H5Dclose(dset_id);
  H5Gclose(group_id);
  
  if (dset_name != NULL) free(dset_name);
  
  return retval;
}

/**
* \brief Retrieves the number of surfaces stored
* \param[in] file_id The HDF file reference id
* \returns Number of surfaces stored
* \retval -1 Failure
*/
int ch5m_surf_get_count(hid_t file_id) {
  hid_t group_id = H5Gopen(file_id, CH5_SURF_GROUP_PATH, H5P_DEFAULT);
  if (group_id < 0) return -1;
  
  int children = ch5_nchild_count_children(group_id);
  
  H5Gclose(group_id);
  
  return children;
}

/**
* \brief Opens a surface by it's numeric id
* \param[in]  file_id The HDF file reference id
* \param[in]  n       0-based index of the surface not exeeding the value
*                     returned from #ch5m_surf_get_count - 1
* \param[out] out_id  The resulting HDF group reference id for the surface
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5m_surf_close, ch5m_surf_get_count
*/
int ch5m_surf_open(hid_t file_id, int n, hid_t *out_id) {
  hid_t group_id = H5Gopen(file_id, CH5_SURF_GROUP_PATH, H5P_DEFAULT);
  if (group_id < 0) return 1;
  
  int result = (ch5_nchild_open_child(group_id, n, out_id, NULL) == 0);
  if (result == 0) return 1;/* Not found or error */
  
  H5Gclose(group_id);
  
  return 0;
}

/**
* \brief Closes a surface given it's HDF dataset id.
* \param[in] surf_id The HDF group reference id for the surface
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5m_surf_open
*/
int ch5m_surf_close(hid_t surf_id) {
  H5Dclose(surf_id);
  return 0;
}

/**
* \brief Fetches the name of a surface identified by it's ID.
* \note The memory required to store the string will be allocated by this
*       function and the user is responsible for freeing this memory.
* \param[in]  surf_id The HDF group reference id for the surface
* \param[out] name    A pointer to a string to store the name
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_surf_get_name(hid_t surf_id, char **name) {
  return ch5_gnrc_get_attr(surf_id, H5T_C_S1, CH5_SURF_NAME_ATTR, name);
}

/**
* \brief Retrieves element data information for the given surface
* \note This method will set the width of out_info to -1 as the width of types
*       is variable.
* \param[in]  surf_id  The HDF group reference id for the surface
* \param[out] out_info Pointer to a #ch5_dataset struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_dset_info
*/
int ch5m_surf_get_elem_info(hid_t surf_id, ch5_dataset *out_info) {
  return ch5_gnrc_get_dset_info(surf_id, ".", out_info);
}

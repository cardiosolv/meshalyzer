#include "purkinjes.h"
#include "../generics.h"

#include <stdlib.h>

/**
* \brief Writes a full series of Purkinje fibres
* \note This function expects that the \p nodes member of each
*       #ch5m_pkje_cable struct is simply an offset pointer to a fully
*       contiguous array of nodes for all fibres.  This is the same way that
*       #ch5m_pkje_get_all returns its data.  As such, this function will
*       reference the nodes array from the first cable's \p nodes member.
* \warning If the details in the note above are not followed, bad things will
*          happen!
* \param[in] file_id The HDF file reference id
* \param[in] n       Number of cables in \p cables
* \param[in] cables  Array of #ch5m_pkje_cable structs to be written
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_pkje_set_all(hid_t file_id, int n, ch5m_pkje_cable *cables) {
  // Write info
  hid_t info_dset_id = _ch5m_pjke_info_dataset(file_id, n);
  if (info_dset_id < 0) return 1;
  
  hid_t info_type_id   = _ch5m_pkje_create_info_compound_type(0);
  if (info_type_id < 0) {
    H5Dclose(info_dset_id);
    return 1;
  }
  
  int result = H5Dwrite(info_dset_id, info_type_id, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, cables);
  
  H5Dclose(info_dset_id);
  H5Tclose(info_type_id);
  
  if (result < 0) return 1;
  
  // Write offsets and count nodes
  int node_count = cables[0].node_count;
  int *offsets = (int*) calloc(sizeof(int), n);
  for (int i = 1; i < n; i++) {
    offsets[i] = offsets[i-1] + cables[i-1].node_count;
    node_count += cables[i].node_count;
  }
  hid_t offset_dset_id = _ch5m_pkje_offsets_dataset(file_id, n);
  if (offset_dset_id < 0) {
    free(offsets);
    return 1;
  }
  
  result = H5Dwrite(offset_dset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, offsets);
  
  free(offsets);
  H5Dclose(offset_dset_id);
  
  if (result < 0) return 1;
  
  // Write nodes
  hid_t nodes_dset_id = _ch5m_pkje_nodes_dataset(file_id, node_count);
  if (nodes_dset_id < 0) return 1;
  
  result = H5Dwrite(nodes_dset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, cables[0].nodes[0]);
  
  H5Dclose(nodes_dset_id);
  
  return (result < 0);
}

/**
* \brief Fetches information about the Purkinjes dataset
* \note Only the \p count member of the #ch5_dataset struct is valid;
*       the \p width member will be -1.
* \param[in]  file_id  The HDF file reference id
* \param[out] out_info Pointer to a #ch5_dataset struct
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
* \sa ch5_gnrc_get_dset_info
*/
int ch5m_pkje_get_info(hid_t file_id, ch5_dataset *out_info) {
  int result = ch5_gnrc_get_dset_info(file_id, CH5_PKJE_INFO_DSET_FULL_PATH,
    out_info);
  if (result == 0) out_info->width = -1;
  return result;
}

/**
* \brief Reads a full series of Purkinje fibres
* \warning You must free the data written to out_cables with the
*          #ch5m_pkje_free_cable_set function!
* \param[in] file_id    The HDF file reference id
* \param[in] out_cables Pointer to an array of ch5m_pkje_cable structs to
*                       read into of size <tt>sizeof(ch5m_pkje_cable) *
*                       count</tt> where \p count is the value from the result
*                       of #ch5m_pkje_get_info
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_pkje_get_all(hid_t file_id, ch5m_pkje_cable *out_cables) {
  htri_t exists;
  hid_t group_id, info_dset_id, nodes_dset_id;
  
  exists = H5Lexists(file_id, CH5_PKJE_GROUP_NAME, H5P_DEFAULT);
  if (exists != 1) return 1;
  group_id = H5Gopen(file_id, CH5_PKJE_GROUP_NAME, H5P_DEFAULT);
  
  exists = H5Lexists(group_id, CH5_PKJE_INFO_DSET_NAME, H5P_DEFAULT);
  if (exists != 1) {
    H5Gclose(group_id);
    return 1;
  }
  info_dset_id = H5Dopen(group_id, CH5_PKJE_INFO_DSET_NAME, H5P_DEFAULT);
  
  exists = H5Lexists(group_id, CH5_PKJE_NODES_DSET_NAME, H5P_DEFAULT);
  if (exists != 1) {
    H5Dclose(info_dset_id);
    H5Gclose(group_id);
    return 1;
  }
  nodes_dset_id = H5Dopen(group_id, CH5_PKJE_NODES_DSET_NAME, H5P_DEFAULT);
  
  hid_t memtype = _ch5m_pkje_create_info_compound_type(0);
  int status = H5Dread(info_dset_id, memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT,
    out_cables);
  if (status < 0) {
    H5Tclose(memtype);
    H5Dclose(nodes_dset_id);
    H5Dclose(info_dset_id);
    H5Gclose(group_id);
    return 1;
  }
  
  ch5_dataset info_info, nodes_info;
  ch5_gnrc_get_dset_info(group_id, CH5_PKJE_INFO_DSET_NAME, &info_info);
  ch5_gnrc_get_dset_info(group_id, CH5_PKJE_NODES_DSET_NAME, &nodes_info);
  double *nodes_raw = (double*) malloc(sizeof(double) * nodes_info.count * nodes_info.width);
  status = H5Dread(nodes_dset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
    H5P_DEFAULT, nodes_raw);
  if (status < 0) {
    free(nodes_raw);
    H5Tclose(memtype);
    H5Dclose(nodes_dset_id);
    H5Dclose(info_dset_id);
    H5Gclose(group_id);
    return 1;
  }
  
  /* Pair up a pseudo 2D array to cables for easy access. */
  double **nodes = (double**) calloc(sizeof(double*), nodes_info.count);
  for (int n = 0; n < nodes_info.count; n++)
    nodes[n] = &nodes_raw[n*nodes_info.width];
  for (int i = 0, node_count = 0; i < info_info.count; i++) {
    out_cables[i].nodes = &nodes[node_count];
    node_count += out_cables[i].node_count;
  }
  
  H5Tclose(memtype);
  H5Dclose(nodes_dset_id);
  H5Dclose(info_dset_id);
  H5Gclose(group_id);
  
  return 0;
}

/**
* \brief Handles freeing the memory for a set of cables
* \details This function will free the cables and the associated nodes.
* \warning This function must be provided the entire set of cables (that is, a
*          pointer to the first cable returned in the fetch call).
* \param[in] cables  Array of #ch5m_pkje_cable structs to be freed (see note)
* \returns Status code
* \retval 1 Failure
* \retval 0 Success
*/
int ch5m_pkje_free_cable_set(ch5m_pkje_cable *cables) {
  if (cables != NULL) {
    if (cables[0].nodes != NULL) {
      if (cables[0].nodes[0] != NULL) free(cables[0].nodes[0]);
      free(cables[0].nodes);
    }
    free(cables);
    return 0;
  }
  return 1;
}

/// @cond INTERNAL

/* Returns 1 if same, 0 otherwise. */
int _ch5m_pkje_compare_cable_info(
  ch5m_pkje_cable *a,
  ch5m_pkje_cable *b)
{
  if (a == b) return 1;
  if ((a->index      != b->index)      ||
      (a->fathers[0] != b->fathers[0]) ||
      (a->fathers[1] != b->fathers[1]) ||
      (a->sons[0]    != b->sons[0])    ||
      (a->sons[1]    != b->sons[1])    ||
      (a->node_count != b->node_count) ||
      (a->radius     != b->radius)     ||
      (a->R_gj       != b->R_gj)       ||
      (a->sigma_i    != b->sigma_i))   { return 0; }
  return 1;
}

/* type: 0 = memory, 1 = file */
hid_t _ch5m_pkje_create_info_compound_type(int type) {
  hid_t id = H5Tcreate(H5T_COMPOUND, sizeof(ch5m_pkje_cable));
  
  H5Tinsert(id, "cable index",
    (type) ? 0             : HOFFSET(ch5m_pkje_cable, index),
    (type) ? H5T_STD_U32LE : H5T_NATIVE_INT);
  
  H5Tinsert(id, "father 0",
    (type) ? 4             : HOFFSET(ch5m_pkje_cable, fathers[0]),
    (type) ? H5T_STD_I32LE : H5T_NATIVE_INT);
  H5Tinsert(id, "father 1",
    (type) ? 4+4           : HOFFSET(ch5m_pkje_cable, fathers[1]),
    (type) ? H5T_STD_I32LE : H5T_NATIVE_INT);
  
  H5Tinsert(id, "son 0",
    (type) ? 4+4+4         : HOFFSET(ch5m_pkje_cable, sons[0]),
    (type) ? H5T_STD_I32LE : H5T_NATIVE_INT);
  H5Tinsert(id, "son 1",
    (type) ? 4+4+4+4       : HOFFSET(ch5m_pkje_cable, sons[1]),
    (type) ? H5T_STD_I32LE : H5T_NATIVE_INT);
    
  H5Tinsert(id, "node count",
    (type) ? 16+4          : HOFFSET(ch5m_pkje_cable, node_count),
    (type) ? H5T_STD_U32LE : H5T_NATIVE_INT);
    
  H5Tinsert(id, "radius",
    (type) ? 16+4+4         : HOFFSET(ch5m_pkje_cable, radius),
    (type) ? H5T_IEEE_F32LE : H5T_NATIVE_FLOAT);
  
  H5Tinsert(id, "gap junction resistance",
    (type) ? 16+4+4+4       : HOFFSET(ch5m_pkje_cable, R_gj),
    (type) ? H5T_IEEE_F32LE : H5T_NATIVE_FLOAT);
  
  H5Tinsert(id, "intracellular conductivity",
    (type) ? 32             : HOFFSET(ch5m_pkje_cable, sigma_i),
    (type) ? H5T_IEEE_F32LE : H5T_NATIVE_FLOAT);
  
  return id;
}

hid_t _ch5m_pjke_info_dataset(hid_t file_id, int size) {
  hid_t group_id = ch5_gnrc_open_or_create_group(file_id, CH5_PKJE_GROUP_NAME);
  hid_t type_id  = _ch5m_pkje_create_info_compound_type(1);
  hid_t dset_id  = ch5_gnrc_open_or_create_chunked_dset(group_id,
    CH5_PKJE_INFO_DSET_NAME, type_id, 1,
    (hsize_t[1]){ size },
    (hsize_t[1]){ H5S_UNLIMITED },
    (hsize_t[1]){ CH5_PKJE_CHUNK_SIZE }, 5);
  H5Tclose(type_id);
  H5Gclose(group_id);
  return dset_id;
}

hid_t _ch5m_pkje_offsets_dataset(hid_t file_id, int size) {
  hid_t group_id = ch5_gnrc_open_or_create_group(file_id, CH5_PKJE_GROUP_NAME);
  hid_t dset_id  = ch5_gnrc_open_or_create_chunked_dset(group_id,
    CH5_PKJE_OFFSETS_DSET_NAME, H5T_STD_U32LE, 1,
    (hsize_t[1]){ size },
    (hsize_t[1]){ H5S_UNLIMITED },
    (hsize_t[1]){ CH5_PKJE_CHUNK_SIZE }, 5);
  H5Gclose(group_id);
  return dset_id;
}

hid_t _ch5m_pkje_nodes_dataset(hid_t file_id, int size) {
  hid_t group_id = ch5_gnrc_open_or_create_group(file_id, CH5_PKJE_GROUP_NAME);
  hid_t dset_id = ch5_gnrc_open_or_create_chunked_dset(group_id,
    CH5_PKJE_NODES_DSET_NAME, H5T_IEEE_F64LE, 2,
    (hsize_t[2]){ size, 3 },
    (hsize_t[2]){ H5S_UNLIMITED, 3 },
    (hsize_t[2]){ CH5_PKJE_CHUNK_SIZE, 3 }, 5);
  H5Gclose(group_id);
  return dset_id;
}

/// @endcond

/**
* \file purkinjes.h
* \brief Functions for reading and writing Purkinje fibre data
*/
#ifndef _CH5M_PURKINJES_H
#define _CH5M_PURKINJES_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_PKJE_GROUP_NAME
* \brief Name for the Purkinje group
* \hideinitializer
*/
#define CH5_PKJE_GROUP_NAME "purkinjes"

/**
* \def CH5_PKJE_GROUP_FULL_PATH
* \brief Full path to the Purkinje group from the root
* \hideinitializer
*/
#define CH5_PKJE_GROUP_FULL_PATH "/" CH5_PKJE_GROUP_NAME

/**
* \def CH5_PKJE_INFO_DSET_NAME
* \brief Name for the info dataset
* \hideinitializer
*/
#define CH5_PKJE_INFO_DSET_NAME "info"

/**
* \def CH5_PKJE_INFO_DSET_FULL_PATH
* \brief Full path to the info dataset from the root
* \hideinitializer
*/
#define CH5_PKJE_INFO_DSET_FULL_PATH CH5_PKJE_GROUP_FULL_PATH "/" CH5_PKJE_INFO_DSET_NAME

/**
* \def CH5_PKJE_NODES_DSET_NAME
* \brief Name for the nodes dataset
* \hideinitializer
*/
#define CH5_PKJE_NODES_DSET_NAME "nodes"

/**
* \def CH5_PKJE_NODES_DSET_FULL_PATH
* \brief Full path to the nodes dataset from the root
* \hideinitializer
*/
#define CH5_PKJE_NODES_DSET_FULL_PATH CH5_PKJE_GROUP_FULL_PATH "/" CH5_PKJE_NODES_DSET_NAME

/**
* \def CH5_PKJE_OFFSETS_DSET_NAME
* \brief Name for the offsets dataset
* \hideinitializer
*/
#define CH5_PKJE_OFFSETS_DSET_NAME "offsets"

/**
* \def CH5_PKJE_OFFSETS_DSET_FULL_PATH
* \brief Full path to the offsets dataset from the root
* \hideinitializer
*/
#define CH5_PKJE_OFFSETS_DSET_FULL_PATH CH5_PKJE_GROUP_FULL_PATH "/" CH5_PKJE_OFFSETS_DSET_NAME

/**
* \def CH5_PKJE_CHUNK_SIZE
* \brief The chunk size for all datasets used with Purkinjes
* \hideinitializer
*/
#define CH5_PKJE_CHUNK_SIZE 1000
/// @endcond

int ch5m_pkje_set_all(hid_t file_id, int n, ch5m_pkje_cable *cables);

int ch5m_pkje_get_info(hid_t file_id, ch5_dataset *out_info);

int ch5m_pkje_get_all(hid_t file_id, ch5m_pkje_cable *out_cables);

int ch5m_pkje_free_cable_set(ch5m_pkje_cable *cables);


/// @cond INTERNAL

int _ch5m_pkje_compare_cable_info(ch5m_pkje_cable *a, ch5m_pkje_cable *b);

hid_t _ch5m_pkje_create_info_compound_type(int type);

hid_t _ch5m_pjke_info_dataset(hid_t file_id, int size);

hid_t _ch5m_pkje_offsets_dataset(hid_t file_id, int size);

hid_t _ch5m_pkje_nodes_dataset(hid_t file_id, int size);

// @endcond

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

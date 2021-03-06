/**
* \file regions.h
* \brief Functions for reading and writing region data
*/
#ifndef _CH5M_REGIONS_H
#define _CH5M_REGIONS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_REGN_GROUP_NAME
* \brief Name for the regions' containing group
* \hideinitializer
*/
#define CH5_REGN_GROUP_NAME "objects"

/**
* \def CH5_REGN_GROUP_FULL_PATH
* \brief Full path to the regions group
* \hideinitializer
*/
#define CH5_REGN_GROUP_FULL_PATH "/" CH5_REGN_GROUP_NAME

/**
* \def CH5_REGN_DSET_NAME
* \brief Name for the regions dataset
* \hideinitializer
*/
#define CH5_REGN_DSET_NAME "regions"

/**
* \def CH5_REGN_DSET_FULL_PATH
* \brief Full path to the info dataset from the root
* \hideinitializer
*/
#define CH5_REGN_DSET_FULL_PATH CH5_REGN_GROUP_FULL_PATH "/" CH5_REGN_DSET_NAME

/**
* \def CH5_REGN_CHUNK_SIZE
* \brief The chunk sized used in the regions HDF dataset
* \hideinitializer
*/
#define CH5_REGN_CHUNK_SIZE 1000
/// @endcond

int ch5m_regn_set_all(hid_t file_id, int n, int *regions);

int ch5m_regn_get_info(hid_t file_id, ch5_dataset *out_info);

int ch5m_regn_get_all(hid_t file_id, int *out_regions);


#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

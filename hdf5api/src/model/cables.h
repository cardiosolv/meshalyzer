/**
* \file cables.h
* \brief Functions for reading and writing cable data
*/
#ifndef _CH5M_CABLES_H
#define _CH5M_CABLES_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_CABL_GROUP_NAME
* \brief Name for the cables' containing group
* \hideinitializer
*/
#define CH5_CABL_GROUP_NAME "objects"

/**
* \def CH5_CABL_GROUP_FULL_PATH
* \brief Full path to the cables group
* \hideinitializer
*/
#define CH5_CABL_GROUP_FULL_PATH "/" CH5_CABL_GROUP_NAME

/**
* \def CH5_CABL_DSET_NAME
* \brief Name for the cables dataset
* \hideinitializer
*/
#define CH5_CABL_DSET_NAME "cables"

/**
* \def CH5_CABL_DSET_FULL_PATH
* \brief Full path to the info dataset from the root
* \hideinitializer
*/
#define CH5_CABL_DSET_FULL_PATH CH5_CABL_GROUP_FULL_PATH "/" CH5_CABL_DSET_NAME

/**
* \def CH5_CABL_CHUNK_SIZE
* \brief The chunk sized used in the cables HDF dataset
* \hideinitializer
*/
#define CH5_CABL_CHUNK_SIZE 1000
/// @endcond

int ch5m_cabl_set_all(hid_t file_id, int n, int *cables);

int ch5m_cabl_get_info(hid_t file_id, ch5_dataset *out_info);

int ch5m_cabl_get_all(hid_t file_id, int *out_cables);


#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

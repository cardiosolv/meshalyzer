/**
* \file lons.h
* \brief Functions and variables related to lons.
*/

#ifndef _CH5M_LONS_H
#define _CH5M_LONS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_LONS_GROUP_NAME
* \brief Group name where the lons dataset lives
* \hideinitializer
*/
#define CH5_LONS_GROUP_NAME "objects"

/**
* \def CH5_LONS_DSET_NAME
* \brief Name of the lons dataset
* \hideinitializer
*/
#define CH5_LONS_DSET_NAME  "lons"

/**
* \def CH5_LONS_FULL_PATH
* \brief Full path to the lons dataset from the root
* \hideinitializer
*/
#define CH5_LONS_FULL_PATH  "/" CH5_LONS_GROUP_NAME "/" CH5_LONS_DSET_NAME

/**
* \def CH5_LONS_ORDER_ATTR_NAME
* \brief Name of the attribute housing the order for the lons dataset
* \hideinitializer
*/
#define CH5_LONS_ORDER_ATTR_NAME "order"

/**
* \def CH5_LONS_CHUNK_SIZE
* \brief The HDF chunk size for the lons dataset
* \hideinitializer
*/
#define CH5_LONS_CHUNK_SIZE 20000
/// @endcond

int ch5m_lons_set_all(hid_t file_id, int n, int order, float *in_data);

int ch5m_lons_get_info(hid_t file_id, ch5_dataset *out_info);

int ch5m_lons_get_order(hid_t file_id);

int ch5m_lons_get_all(hid_t file_id, float *out_data);

int ch5m_lons_get_range(hid_t file_id, int start, int end, float *out_data);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

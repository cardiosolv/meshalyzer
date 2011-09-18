/**
* \file points.h
* \brief Functions and variables related to points.
*/

#ifndef _CH5M_POINTS_H
#define _CH5M_POINTS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_POINTS_GROUP_NAME
* \brief Name of the group which the points dataset is contained within
* \hideinitializer
*/
#define CH5_POINTS_GROUP_NAME "objects"

/**
* \def CH5_POINTS_DSET_NAME
* \brief Name of the points dataset
* \hideinitializer
*/
#define CH5_POINTS_DSET_NAME "points"

/**
* \def CH5_POINTS_FULL_PATH
* \brief The full HDF path to the points dataset from the root
* \hideinitializer
*/
#define CH5_POINTS_FULL_PATH "/" CH5_POINTS_GROUP_NAME "/" CH5_POINTS_DSET_NAME

#define CH5_POINTS_CHUNK_SIZE 20000
/// @endcond

int ch5m_pnts_set_all(hid_t file_id, int n, float *points);

int ch5m_pnts_get_info(hid_t file_id, ch5_dataset *out_info);

int ch5m_pnts_get_all(hid_t file_id, float *out_data);

int ch5m_pnts_get_range(hid_t file_id, int start, int end, float *out_data);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

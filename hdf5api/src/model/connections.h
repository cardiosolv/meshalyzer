/**
* \file connections.h
* \brief Functions for reading and writing connections data
*/
#ifndef _CH5_CONNECTIONS_H
#define _CH5_CONNECTIONS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_CONNECTIONS_GROUP_NAME
* \brief Name of the group which the connections dataset is contained within
* \hideinitializer
*/
#define CH5_CONNECTIONS_GROUP_NAME "objects"

/**
* \def CH5_CONNECTIONS_DSET_NAME
* \brief Name of the connections dataset
* \hideinitializer
*/
#define CH5_CONNECTIONS_DSET_NAME "connections"

/**
* \def CH5_CONNECTIONS_FULL_PATH
* \brief The full HDF path to the connections dataset from the root
* \hideinitializer
*/
#define CH5_CONNECTIONS_FULL_PATH "/" CH5_CONNECTIONS_GROUP_NAME "/" CH5_CONNECTIONS_DSET_NAME

/**
* \def CH5_CONNECTIONS_CHUNK_SIZE
* \brief The chunking size for the connections dataset
* \hideinitializer
*/
#define CH5_CONNECTIONS_CHUNK_SIZE 20000
/// @endcond

int ch5m_conn_get_info(hid_t file_id, ch5_dataset *out_info);

int ch5m_conn_get_all(hid_t file_id, int *out_data);

int ch5m_conn_set_all(hid_t file_id, int n, int *connections);

int ch5m_conn_get_range(hid_t file_id, int start, int end, int *out_data);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

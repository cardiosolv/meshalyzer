/**
* \file elements.h
* \brief Element-related functions, paths and data.
*/

#ifndef _CH5M_ELEMENTS_H
#define _CH5M_ELEMENTS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_ELEM_DSET_NAME
* \brief Global elements dataset name
* \hideinitializer
*/
#define CH5_ELEM_DSET_NAME "elements"

/**
* \def CH5_ELEM_DSET_FULL_PATH
* \brief Global elements dataset name
* \hideinitializer
*/
#define CH5_ELEM_DSET_FULL_PATH "/" CH5_ELEM_DSET_NAME

/**
* \def CH5_ELEM_PRIMITIVE_COUNT
* \brief Total number of primitive types
* \hideinitializer
*/
#define CH5_ELEM_PRIMITIVE_COUNT 7

/**
* \def CH5_ELEM_MAX_VERTEX_COUNT
* \brief Maximum number of vertices in the most complex primitive
* \hideinitializer
*/
#define CH5_ELEM_MAX_VERTEX_COUNT 8

/**
* \def CH5_ELEM_CHUNK_SIZE
* \brief The HDF chunk size for element datasets
* \hideinitializer
*/
#define CH5_ELEM_CHUNK_SIZE 500000

#define CH5_ELEM_TYPE_OFFSET   0
#define CH5_ELEM_REGION_OFFSET 1
#define CH5_ELEM_DATA_OFFSET   2
/// @endcond

/**
* \def CH5_ELEM_WIDTH_ADD
* \brief Number of integers to add to each element to store the type and
*        region
* \hideinitializer
*/
#define CH5_ELEM_WIDTH_ADD     CH5_ELEM_DATA_OFFSET

int ch5m_elem_set_all(hid_t file_id, int n, int max_width, int *in_elem);

int ch5m_elem_set_all_on_dset(hid_t dset_id, int n, int max_width, int *in_elem);

int ch5m_elem_get_info(hid_t file_id, ch5_dataset *out_info);

int ch5m_elem_get_all(hid_t file_id, int *out_elem);

int ch5m_elem_get_all_by_dset(hid_t dset_id, int *out_elem);

ch5m_element_type ch5m_elem_get_type_by_prefix(const char *prefix);

int ch5m_elem_get_width_for_type(ch5m_element_type type);

hid_t ch5m_elem_create_or_open_dset(hid_t parent_id, const char *name, int n, int max_width);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

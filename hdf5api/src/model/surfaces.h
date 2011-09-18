/**
* \file surfaces.h
* \brief Functions and variables related to surfaces
*/

#ifndef _CH5M_SURFACES_H
#define _CH5M_SURFACES_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "../types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_SURF_GROUP_NAME
* \brief The name of the surfaces super-group
* \hideinitializer
*/
#define CH5_SURF_GROUP_NAME "surfaces"

/**
* \def CH5_SURF_GROUP_PATH
* \brief The full path to the surfaces super-group from the root
* \hideinitializer
*/
#define CH5_SURF_GROUP_PATH "/" CH5_SURF_GROUP_NAME

/**
* \def CH5_SURF_CHILD_PREFIX
* \brief The prefixed used for creating child curfaces whithin the main group
* \hideinitializer
*/
#define CH5_SURF_CHILD_PREFIX "Surface"

/**
* \def CH5_SURF_NAME_ATTR
* \brief Attribute name for a surface's name
* \hideinitializer
*/
#define CH5_SURF_NAME_ATTR "name"
/// @endcond

int ch5m_surf_create(hid_t file_id, char *name, int count, int max_width);

int ch5m_surf_get_count(hid_t file_id);

int ch5m_surf_open(hid_t file_id, int n, hid_t *out_id);

int ch5m_surf_close(hid_t surf_id);

int ch5m_surf_get_name(hid_t surf_id, char **name);

int ch5m_surf_get_elem_info(hid_t surf_id, ch5_dataset *out_info);


#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

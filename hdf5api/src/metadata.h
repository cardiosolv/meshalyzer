/**
* \file metadata.h
* \brief Metadata related functions and variables
*/

#ifndef _CH5M_METADATA_H
#define _CH5M_METADATA_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "types.h"

#include <hdf5.h>

/// @cond INTERNAL
/**
* \def CH5_META_VERSION_NAME
* \brief Name for the version attribute
* \hideinitializer
*/
#define CH5_META_VERSION_NAME "version"

/**
* \def CH5_META_NAME_NAME
* \brief Name for the name attribute
* \hideinitializer
*/
#define CH5_META_NAME_NAME "name"
/// @endcond

int ch5_meta_get_version(hid_t file_id, ch5_version *v);

int ch5_meta_set_version(hid_t file_id, ch5_version *v);

int ch5_meta_get_name(hid_t file_id, char **name);

int ch5_meta_set_name(hid_t file_id, const char *name);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

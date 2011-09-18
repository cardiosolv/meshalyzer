/**
* \file file.h
* \brief File management related functions.
*/

#ifndef _CH5M_FILE_H
#define _CH5M_FILE_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include <hdf5.h>

int ch5_open(const char *path, hid_t *file_id);

int ch5_create(const char *path, hid_t *file_id);

int ch5_close(hid_t file_id);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

/**
* \file text.h
* \brief Text file related functions
*/

#ifndef _CH5_TEXT_H
#define _CH5_TEXT_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include "types.h"
#include <hdf5.h>

#define CH5_TEXT_GROUP_NAME       "text_files"
#define CH5_TEXT_FILE_NAME_PREFIX "TextFile"
#define CH5_TEXT_CHUNK_SIZE       100000
#define CH5_TEXT_LABEL_ATTR       "label"

int ch5_text_create(hid_t hdf_file, const char *label);

int ch5_text_info(hid_t hdf_file, unsigned int index, ch5_text_file *info);

int ch5_text_free_info(ch5_text_file *info);

int ch5_text_append(
  hid_t hdf_file,
  unsigned int file_index,
  const char *in
);

int ch5_text_read(
  hid_t hdf_file,
  unsigned int file_index,
  char ***out
);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

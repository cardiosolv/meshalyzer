/**
* \file utils.h
* \brief Various utility functions.
*/

#ifndef _CH5M_UTILS_H
#define _CH5M_UTILS_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

int ch5_util_join_path_strlen(
  int num_parts,
  const char *glue,
  const char *ext, ...
);

void ch5_util_join_path(
  char *out,
  int num_parts,
  const char *glue,
  const char *ext, ...
);

int ch5_int_min(int a, int b);

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

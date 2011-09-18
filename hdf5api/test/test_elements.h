#ifndef TEST_ELEMENTS_H
#define TEST_ELEMENTS_H

#include "types.h"

#include <hdf5.h>

int elem_hashing_and_width();
int elem_writes_all();
int elem_resizes_on_write_all();
int elem_writes_range();
int elem_reads_info();
int elem_bad_info_returns_zero();
int elem_reads_all();
int elem_reads_range();

#endif

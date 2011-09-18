#ifndef PURKINJES_H
#define PURKINJES_H

#include <stdio.h>
#include <hdf5.h>

void pkj_convert_to_hdf5(FILE *pkje_file, hid_t hdf_file_id, char *model_name);

#endif

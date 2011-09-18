#ifndef CABLES_H
#define CABLES_H

#include <stdio.h>
#include <hdf5.h>

void cbl_convert_to_hdf5(FILE *cbl_file, hid_t hdf_file_id, char *model_name);

#endif

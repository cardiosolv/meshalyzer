#ifndef REGIONS_H
#define REGIONS_H

#include <stdio.h>
#include <hdf5.h>

void rgn_convert_to_hdf5(FILE *rgn_file, hid_t hdf_file_id, char *model_name);

#endif

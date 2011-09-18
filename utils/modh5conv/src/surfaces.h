#ifndef SURFACES_H
#define SURFACES_H

#include <stdio.h>
#include <hdf5.h>

/**
 * Conversion entry point for surface files.  Will continue parsing groups of
 * surfaces until hitting the end of file.
 */
void srf_convert_to_hdf5(FILE *srf_file, hid_t hdf_file_id, char *model_name);

int _srf_has_region_data(FILE *srf_file);

#endif

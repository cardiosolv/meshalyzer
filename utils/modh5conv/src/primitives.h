#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <stdio.h>
#include <hdf5.h>


/**
 * TODO
 */
void tet_convert_to_hdf5(FILE* tet_file, hid_t hdf_file_id, char *model_name);

/**
 * TODO
 */
void tri_convert_to_hdf5(FILE* tris_file, hid_t hdf_file_id, char *model_name);

#endif

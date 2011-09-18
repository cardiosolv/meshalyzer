#ifndef LON_H
#define LON_H

#include <stdio.h>
#include <hdf5.h>

/**
 * Given a reference to an ascii lons file with the following format:
 *     [integer:order of vectors in file, N]
 *     [float:vec1_x] [float:vec1_y] [float:vec1_z] ...
 *     [float:vec2_x] [float:vec2_y] [float:vec2_z] ... 
 *           .
 *           .
 *           .
 * This function converts each set of vectors and stores the results in
 * the given HDF5 file referenced by parent_id under the dataset "lons".
 * When the order is 1, only one vector is read (3 floats) and when the order
 * is 2 then two vectors are read (6 floats).
 */
void lon_convert_to_hdf5(FILE *lon_file, hid_t file_id, char *model_name);

#endif

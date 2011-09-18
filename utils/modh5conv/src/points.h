#ifndef POINTS_H
#define POINTS_H

#include <stdio.h>
#include <hdf5.h>

/**
 * Given a reference to an ascii points file with the following format:
 *     [integer:number of points in file, N]
 *     [float:point1_x] [float:point1_y] [float:point1_z]
 *     [float:point2_x] [float:point2_y] [float:point2_z]
 *           .
 *           .
 *           .
 *     [float:pointN_x] [float:pointN_y] [float:pointN_z]
 * This function converts each point to actual floats and stores the result in
 * the given HDF5 file referenced by file_id.
 */
void pts_convert_to_hdf5(FILE *pts_file, hid_t file_id, char *model_name);

#endif

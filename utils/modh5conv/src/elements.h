#ifndef ELEMENTS_H
#define ELEMENTS_H

#include <stdio.h>
#include <hdf5.h>
#include <ch5/ch5.h>

/**
 * Conversion entry point for element files.
 */
void ele_convert_to_hdf5(FILE *ele_file, hid_t hdf_file_id, char *model_name);

/* if force_type is -1 do not force, otherwise use the value */
int* ele_parse_to_array(FILE *ele_file, int *total, char *label, int *max_width,
  int set_default_region, int force_type);

/**
 * Concerns itself with parsing `total` number of rows of data from `ele_file`
 * into sets.  See ele_create_and_write_data for parameter use of similar
 * named parameters.  The final parameter `consume_prefix` should be 1 when
 * the file being parsed contains the two-character primitive type prefix on
 * each line (eg. .elem files) and 0 when this type prefix is not used (eg.
 * .tris files). When set_default_region is any value other than 0, the data
 * parsed will only look for as many nodes as are defined for the particular
 * type (eg. tetrahedrons will parse 4 nodes) and the element will be assigned
 * a default region of 0.  Otherwise, the data parsed will include the region
 * thus it will parse the number of nodes as are defined for the particular
 * element plus one.
 */
int ele_parse_data(FILE *ele_file, int total, int max_width, int *elements,
  int set_default_region, int force_type);

#endif

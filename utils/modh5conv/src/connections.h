#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <stdio.h>
#include <hdf5.h>

void cnx_convert_to_hdf5(FILE *cnx_file, hid_t file_id, char *model_name);

#endif

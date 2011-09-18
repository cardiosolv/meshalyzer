#include "lon.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <ch5/ch5.h>

void lon_convert_to_hdf5(FILE *lon_file, hid_t file_id, char *model_name) {
  // Determine order of lons
  int order, nscan = fscanf(lon_file, "%d", &order);
  if (nscan != 1) {
    fprintf(stderr, "lon_convert_to_hdf5 - incompatible file\n");
    exit(1);
  }
  if (order < 1 || order > 2) {
    fprintf(stderr, "lon_convert_to_hdf5 - unexpected order %d\n", order);
    exit(1);
  }
  int width = 3 * order;
  
  // Store position, count lines in the file and restore file position
  int total = 0;
  char count_buffer[1024];
  fpos_t data_position;
  util_consume_witespace(lon_file);
  
  fgetpos(lon_file, &data_position);
  while (fgets(count_buffer, 1024, lon_file) != NULL) total++;
  fsetpos(lon_file, &data_position);
  
  // Alocate lons and parse
  float *lons = malloc(sizeof(float) * total * width);
  for (int i = 0; i < total; i++) {
    for (int pcount = 0; pcount < order; pcount++) {
      nscan = fscanf(lon_file, "%f %f %f",
        &lons[ARR_OFFSET(i, pcount*3,     width)],
        &lons[ARR_OFFSET(i, pcount*3 + 1, width)],
        &lons[ARR_OFFSET(i, pcount*3 + 2, width)]);
      
      if (nscan != 3) {
        fprintf(stderr, "lon_convert_to_hdf5 - insufficient data\n");
        exit(1);
      }
    }
  }
  
  int result = ch5m_lons_set_all(file_id, total, order, lons);
  if (result) {
    fprintf(stderr, "Could not write lons.\n");
    exit(1);
  }
  
  free(lons);
}

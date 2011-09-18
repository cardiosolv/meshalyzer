#include "regions.h"
#include "utils.h"

#include <stdlib.h>
#include <ch5/ch5.h>

void rgn_convert_to_hdf5(FILE *rgn_file, hid_t hdf_file_id, char *model_name) {
  int total, nscan = fscanf(rgn_file, "%d", &total);
  if (nscan != 1) {
    fprintf(stderr, "rgn_convert_to_hdf5 - incompatible file (no initial count)\n");
    exit(1);
  }
  
  int *regions = (int*) malloc(sizeof(int) * total * 2);
  for (int i = 0; i < total; i++) {
    nscan = fscanf(rgn_file, "%d %d", &regions[i*2], &regions[i*2 + 1]);
    if (nscan != 2) {
      free(regions);
      fprintf(stderr, "rgn_convert_to_hdf5 - parsing failed\n");
      exit(1);
    }
  }
  
  int result = ch5m_regn_set_all(hdf_file_id, total, regions);
  free(regions);
  
  if (result == 0) {
    fprintf(stderr, "rgn_convert_to_hdf5 - failed writing to HDF file\n");
    exit(1);
  }
}

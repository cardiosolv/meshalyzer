#include "cables.h"
#include "utils.h"

#include <stdlib.h>
#include <ch5/ch5.h>

void cbl_convert_to_hdf5(FILE *cbl_file, hid_t hdf_file_id, char *model_name) {
  int total, nscan = fscanf(cbl_file, "%d", &total);
  if (nscan != 1) {
    fprintf(stderr, "cbl_convert_to_hdf5 - incompatible file (no initial count)\n");
    exit(1);
  }
  total += 1;/* Accomodate "one_past_last_point_in_last_cable" */
  
  int *cables = (int*) malloc(sizeof(int) * total);
  for (int i = 0; i < total; i++) {
    nscan = fscanf(cbl_file, "%d", &cables[i]);
    if (nscan != 1) {
      free(cables);
      fprintf(stderr, "cbl_convert_to_hdf5 - parsing failed\n");
      exit(1);
    }
  }
  
  int result = ch5m_cabl_set_all(hdf_file_id, total, cables);
  free(cables);
  
  if (result == 0) {
    fprintf(stderr, "cbl_convert_to_hdf5 - failed writing to HDF file\n");
    exit(1);
  }
}

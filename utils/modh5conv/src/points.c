#include "points.h"
#include "utils.h"

#include <stdlib.h>
#include <ch5/ch5.h>

void pts_convert_to_hdf5(FILE *pts_file, hid_t file_id, char *model_name) {
  // Determine number of points from first integer
  int total, nscan = fscanf(pts_file, "%d", &total);
  if (nscan != 1) {
    fprintf(stderr, "pts_convert_to_hdf5 - incompatible file\n");
    exit(1);
  }
  
  float *points = (float*) calloc(sizeof(float), total * 3);
  for (int i = 0; i < total; i++) {
    nscan = fscanf(pts_file, "%f%f%f",
      &points[ARR_OFFSET(i,0,3)],
      &points[ARR_OFFSET(i,1,3)],
      &points[ARR_OFFSET(i,2,3)]);
    
    if (nscan != 3) {
      fprintf(stderr, "pts_convert_to_hdf5 - insufficient data\n");
      exit(1);
    }
  }
  
  int result = ch5m_pnts_set_all(file_id, total, points);
  if (result) {
    fprintf(stderr, "Could not write points.\n");
    exit(1);
  }
  
  free(points);
}

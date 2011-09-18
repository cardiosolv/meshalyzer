#include "connections.h"
#include "utils.h"

#include <stdlib.h>
#include <ch5/ch5.h>

void cnx_convert_to_hdf5(FILE *cnx_file, hid_t file_id, char *model_name) {
  // Determine number of points from first integer
  int total, nscan = fscanf(cnx_file, "%d", &total);
  if (nscan != 1) {
    fprintf(stderr, "cnx_convert_to_hdf5 - incompatible file\n");
    exit(1);
  }
  
  int *connections = (int*) calloc(sizeof(int), total * 2);
  for (int i = 0; i < total; i++) {
    nscan = fscanf(cnx_file, "%d%d",
      &connections[ARR_OFFSET(i,0,2)],
      &connections[ARR_OFFSET(i,1,2)]);
    
    if (nscan != 2) {
      fprintf(stderr, "cnx_convert_to_hdf5 - insufficient data\n");
      exit(1);
    }
  }
  
  int result = ch5m_conn_set_all(file_id, total, connections);
  if (result == 0) {
    fprintf(stderr, "Could not write connections.\n");
    exit(1);
  }
  
  free(connections);
}

#include "purkinjes.h"
#include "utils.h"

#include <stdlib.h>
#include <ch5/ch5.h>

/*
* NOTE: the cables from this method are not 100% usable, they are constructed
*       solely to provide the minimum amount of data to write to the file.
*       What is missing is the correct pointers on each cable to the proper
*       offset in the nodes array.  Only the first cable has the correct
*       pointer.
*/
void pkj_convert_to_hdf5(FILE *pkje_file, hid_t hdf_file_id, char *model_name) {
  int total, nscan = fscanf(pkje_file, "%d", &total);
  if (nscan != 1) {
    fprintf(stderr, "pkj_convert_to_hdf5 - incompatible file\n");
    exit(1);
  }
  util_consume_witespace(pkje_file);
  
  int node_count = 0;
  double *nodes = NULL;/* array containing all 3-tuple float nodes for all cables */
  ch5m_pkje_cable *cables = (ch5m_pkje_cable*) calloc(sizeof(ch5m_pkje_cable), total);
  
  /* Parse each cable and reallocate nodes as needed */
  char buffer[1024];
  for (int i = 0; i < total; i++) {
    /* Discard first "Cable N" line */
    char *result = fgets(buffer, 1024, pkje_file);
    if (result == NULL) {
      fprintf(stderr, "pkj_convert_to_hdf5 - error reading from input file\n");
      exit(1);
    }
    
    /* Parse the cable details */
    cables[i].index = i;
    nscan = fscanf(pkje_file,
      "%d %d %d %d %d %f %f %f",
      &(cables[i].fathers[0]), &(cables[i].fathers[1]),
      &(cables[i].sons[0]),    &(cables[i].sons[1]),
      &(cables[i].node_count),
      &(cables[i].radius),
      &(cables[i].R_gj),
      &(cables[i].sigma_i));
    if (nscan != 8) {
      fprintf(stderr, "pkj_convert_to_hdf5 - error parsing input file\n");
      exit(1);
    }
    
    /* Allocate memory for incoming nodes and parse them */
    int prev_node_count = node_count;
    node_count += cables[i].node_count;
    nodes = (double*) realloc(nodes, sizeof(double) * node_count * 3);
    if (nodes == NULL) {
      fprintf(stderr, "pkj_convert_to_hdf5 - error allocating memory\n");
      exit(1);
    }
    
    for (int n = 0; n < cables[i].node_count; n++) {
      int offset = (prev_node_count + n) * 3;
      nscan = fscanf(pkje_file, "%lf %lf %lf",
        &nodes[offset + 0], &nodes[offset + 1], &nodes[offset + 2]);
      if (nscan != 3) {
        fprintf(stderr, "pkj_convert_to_hdf5 - error parsing input file\n");
        exit(1);
      }
    }
    util_consume_witespace(pkje_file);
  }
  cables[0].nodes = &nodes;/* Make sure that the first cable has the correct pointer to the nodes */
  
  /* Write data */
  int result = ch5m_pkje_set_all(hdf_file_id, total, cables);
  if (result == 0) {
    fprintf(stderr, "pkj_convert_to_hdf5 - error writing with ch5m_pkje_set_all\n");
    exit(1);
  }
  
  /* Clean up */
  free(cables);
  free(nodes);
}

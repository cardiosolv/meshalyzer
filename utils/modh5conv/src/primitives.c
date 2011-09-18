#include "primitives.h"
#include "elements.h"
#include "surfaces.h"
#include "utils.h"

#include <stdlib.h>
#include <H5LTpublic.h>

// tets act as the main elements dataset
void tet_convert_to_hdf5(FILE *tet_file, hid_t hdf_file_id, char *model_name) {
  char label[1024];
  int total = 0, max_width = 0;
  int *elements = ele_parse_to_array(tet_file, &total, label, &max_width, 0,
    CH5_TETRAHEDRON);
  ch5m_elem_set_all(hdf_file_id, total, max_width, elements);
  free(elements);
}

void tri_convert_to_hdf5(FILE *tris_file, hid_t hdf_file_id, char *model_name) {
  while (!feof(tris_file)) {
    char label[1024];
    int total = 0, max_width = 0;
    int *elements = ele_parse_to_array(tris_file, &total, label, &max_width, 0,
      CH5_TRIANGLE);
    
    int surf_index = ch5m_surf_create(hdf_file_id, NULL, total, max_width);
    hid_t surf_id;
    ch5m_surf_open(hdf_file_id, surf_index, &surf_id);
    
    ch5m_elem_set_all_on_dset(surf_id, total, max_width, elements);
    free(elements);
    
    ch5m_surf_close(surf_id);
    util_consume_witespace(tris_file);
  }
}


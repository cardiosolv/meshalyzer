#include "surfaces.h"
#include "elements.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

void srf_convert_to_hdf5(FILE *srf_file, hid_t hdf_file_id, char *model_name) {
  int surf_count = 0;
  while (!feof(srf_file)) {
    int default_region_reqd = (_srf_has_region_data(srf_file)) ? 0 : 1;
    char label_buf[1024] = { '\0' };
    int total = 0, max_width = 0;
    int *elements = ele_parse_to_array(srf_file, &total, label_buf, &max_width,
      default_region_reqd, -1);
    if (strlen(label_buf) == 0) sprintf(label_buf, "%s.s%d", model_name, surf_count);
    int surf_index = ch5m_surf_create(hdf_file_id, label_buf, total, max_width);
    hid_t surf_id;
    ch5m_surf_open(hdf_file_id, surf_index, &surf_id);
    ch5m_elem_set_all_on_dset(surf_id, total, max_width, elements);
    
    free(elements);
    
    ch5m_surf_close(surf_id);
    util_consume_witespace(srf_file);
    surf_count++;
  }
}

int _srf_has_region_data(FILE *srf_file) {
  fpos_t saved_pos;
  fgetpos(srf_file, &saved_pos);
  
  // Consume the total and the optional label
  char line_buf[255];
  char label_buf[255];
  int total = 0, nscan;
  fgets(line_buf, 255, srf_file);
  nscan = sscanf(line_buf, "%d %s", &total, label_buf);
  fscanf(srf_file, "%d", &total);
  if (nscan != 1 && nscan != 2) {
    fprintf(stderr, "_srf_has_region_data - incompatible file (no initial total or optional label)\n");
    exit(1);
  }
  util_consume_witespace(srf_file);
  
  // Get type
  char type[3] = { [2]='\0' };
  nscan = fscanf(srf_file, "%c%c", &type[0], &type[1]);
  if (nscan != 2) {
    fprintf(stderr, "_srf_has_region_data - incompatible file (no primitive prefix label)\n");
    exit(1);
  }
  int primitive_idx = ch5m_elem_get_type_by_prefix(type);
  if (primitive_idx == -1) {
    fprintf(stderr, "_srf_has_region_data - incompatible file (unknown primitive prefix label)\n");
    exit(1);
  }
  
  // count integers until we hit a \n
  util_consume_witespace(srf_file);
  int i = 0, count = 0, prev_was_nonwhite = 0, curr_is_nonwhite = 0;// count++ when curr_is_nonwhite:false and prev_was_nonwhite:true
  char buffer[256];
  fgets(buffer, 256, srf_file);
  do {
    curr_is_nonwhite = (buffer[i++] != ' ') ? 1 : 0;
    if ((curr_is_nonwhite == 0) && (prev_was_nonwhite == 1)) count++;
    prev_was_nonwhite = curr_is_nonwhite;
  } while (buffer[i] != '\n');
  if (prev_was_nonwhite == 1) count++;
  
  int type_width = ch5m_elem_get_width_for_type(primitive_idx);
  if ((count < type_width) || (count > (type_width + 1))) {
    fprintf(stderr, "_srf_has_region_data - wrong number of nodes for type\n");
    exit(1);
  }
  
  fsetpos(srf_file, &saved_pos);
  
  return (count == type_width) ? 0 : 1;
}

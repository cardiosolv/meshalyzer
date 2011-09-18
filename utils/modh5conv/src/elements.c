#include "elements.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

void ele_convert_to_hdf5(FILE *ele_file, hid_t hdf_file_id, char *model_name) {
  char labelbuff[1024];
  int total = 0, max_width = 0;
  int *elements = ele_parse_to_array(ele_file, &total, labelbuff, &max_width, 0, -1);
  int result = ch5m_elem_set_all(hdf_file_id, total, max_width, elements);
  free(elements);
  if (result) {
    fprintf(stderr, "ele_convert_to_hdf5 - write failed\n");
    exit(1);
  }
}

int* ele_parse_to_array(FILE *ele_file, int *total, char *label, int *max_width,
  int set_default_region, int force_type)
{
  char linebuff[1024];
  fgets(linebuff, 1024, ele_file);
  int nscan = sscanf(linebuff, "%d %[^\n]s", total, label);
  if (nscan != 1 && nscan != 2) {
    fprintf(stderr, "ele_parse_to_array - incompatible file\n");
    exit(1);
  }
  util_consume_witespace(ele_file);
  
  /* Store file position for parsing after finding largest element */
  fpos_t data_position;
  fgetpos(ele_file, &data_position);
  
  char prefixbuff[3] = { [2]=0 };
  int type, size;
  if (force_type != -1) *max_width = ch5m_elem_get_width_for_type(force_type);
  else {
    for (int i = 0; i < (*total); i++) {
      fgets(linebuff, 1024, ele_file);
      sscanf(linebuff, "%c%c", &prefixbuff[0], &prefixbuff[1]);
      type = ch5m_elem_get_type_by_prefix(prefixbuff);
      if (type == -1) {
        fprintf(stderr, "ele_parse_to_array - could not divine primitive type from %s\n", prefixbuff);
        exit(1);
      }
      size = ch5m_elem_get_width_for_type(type);
      if (size > (*max_width)) *max_width = size;
    }
  }
  
  /* Restore position and begin parsing */
  fsetpos(ele_file, &data_position);
  int *elements = (int*) malloc(sizeof(int) * (*total) * ((*max_width) + CH5_ELEM_WIDTH_ADD));
  int parse_result = ele_parse_data(ele_file, *total, (*max_width) + CH5_ELEM_WIDTH_ADD, elements,
    set_default_region, force_type);
  if (parse_result == 0) {
    fprintf(stderr, "ele_parse_to_array - insufficient data\n");
    exit(1);
  }
  
  return elements;
}

/* when force_type is not -1, prefixes are assumed to not exist */
int ele_parse_data(FILE *ele_file, int total, int max_width, int *elements,
  int set_default_region, int force_type)
{
  int nscan;
  char buff[3] = { [2]=0 };
  for (int i = 0; i < total; i++) {
    int type;
    if (force_type == -1) {
      fscanf(ele_file, "%c%c", &buff[0], &buff[1]);
      type = ch5m_elem_get_type_by_prefix(buff);
      if (type == -1) {
        fprintf(stderr, "ele_parse_data - could not divine primitive type from %s\n", buff);
        exit(1);
      }
    }
    else type = force_type;
    
    int read_width  = ch5m_elem_get_width_for_type(type);
    int offset = i * max_width;
    
    elements[offset + CH5_ELEM_TYPE_OFFSET] = type;
    
    for (int d = 0; d < (max_width - CH5_ELEM_WIDTH_ADD); d++) {
      if (d < read_width) {
        nscan = fscanf(ele_file, "%d", &elements[offset + CH5_ELEM_DATA_OFFSET + d]);
        if (nscan != 1) return 0;
      }
      else elements[offset + CH5_ELEM_DATA_OFFSET + d] = 0;
    }
    
    if (set_default_region) elements[offset + CH5_ELEM_REGION_OFFSET] = 0;
    else {
      nscan = fscanf(ele_file, "%d", &elements[offset + CH5_ELEM_REGION_OFFSET]);
      if (nscan != 1) return 0;
    }
    
    util_consume_witespace(ele_file);
  }
  
  return 1;
}

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
  
  char prefixbuff[3] = { [2]=0 };
  if (force_type != -1) 
    *max_width = ch5m_elem_get_width_for_type(force_type);
  else {
    for (int i = 0; i < *total; i++) {
      fgets(linebuff, 1024, ele_file);
      sscanf(linebuff, "%2c", prefixbuff);
      int type = ch5m_elem_get_type_by_prefix(prefixbuff);
      if (type == -1) {
        fprintf(stderr, "ele_parse_to_array - could not divine primitive type from %s\n", prefixbuff);
        exit(1);
      }
      int size = ch5m_elem_get_width_for_type(type);
      if (size > (*max_width)) *max_width = size;
    }
  }

  /* Restore position and begin parsing */
  rewind(ele_file);
  fgets(linebuff, 1024, ele_file);
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
  char linebuff[1024], *lb;
  char buff[3] = { [2]=0 };
  int  consumed;

  for (int i = 0; i < total; i++) {
    int type;
    lb = linebuff;
    fgets(lb, 1024, ele_file);
    if (force_type == -1) {
      sscanf(lb, " %2c%n", buff, &consumed);
      type = ch5m_elem_get_type_by_prefix(buff);
      if (type == -1) {
        fprintf(stderr, "ele_parse_data - could not divine primitive type from %s\n", buff);
        exit(1);
      }
      lb += consumed;
    } else 
      type = force_type;

    int read_width  = ch5m_elem_get_width_for_type(type);
    int offset = i * max_width;

    elements[offset + CH5_ELEM_TYPE_OFFSET] = type;

    for (int d = 0; d < (max_width - CH5_ELEM_WIDTH_ADD); d++) {
      if (d < read_width) {
        if( !sscanf(lb, "%d%n", &elements[offset + CH5_ELEM_DATA_OFFSET + d],
                                                                &consumed ) )
          return 0;
        else
          lb += consumed;
      } else 
        elements[offset + CH5_ELEM_DATA_OFFSET + d] = 0;
    }
    
    elements[offset + CH5_ELEM_REGION_OFFSET] = set_default_region;
    sscanf(lb, "%d", &elements[offset + CH5_ELEM_REGION_OFFSET]);
  }
  
  return 1;
}

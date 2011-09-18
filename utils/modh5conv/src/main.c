/*
 * Parses the following files in this order, if found:
 *   ~/Documents/heart_model/HumanHeart_x2.pts    (required)
 *   ~/Documents/heart_model/HumanHeart_x2.lon    (optional)
 *   ~/Documents/heart_model/HumanHeart_x2.cnnx   (optional)
 *   ~/Documents/heart_model/HumanHeart_x2.elem   (optional)
 *   ~/Documents/heart_model/HumanHeart_x2.tetras (optional, checked if .elem missing)
 *   ~/Documents/heart_model/HumanHeart_x2.surf   (optional)
 *   ~/Documents/heart_model/HumanHeart_x2.tris   (optional, checked if .surf missing)
 *   ~/Documents/heart_model/HumanHeart_x2.cables (optional)
 */

#include "cables.h"
#include "connections.h"
#include "elements.h"
#include "lon.h"
#include "points.h"
#include "primitives.h"
#include "purkinjes.h"
#include "regions.h"
#include "surfaces.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ch5/ch5.h>
#include <hdf5.h>
#include <time.h>

/**
 * Harness for converting specific files.  Accepts a base path `path`, a file
 * name prefix `prefix` and an extension `ext` to build paths for files to
 * parse.  Returns 0 if the file is not required and does not exist, returns a
 * 1 when the file exists and was parsed regardless of whether or not it is
 * required.  Exits when a required file is not found.
 */
int convert_file(char const *prefix, char const *ext, hid_t hdf_file_id,
  char *model_name, void (*conversion_fn)(FILE*, hid_t, char*), int required);

void print_usage() {
  printf("USAGE\n  modh5conv input_model_prefix output_file Model_label\n");
}

int main (int argc, char const *argv[]) {
  if (argc != 4) {
    print_usage();
    return 1;
  }
  
  if (strstr(argv[2], ".modH5") == NULL)
    fprintf(stderr, "WARNING: model files should have the .modH5 suffix\n");
  
  hid_t hdf_file_id = H5Fcreate(argv[2], H5F_ACC_TRUNC, H5P_DEFAULT,
    H5P_DEFAULT);
  if (hdf_file_id < 0) {
    fprintf(stderr, "Could not create HDF5 file.");
    return 1;
  }
  
  /* METADATA */
  /* TODO version from CH5 library */
  ch5_version version = { 0, 0, 1 };
  ch5_meta_set_version(hdf_file_id, &version);
  char *model_name = NULL;
  ch5_meta_set_name(hdf_file_id, (char*)argv[3]);
  model_name = (char*)argv[3];
  
  /* CONVERSIONS */
  convert_file(argv[1], ".pts",  hdf_file_id, model_name, pts_convert_to_hdf5, 1);
  convert_file(argv[1], ".lon",  hdf_file_id, model_name, lon_convert_to_hdf5, 0);
  convert_file(argv[1], ".cnnx", hdf_file_id, model_name, cnx_convert_to_hdf5, 0);
  // .elem supercedes .tetras
  if (!convert_file(argv[1], ".elem", hdf_file_id, model_name, ele_convert_to_hdf5, 0))
    convert_file(argv[1], ".tetras", hdf_file_id, model_name, tet_convert_to_hdf5, 0);
  else {
    char *tets_path = util_join_path(1, "/", ".tetras", argv[1]);
    if (util_file_exists(tets_path))
      printf("WARNING: Both .elem and .tetras files present, skipped .tetras in favour of .elems\n");
    free(tets_path);
  }
  // .surf supercedes .tris
  if (!convert_file(argv[1], ".surf", hdf_file_id, model_name, srf_convert_to_hdf5, 0))
    convert_file(argv[1], ".tris", hdf_file_id, model_name, tri_convert_to_hdf5, 0);
  else {
    char *tris_path = util_join_path(1, "/", ".tris", argv[1]);
    if (util_file_exists(tris_path))
      printf("WARNING: Both .surf and .tris files present, skipped .tris in favour of .surf\n");
    free(tris_path);
  }
  convert_file(argv[1], ".pkje", hdf_file_id, model_name, pkj_convert_to_hdf5, 0);
  convert_file(argv[1], ".region", hdf_file_id, model_name, rgn_convert_to_hdf5, 0);
  convert_file(argv[1], ".cables", hdf_file_id, model_name, cbl_convert_to_hdf5, 0);
  
  H5Fclose(hdf_file_id);

  return 0;
}

int convert_file(char const *prefix, char const *ext, hid_t hdf_file_id,
  char *model_name, void (*conversion_fn)(FILE*, hid_t, char*), int required)
{
  char *full_path = util_join_path(1, "/", ext, prefix);
  int path_length = strlen(full_path);
  FILE *data_file = fopen(full_path, "r");
  if (data_file == NULL) {
    if (required == 0) {
      free(full_path);
      return 0;
    }
    fprintf(stderr, "Required data file \"%s\" not found.\n", full_path);
    exit(1);
  }
  
  int full_path_offset = 0;
  char elipsis[] = "...";
  if (path_length > 50) full_path_offset = path_length - 50;
  else elipsis[0] = '\0';
  
  printf("Parsing: ...%-50s", &full_path[full_path_offset]);
  fflush(stdout);
  
  long start = (long)clock();
  (*conversion_fn)(data_file, hdf_file_id, model_name);
  printf(" : %ldms\n", (((long)clock() - start)*1000)/CLOCKS_PER_SEC);
  fflush(stdout);
  
  fclose(data_file);
  free(full_path);
  
  return 1;
}

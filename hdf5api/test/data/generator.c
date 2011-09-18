#include <stdio.h>
#include <hdf5.h>

int main (int argc, char const *argv[]) {
  if (argc != 2) {
    printf("ERROR: must supply one argument that is the path for the output");
    return 1;
  }
  printf("Generating empty h5 file for invalid data tests.\n");
  hid_t file = H5Fcreate(argv[1], H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  hid_t group = H5Gcreate(file, "bunk", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Gclose(group);
  H5Fclose(file);
  
  return 0;
}
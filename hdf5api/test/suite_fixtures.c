/**
* \file suite_fixtures.c
* \brief Functions that generates fixture data for tests.
*/
#include "suite_fixtures.h"
#include "suite_utils.h"
#include "ch5.h"

#include <stdlib.h>
#include <string.h>

/* GENERICS */
hid_t fixture_create_generics_hdf_file() {
  hid_t file = H5Fcreate(GNRC_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  hid_t group_id = ch5_gnrc_open_or_create_group(file, "Group");
  
  float f_value = fixture_generics_float();
  ch5_gnrc_set_attr(group_id, H5T_IEEE_F32LE, "float", &f_value);
  
  int i_value = fixture_generics_int();
  ch5_gnrc_set_attr(group_id, H5T_STD_U32LE, "int", &i_value);
  
  char *s_value = fixture_generics_str();
  ch5_gnrc_set_attr(group_id, H5T_C_S1, "char", s_value);
  free(s_value);
  
  H5Gclose(group_id);
  
  return file;
}

void fixture_close_generics_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(GNRC_HDF_PATH);
}

float fixture_generics_float() { return 0.125; };
int fixture_generics_int() { return 1337; }
char* fixture_generics_str() {
  char *out = (char*) malloc(sizeof(char) * 13);
  strcpy(out, "Company Flow");
  return out;
}


/* METADATA */
hid_t fixture_create_metadata_hdf_file() {
  hid_t file = H5Fcreate(META_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  ch5_version *version = fixture_metadata_version();
  ch5_meta_set_version(file, version);
  char *name = fixture_metadata_name();
  ch5_meta_set_name(file, name);
  free(version);
  free(name);
  return file;
}

void fixture_close_metadata_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(META_HDF_PATH);
}

ch5_version* fixture_metadata_version() {
  ch5_version *v = (ch5_version*) malloc(sizeof(ch5_version));
  v->major = 1;
  v->minor = 2;
  v->patch = 3;
  return v;
}

char* fixture_metadata_name() {
  char name[] = "Cthulhu";
  char *out = (char*) malloc(sizeof(char) * (strlen(name) + 1));
  strcpy(out, name);
  return out;
}



/* POINTS */
hid_t fixture_create_points_hdf_file() {
  hid_t file = H5Fcreate(PNTS_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT,
    H5P_DEFAULT);
  float **points = fixture_points(10);
  ch5m_pnts_set_all(file, 10, points[0]);
  fixture_free_points(points);
  return file;
}

void fixture_close_points_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(PNTS_HDF_PATH);
}

float** fixture_points(int n) {
  float **points = alloc_2d_float(3, n);
  for (int i = 0; i < n; i++)
    for (int p = 0; p < 3; p++) points[i][p] = (float)(i + 1);
  return points;
}

void fixture_free_points(float **points) {
  free_2d_float(points);
}



/* CONNECTIONS */
hid_t fixture_create_connections_hdf_file() {
  hid_t file = H5Fcreate(CONN_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT,
    H5P_DEFAULT);
  int **cnnx = fixture_connections(10);
  ch5m_conn_set_all(file, 10, cnnx[0]);
  fixture_free_connections(cnnx);
  return file;
}

void fixture_close_connections_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(CONN_HDF_PATH);
}

int** fixture_connections(int n) {
  int **connections = alloc_2d_int(2, n);
  for (int i = 0; i < n; i++)
    for (int j = 0; j < 2; j++) connections[i][j] = i * 2 + j;
  return connections;
}

void fixture_free_connections(int **connections) {
  free_2d_int(connections);
}



/* REGIONS */
hid_t fixture_create_regions_hdf_file() {
  hid_t file = H5Fcreate(REGN_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT,
    H5P_DEFAULT);
  int **regions = fixture_regions(10);
  ch5m_regn_set_all(file, 10, regions[0]);
  fixture_free_regions(regions);
  return file;
}

void fixture_close_regions_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(REGN_HDF_PATH);
}

int** fixture_regions(int n) {
  int **regions = alloc_2d_int(2, n);
  for (int i = 0; i < n; i++)
    for (int j = 0; j < 2; j++) regions[i][j] = i * 10 + j * 10 - j;
  return regions;
}

void fixture_free_regions(int **regions) {
  free_2d_int(regions);
}



/* CABLES */
hid_t fixture_create_cables_hdf_file() {
  hid_t file = H5Fcreate(CABL_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT,
    H5P_DEFAULT);
  int *cables = fixture_cables(10);
  ch5m_cabl_set_all(file, 10, cables);
  fixture_free_cables(cables);
  return file;
}

void fixture_close_cables_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(CABL_HDF_PATH);
}

int* fixture_cables(int n) {
  int *cables = (int*) malloc(sizeof(int) * n);
  for (int i = 0; i < n; i++) cables[i] = i * 10;
  return cables;
}

void fixture_free_cables(int *cables) {
  free(cables);
}



/* LONS */
hid_t fixture_create_lons_hdf_file(int order) {
  if (!(order == 1 || order == 2)) {
    fprintf(stderr, "Order must be either 1 or 2.\n");
    exit(1);
  }
  hid_t file = H5Fcreate(LONS_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT,
    H5P_DEFAULT);
  float **lons = fixture_lons(5, order);
  ch5m_lons_set_all(file, 5, order, lons[0]);
  fixture_free_lons(lons);
  return file;
}

void fixture_close_lons_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(LONS_HDF_PATH);
}

float** fixture_lons(int n, int order) {
  if (!(order == 1 || order == 2)) {
    fprintf(stderr, "Order must be either 1 or 2.\n");
    exit(1);
  }
  int width = order * 3;
  float **lons = alloc_2d_float(width, n);
  for (int i = 0; i < n; i++)
    for (int p = 0; p < width; p++) lons[i][p] = (((float)i + 1) + ((float)p / 10));
  return lons;
}

void fixture_free_lons(float **lons) {
  free_2d_float(lons);
}



/* ELEMENTS */
hid_t fixture_create_elements_hdf_file() {
  hid_t file = H5Fcreate(ELEM_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT,
    H5P_DEFAULT);
  int **elements = fixture_elements(10, 8);
  ch5m_elem_set_all(file, 10, 8, elements[0]);
  fixture_free_elements(elements);
  return file;
}

void fixture_close_elements_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(ELEM_HDF_PATH);
}

int** fixture_elements(int n, int max_width) {
  if (n < 1) {
    fprintf(stderr, "You have to specify at least one element.\n");
    exit(1);
  }
  int **elements = alloc_2d_int(max_width + CH5_ELEM_WIDTH_ADD, n);
  
  for (int i = 0, t = 0; i < n; i++, t++) {
    int type  = t % (CH5_HEXAHEDRON + 1);
    int width = ch5m_elem_get_width_for_type(type);
    if (width > max_width) {
      i--;
      continue;
    }
    elements[i][0] = type;
    elements[i][1] = i; /* region */
    for (int d = 0; d < max_width; d++)
      elements[i][CH5_ELEM_WIDTH_ADD+d] = (d < width) ? d+1 : 0;// fill unused values with 0
  }
  return elements;
}

void fixture_free_elements(int **elements) {
  free_2d_int(elements);
}



/* SURFACES */
hid_t fixture_create_surfaces_hdf_file() {
  hid_t file = H5Fcreate(SURF_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  int **elements = fixture_elements(5, 8);
  
  for (int i = 0; i < 2; i++) {
    char *name = fixture_surface_name(i);
    int surf_index = ch5m_surf_create(file, name, 5, 8);
    hid_t surf_id;
    ch5m_surf_open(file, surf_index, &surf_id);
    ch5m_elem_set_all_on_dset(surf_id, 5, 8, elements[0]);
    ch5m_surf_close(surf_id);
    free(name);
  }
  
  fixture_free_elements(elements);
  
  return file;
}

void fixture_close_surfaces_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(SURF_HDF_PATH);
}

char* fixture_surface_name(int i) {
  char *name = (char*) malloc(sizeof(char) * 9);
  switch (i) {
    case 0:
      strcpy(name, "DICKDALE");
      break;
    
    case 1:
      strcpy(name, "BIGWAVES");
      break;
  }
  
  return name;
}



/* PURKINJES */
hid_t fixture_create_purkinje_hdf_file() {
  hid_t file = H5Fcreate(PKJE_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  ch5m_pkje_cable *cables = fixture_purkinje_cables();
  ch5m_pkje_set_all(file, 2, cables);
  fixture_free_purkinje_cables(cables);
  return file;
}

void fixture_close_purkinje_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(PKJE_HDF_PATH);
}

/**
* \brief Generates a fixture of 2 Purkinje cables with nodes
* \note The returned array should be freed with #fixture_free_purkinje_cables.
* \sa fixture_free_purkinje_cables
*/
ch5m_pkje_cable* fixture_purkinje_cables() {
  ch5m_pkje_cable *cables = (ch5m_pkje_cable*) malloc(sizeof(ch5m_pkje_cable) * 2);
  
  /* Cable 0 Info */
  cables[0].index      = 0;
  cables[0].fathers[0] = -1;
  cables[0].fathers[1] = -1;
  cables[0].sons[0]    = 1;
  cables[0].sons[1]    = -1;
  cables[0].node_count = 2;
  cables[0].radius     = 1.23;
  cables[0].R_gj       = 4.56;
  cables[0].sigma_i    = 7.89;
  
  /* Cable 1 Info */
  cables[1].index      = 1;
  cables[1].fathers[0] = 0;
  cables[1].fathers[1] = -1;
  cables[1].sons[0]    = -1;
  cables[1].sons[1]    = -1;
  cables[1].node_count = 3;
  cables[1].radius     = 3.21;
  cables[1].R_gj       = 6.54;
  cables[1].sigma_i    = 9.87;
  
  /* Cable Nodes */
  double *nodes = (double*) malloc(sizeof(double) * 15);
  /* Cable 0 Nodes */
  nodes[0] = 0.1;
  nodes[1] = 0.2;
  nodes[2] = 0.3;
  nodes[3] = 0.4;
  nodes[4] = 0.5;
  nodes[5] = 0.6;
  /* Cable 1 Nodes */
  nodes[6] = 1.1;
  nodes[7] = 2.2;
  nodes[8] = 3.3;
  nodes[9] = 4.4;
  nodes[10] = 5.5;
  nodes[11] = 6.6;
  nodes[12] = 7.7;
  nodes[13] = 8.8;
  nodes[14] = 9.9;
  /* Pseudo 2D Array */
  double **cnodes = (double**) malloc(sizeof(double*) * 5);
  cnodes[0] = &nodes[0];
  cnodes[1] = &nodes[3];
  cnodes[2] = &nodes[6];
  cnodes[3] = &nodes[9];
  cnodes[4] = &nodes[12];
  
  /* Assign Nodes to Cables */
  cables[0].nodes = &cnodes[0];
  cables[1].nodes = &cnodes[2];
  
  return cables;
}

/**
* \brief Frees the memory allocated in #fixture_purkinje_cables
* \param[in] cables Array of #ch5m_pkje_cable structs to be freed
* \sa fixture_purkinje_cables
*/
void fixture_free_purkinje_cables(ch5m_pkje_cable *cables) {
  if (cables != NULL) {
    if (cables[0].nodes != NULL) {
      if (cables[0].nodes[0] != NULL) free(cables[0].nodes[0]);
      free(cables[0].nodes);
    }
    free(cables);
  }
}



/* VECTORS */
hid_t fixture_create_vectors_hdf_file() {
  hid_t file = H5Fcreate(S_VECS_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  
  fixture_vectors_create_and_pop_grid(file, 4, 6, 1.25, "Moss", "Roy", "seconds", "cm^2", "DERP");
  fixture_vectors_create_and_pop_grid(file, 3, 2, 0.01, "Lister", NULL, NULL, NULL, "HAI");
  fixture_vectors_create_and_pop_grid(file, 2, 3, 2.00, NULL, "Rimmer", "XYZ", "hours", NULL);
  
  return file;
}

void fixture_close_vectors_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(S_VECS_HDF_PATH);
}

float* fixture_vectors(int n, int t, int c) {
  return fixture_generic_3d_array(t, n, c);
}

void fixture_free_vectors(float *vectors) {
  free(vectors);
}

float* fixture_vectors_data(int n, int t) {
  float *out = (float*) malloc(sizeof(float) * n * t);
  for (int x = 0; x < t; x++) {/* time steps */
    for (int y = 0; y < n; y++) {/* vectors */
      int index = x*n + y;
      out[index] = (float) index;
    }
  }
  return out;
}

void fixture_vectors_create_and_pop_grid(hid_t file, int n, int t, float time_delta,
  char *grid_label, char *scalar_label, char *time_units, char *units, char *comments)
{
  float **points = fixture_points(n);
  float *data = fixture_vectors(n, t, (scalar_label == NULL) ? CH5_VECS_BASE_COMPONENTS : CH5_VECS_BASE_COMPONENTS + 1);
  int gi = ch5s_vector_create_grid(file, n, t, 2., time_delta, points[0], grid_label, scalar_label, time_units, units, comments);
  ch5s_vector_write_all(file, gi, data);
  fixture_free_points(points);
  fixture_free_vectors(data);
}


/* NODAL */
hid_t fixture_create_nodal_hdf_file() {
  hid_t file = H5Fcreate(S_NODA_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  
  fixture_nodal_create_and_pop_grid(file, 2, 3, 0.125, CH5_SCALAR, "KGB", "seconds", "siemens", NULL);
  fixture_nodal_create_and_pop_grid(file, 4, 6, 0.5,   CH5_DYN_PTS, NULL, NULL, NULL, "Aerodynamique");
  
  return file;
}

void fixture_close_nodal_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(S_NODA_HDF_PATH);
}

float* fixture_nodal_data(ch5s_nodal_type type, int n, int t) {
  return fixture_generic_3d_array(t, n,
    (type == CH5_SCALAR) ? CH5_NODA_SCALAR_NODE_WIDTH : CH5_NODA_DYNPTS_NODE_WIDTH);
}

void fixture_free_nodal_data(float *data) {
  free(data);
}

void fixture_nodal_create_and_pop_grid(hid_t file, int n, int t, float time_delta,
  ch5s_nodal_type type, char *label, char *time_units, char *units, char *comments)
{
  float *data = fixture_nodal_data(type, n, t);
  int gi = ch5s_nodal_create_grid(file, n, t, 1., time_delta, type, label, time_units, units, comments);
  ch5s_nodal_write(file, gi, 0, t-1, data);
  fixture_free_nodal_data(data);
}



/* TIME SERIES */
hid_t fixture_create_timeseries_hdf_file() {
  hid_t file = H5Fcreate(S_TIME_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  int tsid;
  
  tsid = ch5s_series_create(file, 2, 4, "Moderat", "QUANT", "units", "comments");
  double *data1 = fixture_timeseries(2, 5);
  ch5s_series_write(file, tsid, 0, 1, data1);
  fixture_free_timeseries(data1);
  
  tsid = ch5s_series_create(file, 3, 3, "AphexTwin", NULL, NULL, NULL);
  double *data2 = fixture_timeseries(2, 4);
  ch5s_series_write(file, tsid, 0, 2, data2);
  fixture_free_timeseries(data2);
  
  return file;
}

void fixture_close_timeseries_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(S_TIME_HDF_PATH);
}

double* fixture_timeseries(int t, int n) {
  return fixture_generic_double_2d_array(t, n);
}

void fixture_free_timeseries(double *data) {
  free(data);
}



/* AUX GRIDS */
hid_t fixture_create_auxgrid_hdf_file() {
  hid_t file = H5Fcreate(S_AUX_HDF_PATH, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  int gid;

  gid = ch5s_aux_create_grid(file, 666., 0.5, "Label", "Time Units", "Units", "Comments");
  float      **points1    = fixture_auxgrid_points(3);
  unsigned int elems1[12] = { 0, 1, 2, 1, 1, 2, 3, 2, 1, 2, 3, 4 };
  float       *data1      = fixture_auxgrid_data(3);
  ch5s_aux_write_next(file, gid, 3, points1[0], 3, 4, elems1, data1);
  ch5s_aux_write_next(file, gid, 3, points1[0], 3, 4, elems1, data1);
  fixture_auxgrid_free_points(points1);
  fixture_auxgrid_free_data(data1);
  
  gid = ch5s_aux_create_grid(file, 3., 1.25, "AphexTwin", NULL, "RumbleFish", NULL);
  float          **points2   = fixture_auxgrid_points(2);
  unsigned int     elems2[5] = { 2, 1, 2, 3, 4 };
  ch5s_aux_write_next(file, gid, 2, points2[0], 1, 4, elems2, NULL);
  ch5s_aux_write_next(file, gid, 2, points2[0], 0, 0, NULL, NULL);
  fixture_auxgrid_free_points(points2);
  
  return file;
}

float** fixture_auxgrid_points(int n) {
  return fixture_points(n);
}

void fixture_auxgrid_free_points(float** points) {
  fixture_free_points(points);
}

float* fixture_auxgrid_data(int n) {
  float *out = (float*) malloc(sizeof(float) * n);
  for (int i = 0; i < n; i++) out[i] = i;
  return out;
}

void fixture_auxgrid_free_data(float *data) {
  free(data);
}

void fixture_close_auxgrid_hdf_file(hid_t file) {
  H5Fclose(file);
  remove(S_AUX_HDF_PATH);
}



/* MISC */
float* fixture_generic_3d_array(int a, int b, int c) {
  float *out = (float*) malloc(sizeof(float) * a * b * c);
  for (int x = 0; x < a; x++) {/* time steps */
    for (int y = 0; y < b; y++) {/* vectors */
      for (int z = 0; z < c; z++) {/* data */
        int index = x*b*c + y*c + z;
        out[index] = (float) index;
      }
    }
  }
  return out;
}

double* fixture_generic_double_2d_array(int a, int b) {
  double *out = (double*) malloc(sizeof(double) * a * b);
  for (int x = 0; x < a; x++) {
    for (int y = 0; y < b; y++) {
      int index = x*b + y;
      out[index] = (double) index;
    }
  }
  return out;
}

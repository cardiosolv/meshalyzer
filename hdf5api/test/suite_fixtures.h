#ifndef SUITE_FIXTURES_H
#define SUITE_FIXTURES_H

#include "../src/types.h"

#include <hdf5.h>

#define BASE_HDF_PATH   "test/data"
#define GNRC_HDF_PATH   BASE_HDF_PATH "/test_generics.h5"
#define META_HDF_PATH   BASE_HDF_PATH "/test_metadata.h5"
#define PNTS_HDF_PATH   BASE_HDF_PATH "/test_points.h5"
#define LONS_HDF_PATH   BASE_HDF_PATH "/test_lons.h5"
#define ELEM_HDF_PATH   BASE_HDF_PATH "/test_elements.h5"
#define CONN_HDF_PATH   BASE_HDF_PATH "/test_connections.h5"
#define REGN_HDF_PATH   BASE_HDF_PATH "/test_regions.h5"
#define CABL_HDF_PATH   BASE_HDF_PATH "/test_cables.h5"
#define SURF_HDF_PATH   BASE_HDF_PATH "/test_surfaces.h5"
#define PKJE_HDF_PATH   BASE_HDF_PATH "/test_purkinje.h5"
#define S_VECS_HDF_PATH BASE_HDF_PATH "/test_s_vectors.h5"
#define S_NODA_HDF_PATH BASE_HDF_PATH "/test_s_nodal.h5"
#define S_TIME_HDF_PATH BASE_HDF_PATH "/test_s_timeseries.h5"
#define S_AUX_HDF_PATH  BASE_HDF_PATH "/test_s_auxgrid.h5"

hid_t            fixture_create_generics_hdf_file();
void             fixture_close_generics_hdf_file(hid_t file);
float            fixture_generics_float();
int              fixture_generics_int();
char*            fixture_generics_str();

hid_t            fixture_create_metadata_hdf_file();
void             fixture_close_metadata_hdf_file(hid_t file);
ch5_version*     fixture_metadata_version();
char*            fixture_metadata_name();

hid_t            fixture_create_points_hdf_file();
void             fixture_close_points_hdf_file(hid_t file);
float**          fixture_points(int n);
void             fixture_free_points(float **points);

hid_t            fixture_create_connections_hdf_file();
void             fixture_close_connections_hdf_file(hid_t file);
int**            fixture_connections(int n);
void             fixture_free_connections(int **connections);

hid_t            fixture_create_regions_hdf_file();
void             fixture_close_regions_hdf_file(hid_t file);
int**            fixture_regions(int n);
void             fixture_free_regions(int **regions);

hid_t            fixture_create_cables_hdf_file();
void             fixture_close_cables_hdf_file(hid_t file);
int*             fixture_cables(int n);
void             fixture_free_cables(int *cables);

hid_t            fixture_create_lons_hdf_file(int order);
void             fixture_close_lons_hdf_file(hid_t file);
float**          fixture_lons(int n, int order);
void             fixture_free_lons(float **lons);

hid_t            fixture_create_elements_hdf_file();
void             fixture_close_elements_hdf_file(hid_t file);
int**            fixture_elements(int n, int max_width);
void             fixture_free_elements(int **elements);

hid_t            fixture_create_surfaces_hdf_file();
void             fixture_close_surfaces_hdf_file(hid_t file);
char*            fixture_surface_name(int i);

hid_t            fixture_create_purkinje_hdf_file();
void             fixture_close_purkinje_hdf_file(hid_t file);
ch5m_pkje_cable* fixture_purkinje_cables();
void             fixture_free_purkinje_cables(ch5m_pkje_cable *cables);

hid_t            fixture_create_vectors_hdf_file();
void             fixture_close_vectors_hdf_file(hid_t file);
float*           fixture_vectors(int n, int t, int c);
void             fixture_free_vectors(float *vectors);
void             fixture_vectors_create_and_pop_grid(hid_t file, int n, int t, float time_delta, char *grid_label, char *scalar_label, char *time_units, char *units, char *comments);

hid_t            fixture_create_nodal_hdf_file();
void             fixture_close_nodal_hdf_file(hid_t file);
float*           fixture_nodal_data(ch5s_nodal_type type, int n, int t);
void             fixture_free_nodal_data(float *data);
void             fixture_nodal_create_and_pop_grid(hid_t file, int n, int t, float time_delta, ch5s_nodal_type type, char *label, char *time_units, char *units, char *comments);

hid_t            fixture_create_timeseries_hdf_file();
void             fixture_close_timeseries_hdf_file(hid_t file);
double*          fixture_timeseries(int t, int n);
void             fixture_free_timeseries(double *data);

hid_t            fixture_create_auxgrid_hdf_file();
void             fixture_close_auxgrid_hdf_file(hid_t file);
float**          fixture_auxgrid_points(int n);
void             fixture_auxgrid_free_points(float** points);
float*           fixture_auxgrid_data(int n);
void             fixture_auxgrid_free_data(float* data);

float*           fixture_generic_3d_array(int a, int b, int c);
double*          fixture_generic_double_2d_array(int a, int b);

#endif

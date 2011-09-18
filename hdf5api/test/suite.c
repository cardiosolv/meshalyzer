#include "suite_utils.h"

#include "test_generics.h"
#include "test_text.h"
#include "test_utils.h"

#include "test_metadata.h"
#include "test_points.h"
#include "test_cables.h"
#include "test_connections.h"
#include "test_lons.h"
#include "test_elements.h"
#include "test_purkinjes.h"
#include "test_regions.h"
#include "test_surfaces.h"

#include "test_s_vectors.h"
#include "test_s_nodal.h"
#include "test_s_time_series.h"
#include "test_s_aux_grids.h"

#include <stdio.h>
#include <getopt.h>
#include <hdf5.h>

int main (int argc, char * const *argv) {
  /* Parse args for flags to disable model or simulation related tests. */
  int test_general = 1, test_model = 1, test_simulation = 1;
  struct option long_options[] = {
    {"no-general", no_argument, &test_general,    0},
    {"no-model",   no_argument, &test_model,      0},
    {"no-sim",     no_argument, &test_simulation, 0}
  };
  while (getopt_long(argc, argv, "", long_options, NULL) != -1) { continue; }
  
  /* Turn off the verbose hdf5 error printing for checking on bad data which
     produces a lot of errors that we don't need to see.  It's useful, however,
     to comment this out when determining the source of an error while getting
     tests to pass. */
  H5Eset_auto1(NULL, NULL);
  printf("\n");
  
  if (test_general) {
    print_title("Utilities");
    run_test("Computes joined path length", util_computes_joined_path_length);
    run_test("Joins a path to a static string", util_joins_static_path);
    
    print_title("Attributes");
    run_test("Sets a single-value attribute regardless of type", sets_type_insensitive_single_attribute);
    run_test("Gets a single-value attribute regardless of type", gets_type_insensitive_single_attribute);
    
    print_title("N-Child Management");
    run_test("Generates names based on a prefix and number", nchild_generates_names);
    run_test("Creates or opens an nchild container", nchild_creates_or_opens_container);
    run_test("Counts the number of children", nchild_counts_children);
    
    print_title("Text Data");
    run_test("Creates a new text container", text_creates_new);
    run_test("Retrieves info about the text container", text_gets_info);
    run_test("Appends to container", text_appends);
    run_test("Reads from container", text_reads);
  }
  
  if (test_model) {
    print_title("Metadata");
    run_test("Writes version metadata", meta_writes_version);
    run_test("Writes name metadata", meta_writes_name);
    run_test("Reads version metadata", meta_reads_version);
    run_test("Invalid version metadata returns 0", meta_bad_version_returns_zero);
    run_test("Reads name metadata", meta_reads_name);
    run_test("Invalid name metadata returns zero", meta_bad_name_returns_zero);
  
    print_title("Points");
    run_test("Writes full set of points", pnts_writes_all_data);
    run_test("Resizes dataset and writes points on rewrite", pnts_resizes_on_write_all);
    run_test("Writes a range of points", pnts_writes_range);
    run_test("Reads points info", pnts_reads_info);
    run_test("Invalid info returns 0", pnts_bad_info_returns_zero);
    run_test("Reads entire points data", pnts_reads_all_data);
    run_test("Invalid data returns 0", pnts_bad_data_returns_zero);
    run_test("Reads a range of points data", pnts_reads_range);
  
    print_title("Connections");
    run_test("Writes full set of connections", conn_writes_all);
    run_test("Resizes dataset and writes connections on rewrite", conn_resizes_on_write_all);
    run_test("Writes a range of connections", conn_writes_range);
    run_test("Reads connections dataset info", conn_reads_info);
    run_test("Invalid info returns 0", conn_bad_info_returns_zero);
    run_test("Reads entire connections data", conn_reads_all_data);
    run_test("Invalid data returns 0", conn_bad_data_returns_zero);
    run_test("Reads a range of connections data", conn_reads_range);
  
    print_title("Regions");
    run_test("Writes full set of regions", regn_writes_all);
    run_test("Reads regions dataset info", regn_reads_info);
    run_test("Reads entire regions data", regn_reads_all);
  
    print_title("Cables");
    run_test("Writes full set of cables", cabl_writes_all);
    run_test("Reads cables dataset info", cabl_reads_info);
    run_test("Reads entire cables data", cabl_reads_all);
  
    print_title("Lons");
    run_test("Writes full set of lons (orders 1 and 2)", lons_writes_all);
    run_test("Resizes dataset and writes lons on rewrite", lons_resizes_on_write_all);
    run_test("Writes a range of lons", lons_writes_range);
    run_test("Reads lons info", lons_reads_info);
    run_test("Invalid info returns 0", lons_bad_info_returns_zero);
    run_test("Reads lons order", lons_reads_order);
    run_test("Invalid order returns -1 and order is not modified", lons_bad_order_returns_negone);
    run_test("Reads entire lons data", lons_reads_all_data);
    run_test("Reads a range of data", lons_reads_range);
  
    print_title("Elements");
    run_test("Short-type hashing and width lookups work correctly", elem_hashing_and_width);
    run_test("Writes full set of elements", elem_writes_all);
    run_test("Resizes dataset and writes elements on rewrite", elem_resizes_on_write_all);
    run_test("Writes a range of elements", elem_writes_range);
    run_test("Reads elements info", elem_reads_info);
    run_test("Invalid info returns 0", elem_bad_info_returns_zero);
    run_test("Reads entire elements data", elem_reads_all);
    run_test("Reads range of elements data", elem_reads_range);
  
    print_title("Surfaces");
    /* Write tests are all handled by the element writing methods */
    run_test("Creates a named surface", surf_creates_named_surface);
    run_test("Creates an unnamed surface", surf_creates_unnamed_surface);
    run_test("Returns the number of surfaces", surf_gets_surface_count);
    run_test("Opens a surface by id number", surf_opens_surface_by_index);
    run_test("Reads a surface name", surf_reads_name);
    run_test("Reads elements info for surface", surf_reads_info);
    run_test("Reads entire elements data for surface", surf_reads_all_elements);
    run_test("Reads range of elements data for surface", surf_reads_range_of_elements);
  
    print_title("Purkinje Fibres");
    run_test("Cable info comparisons work as expected", pkje_data_comparisson);
    run_test("Writes set of fibre data", pkje_writes_all);
    run_test("Reads Purkinje fibre info", pkje_reads_info);
    run_test("Read from bad file returns zero", pkje_read_bad_file_returns_zero);
    run_test("Reads a set of fibre data", pkje_reads_all);
  }
  
  if (test_simulation) {
    print_title("Vector Grids");
    run_test("Creates a vector grid with correct properties", s_vect_creates_grid);
    run_test("Retrieves and frees info structs for vector grids", s_vect_gets_info);
    run_test("Counts the number of vector grids", s_vect_counts_grids);
    run_test("Writes vectors to grid in time ranges", s_vect_writes_vectors);
    run_test("Returns errors writing bad time ranges", s_vect_error_writing_bad_time_ranges);
    run_test("Reads entire set of vectors", s_vect_reads_vectors);
    run_test("Reads the entire set of points", s_vect_reads_points);
    
    print_title("Nodal Grids");
    run_test("Creates a nodal grid with the correct properties", s_noda_creates_grid);
    run_test("Retrieves and frees info structs for nodal grids", s_noda_gets_info);
    run_test("Counts the number of nodal grids", s_noda_counts_grids);
    run_test("Writes data to the nodal grid in time ranges", s_noda_writes_data);
    run_test("Returns errors writing bad time ranges", s_noda_error_writing_bad_time_ranges);
    run_test("Reads data by a time range", s_noda_reads_data_by_time_range);
    run_test("Reads data over time by a node index", s_noda_reads_data_by_node_index);
    
    print_title("Time Series");
    run_test("Creates a time series with the right properties", s_time_creates_series);
    run_test("Retrieves and frees info structs for a series", s_time_gets_series_info);
    run_test("Counts the number of time series", s_time_counts_series);
    run_test("Writes data to a time seires in time ranges", s_time_writes_data);
    run_test("Reads data by time range", s_time_reads_data);
    run_test("Reads data by value range", s_time_reads_data_by_value);
    
    print_title("Auxilliary Grids");
    run_test("Creates an auxiliary grid with the right properties", s_aux_creates_grid);
    run_test("Retrieves and frees info for an auxiliary grid", s_aux_gets_grid_info);
    run_test("Counts the number of auxiliary grids", s_aux_counts_grids);
    run_test("Writes all data to the next time index", s_aux_writes_next_time);
    run_test("Retrieves info for a time step in a grid", s_aux_gets_time_info);
    run_test("Reads data at a given time index", s_aux_reads_time);
  }
  
  print_title("TEST SUITE RESULTS");
  print_result_summary();
  
  return 0;
}

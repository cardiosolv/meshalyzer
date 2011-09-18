/* CARP HDF5 Simulation API Draft */
/*
* Notes:
*   - Prefix ch5  (CARP hdf5)
*   - Prefix ch5m ("m" for model)
*   - Prefix ch5s ("s" for simulation)
*   - Functions marked with a + have been implemented
*/


/***

    VECTORS

***/
/*
* Returns number of vector grids stored.
*/
+ int ch5s_vector_grid_count(hid_t hdf_file);

/*
* Populates a ch5s_vector_grid struct with the grid information.  API
* consumer is responsible for freeing this struct with
* #ch5s_vector_free_grid_info
*/
struct ch5s_vector_grid {
  unsigned int time_steps;
  float        time_delta;
  unsigned int num_vectors;
  char*        label;
  char*        scalar_label;
};
+ int ch5s_vector_grid_info(hid_t hdf_file, unsigned int grid_index, struct ch5s_vector_grid *info);

/*
* Manages freeing data contained within the #ch5s_vector_grid struct.
*/
+ int ch5s_vector_free_grid_info(struct ch5s_vector_grid *info);

/*
* Fetches vectors. Size for `out` is `sizeof(float) * 4 * (to_time-from_time+1)`
* and is a 3D array in the form:
*
*     out[time(0..to_time-from_time)][vector(0..n-1)][x(0),y(1),z(2),d(3)]
*/
+ int ch5s_vector_read_vecs(hid_t hdf_file, unsigned int grid_index, unsigned int from_time, unsigned int to_time, float *out);

/*
* Fetches vector data. Size for \p out is `sizeof(float) * (to_time-from_time+1)`
* and is a 2D array in the form:
*
*     out[time(0..to_time-from_time)][vector(0..n-1)]
*/
+ int ch5s_vector_read_data(hid_t hdf_file, unsigned int grid_index, unsigned int from_time, unsigned int to_time, float *out);

/*
* Creates a new grid with n vectors, t time steps, a time delta for each step
* and a label.  Returns the new grid's index.
*/
+ int ch5s_vector_create_grid(hid_t hdf_file, unsigned int n, unsigned int t, float time_delta, const char* grid_label, const char* scalar_label);

/*
* Writes vectors to the given grid index starting at the next time step.
* See ch5s_vector_get_vecs for \p in structure and size.
*/
(deferred) int ch5s_vector_write_next_vecs(hid_t hdf_file, unsigned int grid_index, float *in);

/*
* Writes data values (one for each vector) to the given grid index at the next
* time step.  See ch5s_vector_get_data for `in` structure and size.
*/
(deferred) int ch5s_vector_write_next_data(hid_t hdf_file, unsigned int grid_index, float *in);

/*
* Writes vectors to the given grid index starting at from_time upto and
* including to_time.  See ch5s_vector_get_vecs for `in` structure and size.
*/
+ int ch5s_vector_write_vecs(hid_t hdf_file, unsigned int grid_index, unsigned int from_time, unsigned int to_time, float *in);

/*
* Writes data values (one for each vector) to the given grid index over the
* specified time range.  See ch5s_vector_get_data for `in` structure and size.
*/
+ int ch5s_vector_write_data(hid_t hdf_file, unsigned int grid_index, unsigned int from_time, unsigned int to_time, float *in);


/***

    NODAL

***/
/*
* Returns number of nodal data grids.
*/
+ int ch5s_nodal_grid_count(hid_t hdf_file);

/*
* Populates an array with nodal data grid types (IGB or DYN_PTS) where the
* resultant array's indices match the grid indices.  Size for `types` is
* `sizeof(enum ch5s_nodal_type) * n` where n is the number of nodal
* data grids. Types are defined in an enumeration ch5s_nodal_type.
*/
enum ch5s_nodal_type { CH5_SCALAR, CH5_DYN_PTS };
(deferred) int ch5s_nodal_grid_types(hid_t hdf_file, enum ch5s_nodal_type *types);

/*
* Populates a `ch5_nodal_grid_info` struct with information about the given
* grid.
*/
struct ch5s_nodal_grid {
  unsigned int         time_steps;
  float                time_delta;
  unsigned int         num_nodes;
  enum ch5s_nodal_type type;
  char*                label;
};
+ int ch5s_nodal_grid_info(hid_t hdf_file, unsigned int grid_index, struct ch5s_nodal_grid *info);

/*
* Manages freeing any allocated memory used in ch5s_nodal_grid_info structs.
*/
+ int ch5s_nodal_free_grid_info(struct ch5s_nodal_grid *info);

/*
* Fetches the grid data for the given time slice (single time step).  Size of
* `out` is `sizeof(float) * 3 * n * (to_time-from_time+1)` where n is the number
* of nodes defined for the given grid.  The format of `out` is a 3D array with
* the form:
*
*     out[time(from_time..to_time)][node(0..n)][x(0),y(1),z(2)]
*/
+ int ch5s_nodal_read(hid_t hdf_file, unsigned int grid_index, unsigned int from_time, unsigned int to_time, float *out);

/*
* Fetches the entire time-series for a given node.  Size of `out` should be at
* least `sizeof(float) * 3 * t` where `t` is the number of time steps in the
* given grid (see the output of ch5s_nodal_grid_info). The format of `out` is
* a 2D array in the form:
*
*     out[time(0..t-1)][x(0),y(1),z(2)]
*/
+ int ch5s_nodal_read_time_series(hid_t hdf_file, unsigned int grid_index, unsigned int node_index, float *out);

/*
* Creates a new nodal grid with the number of nodes, n, the number of
* time steps, the time step delta, a label and a given type and returns the
* new grid's index.
*/
+ int ch5s_nodal_create_grid(hid_t hdf_file, unsigned int n, unsigned int t, float time_delta, enum ch5s_nodal_type type, const char* label);

/*
* Writes data to the given grid at the next time step. See ch5s_nodal_get_data
* for the array structure expected for `in`.
*/
(deferred) int ch5s_nodal_write_next_data(hid_t hdf_file, unsigned int grid_index, float *in);

/*
* Writes data to the given grid. See ch5s_nodal_get_data for the array
* structure expected for `in`.
*/
+ int ch5s_nodal_write(hid_t hdf_file, unsigned int grid_index, unsigned int from_time, unsigned int to_time, float *in);


/***

    AUX GRIDS

***/
/*
* Returns the number of grids in the file.
*/
int ch5s_aux_grid_count(hid_t hdf_file);

/*
* Populates the `info` struct with information on how many time steps are
* available for points, elements and data.  Possible values are:
*
*     points:      1, n
*     elements: 0, 1, n
*     data:     0, 1, n
*
* where `n` is a grid-wide time step defined by the number of points time
* steps.
*/
struct ch5s_aux_grid {
  unsigned int time_steps;
  float        time_delta;
  const char   *label;
  const char   *time_units;
  const char   *units;
  const char   *comments;
};
int ch5s_aux_grid_info(hid_t hdf_file, unsigned int grid_index, struct ch5s_aux_grid *info);

/*
* Manages freeing any allocated memory for ch5s_aux_grid structs.
*/
void ch5s_aux_free_grid_info(struct ch5s_aux_grid *info);

/*
* Fills a ch5s_aux_info struct with information about the number of points,
* elements and data for a particular time instance.
*/
struct ch5s_aux_time_step {
  unsigned int time_index;
  unsigned int num_pnts;
  unsigned int num_elem;
  unsigned int max_elem_width;
  bool has_data;
};
int ch5s_aux_time_step_info(hid_t hdf_file, unsigned int grid_index, unsigned int time, ch5s_aux_time_step *info);

int ch5s_aux_read(hid_t hdf_file, unsigned int grid_index, unsigned int time,
  float *points, int *elements, float *data);

/*
* Fetches the points data for the given grid index at the given time instance.
* See `ch5s_aux_time_info` for determining the number of points, n, to use
* when determining the size of `out` with `sizeof(float) * n * 3` with a
* structure of:
*
*     out[point(0..n-1)][x(0),y(1),z(2)]
*/
//int ch5s_aux_get_pnts(hid_t hdf_file, unsigned int grid_index, unsigned int time, float *out);

/*
* Fetches elements for the given grid at the given time instance. The size of
* `out` should be at least `sizeof(int) * n * (CH5_ELEM_MAX_VERTEX_COUNT + CH5_ELEM_WIDTH_ADD)`
* where `n` is the number of elements for this time instance (see the output
* of ch5s_aux_time_info).  Refer to the ch5m element documentation for details
* on the structure of the element data.
*/
//int ch5s_aux_get_elem(hid_t hdf_file, unsigned int grid_index, unsigned int time, int *out);

/*
* Fetches data values for a given grid and the given time instance.  The size
* of `out` should be at least `sizeof(float) * n` where `n` is the number of
* data values for this time instance (see output of ch5s_aux_time_info). The
* structure of out will be:
*
*     out[point(0..p-1)]
*
* where `p` is the total number of points for this time step (again, see the
* output of ch5s_aux_time_info for this).
*/
//int ch5s_aux_get_data(hid_t hdf_file, unsigned int grid_index, unsigned int time, float *out);

/*
* Creates a new grid and returns its index. t is the number of time steps for
* the grid, np is the number of points, ne is the number of elements and nd is
* the number of data "rows".
*
* TODO: checks that np == nd when nd > 1
*/
int ch5s_aux_create_grid(hid_t hdf_file, unsigned int t, float time_delta,
  const char *label, const char *time_units, const char *units,
  const char *comments);

int ch5s_aux_write_next(hid_t hdf_file, unsigned int grid_index,
  unsigned int npoints, float *points, unsigned int nelem,
  unsigned int max_elem_width, int *elem, float *data);

/*
* Writes n points to the given grid at `time`. Note that this function expects
* to be called sequentially with respect to time (e.g. 0,1,2,3 but not
* 0,2,1,3).  See `ch5s_aux_get_pnts` for the inbound data structure.
*/
//int ch5s_aux_write_next_pnts(hid_t hdf_file, unsigned int grid_index, unsigned int time, unsigned int n, float *in);

/*
* Writes n elements to the given grid at `time`. Note that this function
* expects to be called sequentially with respect to time (e.g. 0,1,2,3 but not
* 0,2,1,3).  See `ch5s_aux_get_elem` for the inbound data structure.
*/
//int ch5s_aux_write_next_elem(hid_t hdf_file, unsigned int grid_index, unsigned int time, unsigned int n, int *in);

/*
* Writes p data values to the given grid at `time` where p is the number of
* points for the given timestep. Note that this function expects to be called
* sequentially with respect to time (e.g. 0,1,2,3 but not 0,2,1,3).  See
* `ch5s_aux_get_elem` for the inbound data structure.
*
* FIXME: still unsure if every time step will have the same number of points
*        associated with it or if it varies over time thus making this dataset
*        a kind of ragged array.
*/
//int ch5s_aux_write_next_data(hid_t hdf_file, unsigned int grid_index, unsigned int time, float *in);



/***

    TIME SERIES

***/
/*
* Returns the number of time series stored in the file.
*/
int ch5s_series_count(hid_t hdf_file);

/*
* Populates a ch5s_series struct with the details of the given series.
* API consumer is responsible for freeing this struct with
* `ch5s_series_free_info`,
*/
enum ch5s_series_type { CH5_INT, CH5_FLOAT };
struct ch5s_series {
  unsigned int          time_steps;
  float                 time_delta;
  unsigned int          num_values;/* per time step */
  enum ch5s_series_type *format;/* array defining ordered types */
  char                  *label;
};
int ch5s_series_info(hid_t hdf_file, unsigned int series_index, struct ch5s_series *info);

/*
* Manages freeing any allocated data in a `ch5s_series` struct.
*/
int ch5s_series_free_info(struct ch5s_series *info);

/*
* Fetches the entire time series data for the given series and value. Size of
* `out` should be at least `sizeof(struct_for_format) * t` where t is the
* number of series timesteps (see ch5s_series_info) and struct_for_format is
* an API consumer defined structure that will hold the appropriate data types
* as defined in the series' format.  The structure of `out` will be:
*
*     out[time(0..t-1)]
*
* FIXME: This is all subject to change depending on how feasable this is to
*        implement in both HDF and C in general.
*/
int ch5s_series_read(hid_t hdf_file, unsigned int series_index, unsigned int value_index, void *out);

/*
* Creates a new series with the given time (t), the time delta and ordered
* type format returning its index.
*/
int ch5s_series_create(hid_t hdf_file, unsigned int t, float time_delta, const char *label, enum ch5s_series_type *format);

/*
* Writes all values for the next time step.  `in` should be a 2D array of
* structs that conform to the series' defined format which is in the form:
*
*     in[time_index(0..t-1)][value_index(0..n-1)]
*/
(deferred) int ch5s_series_write_next(hid_t hdf_file, unsigned int series_index, void *in);

/*
* Writes all values for a given range of time.  `in` should be a 2D array of
* structs that conform to the series' defined format which is in the form:
*
*     in[time_index(from_time..to_time-from_time-1)][value_index(0..n-1)]
*/
int ch5s_series_write(hid_t hdf_file, unsigned int series_index, unsigned int from_time, unsigned int to_time, void *in);



/***

    TEXT FILES

***/
/*
* Returns the number of text files stored in the file.
*/
int ch5s_text_count(hid_t hdf_file);

/*
* Populates a ch5s_text_file struct with the information for the given file
* identified by it's index.
*/
struct ch5s_text_file_info {
  unsigned int index;
  unsigned int size;
  char         *label;
};
int ch5s_text_info(hid_t hdf_file, int file_index, struct ch5s_text_file_info *info);

/*
* Manages freeing any memory allocated in the ch5s_text_free_info struct.
*/
void ch5s_text_free_info(struct ch5s_text_file_info *info);

/*
* Fetches text file data by the file's index into `out` which should be large
* enough to hold a number of chars equal or greater than the `size` field from
* the corresponding ch5s_text_file value.
*/
int ch5s_text_get_file(hid_t hdf_file, int file_index, char *out);

/*
* Creates a new file with a given label and returns the new file's index.
*/
int ch5s_text_create_file(hid_t hdf_file, const char *label);

/*
* Appends n chars of a string to the file identified by its index.
*/
int ch5s_text_append_to_file(hid_t hdf_file, unsigned int file_index, unsigned int n, char *in);

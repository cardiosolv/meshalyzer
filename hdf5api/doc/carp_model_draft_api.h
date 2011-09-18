/* Draft CARP/Meshalyzer HDF5 API */
/*
* Notes:
*   - Prefix ch5  (CARP hdf5)
*   - Prefix ch5m ("m" for model)
*   - Prefix ch5s ("s" for simulation)
*   - Functions marked with a + have been implemented
*/


/***

    FILE HANDLING

***/
+ int ch5_open(char const *path, hid_t *out_file_id);
+ int ch5_close(hid_t file_id);


/***

    METADATA

***/
struct ch5m_version {
  int major;
  int minor;
  int patch;
};
+ int   ch5m_meta_get_version(hid_t file_id, struct version *out_v);
+ int   ch5m_meta_set_version(hid_t file_id, struct version *in_v);
+ char* ch5_meta_get_name(hid_t file_id);
+ int   ch5_meta_set_name(hid_t file_id, char *in_name);


/***

    DATA

***/
struct ch5_dataset { /* used by all *_info functions */
  int count; /* number of elements */
  int width; /* values per element */
};

/* Connections */
+ int ch5m_conn_get_info(hid_t file_id, struct ch5_dataset *out_info);
+ int ch5m_conn_get_all(hid_t file_id, int *out_data);
+ int ch5m_conn_set_all(hid_t file_id, int *in_data);
+ int ch5m_conn_get_range(hid_t file_id, int start, int end, int *out_data);

/* Lons */
+ int ch5m_lons_get_info(hid_t file_id, struct ch5_dataset *out_info); /* width in out_info will take the order into account (3 or 6) */
+ int ch5m_lons_get_order(hid_t file_id, int *out_order);
+ int ch5m_lons_get_all(hid_t file_id, float *out_data);
+ int ch5m_lons_set_all(hid_t file_id, int order, float *in_data);
+ int ch5m_lons_get_range(hid_t file_id, int start, int end, float *out_data);

/* Points */
+ int ch5m_pnts_get_info(hid_t file_id, struct ch5_dataset *out_info);
+ int ch5m_pnts_get_all(hid_t file_id, float *out_data);
+ int ch5m_pnts_set_all(hid_t file_id, int n, float *in_data);
+ int ch5m_pnts_get_range(hid_t file_id, int start, int end, float *out_data);

/* Purkinje Fibres */
struct ch5m_pkje_cable_info {
  int   index;
  int   fathers[2];
  int   sons[2];
  int   node_count;
  float radius;
  float R_gj;
  float sigma_i;
  float *nodes; /* array of info->node_count rows of nodes in 3-tuples */
};
+ int ch5m_pkje_free_cable_set(struct ch5m_pkje_cable *cables);
+ int ch5m_pkje_get_info(hid_t file_id, struct ch5_dataset *out_info); /* width is unused in the output */
+ int ch5m_pkje_get_all(hid_t file_id, struct ch5m_pkje_cable *out_data);
+ int ch5m_pkje_set_all(hid_t file_id, int n, struct ch5m_pkje_cable *in_data);

/* Elements */
+ int ch5m_elem_get_info(hid_t file_id, int *out_info); /* note width in out_info is not used */
+ int ch5m_elem_get_one(hid_t file_id, int n, int *out_data);
+ int ch5m_elem_set_all(hid_t file_id, int n, int max_elem_width, int *in_data);

/* Surfaces */
+ int   ch5m_surf_get_count(hid_t file_id, int *count); /* returns number of surfaces */
+ int   ch5m_surf_open(hid_t file_id, int n, hid_t *out_surf_id);
+ int   ch5m_surf_create(hid_t file_id, char *name, hid_t *out_surf_id);
+ int   ch5m_surf_close(hid_t surf_id);
+ char* ch5m_surf_get_name(hid_t surf_id);
+ int   ch5m_surf_get_elem_info(hid_t surf_id, struct ch5_dataset *out_info); /* note width in out_info is not used */
/* The following are implemented through element functions defined above
+ int   ch5m_surf_elem_get_one(hid_t surf_id, int n, struct ch5m_element *out_data);
+ int   ch5m_surf_elem_set_all(hid_t surf_id, struct ch5m_element *in_data); */

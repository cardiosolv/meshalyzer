/**
* \file types.h
* \brief Defines types used throughout.
*/
#ifndef _CH5M_TYPES_H
#define _CH5M_TYPES_H

#if !defined(_CH5_H) && defined(__cplusplus)
extern "C" {
#endif

#include <stdbool.h>

#define CH5_CREATE 0
#define CH5_READ   1
#define CH5_WRITE  2

//Common attributes
#define CH5_T_ATTR             "number instances"
#define CH5_DELTA_T_ATTR       "time_delta"
#define CH5_T0_ATTR            "time origin"
#define CH5_LABEL_ATTR         "label"
#define CH5_TIME_UNITS_ATTR    "time_units"
#define CH5_UNITS_ATTR         "units"
#define CH5_COMMENTS_ATTR      "comments"

/**
* \struct ch5_version
* \brief Describes an HDF5 file's version
*/
struct ch5_version {
  int major; //!< Major version
  int minor; //!< Minor version
  int patch; //!< Patch version
};
typedef struct ch5_version ch5_version;

/**
* \struct ch5_dataset
* \brief Contains info for a basic two-dimensional dataset
* \details This is used primarily by model-related functions as the data
*          is often represented in a 2D fashion.  However, it would be nice
*          to expand this struct to encompas multi-dimensional information
*          and other information with respect to datasets.
*/
struct ch5_dataset {
  int count; //!< Number of elements
  int width; //!< Values per element
};
typedef struct ch5_dataset ch5_dataset;

/// @cond INTERNAL
/**
* \def CH5_ELEM_MAX_NODES
* \brief Maximum number of nodes in the most complex primitive.
*
* Used pimarily to define the size of an element's node data array.
*/
#define CH5_ELEM_MAX_NODES 8
/// @endcond

/**
* \enum ch5m_element_type
* \brief An ordered enumeration of element primitive types.
*
* These correspond directly to many lookup tables used by element-related
* functions.  Any change in order here should also be reflected in those
* lookup tables.
*/
enum ch5m_element_type {
  CH5_CONNECTION,
  CH5_TRIANGLE,
  CH5_QUADRILATERAL,
  CH5_TETRAHEDRON,
  CH5_PYRAMID,
  CH5_PRISM,
  CH5_HEXAHEDRON
};
typedef enum ch5m_element_type ch5m_element_type;

/**
* \struct ch5m_pkje_cable
* \brief Describes a single Purkinje fibre.
*/
struct ch5m_pkje_cable {
  int    index;      //!< The cable index (0-based)
  int    fathers[2]; //!< The cable's father cables (-1 indicates no father)
  int    sons[2];    //!< The cable's son cables (-1 indicates no son)
  int    node_count; //!< Number of nodes in \p nodes
  float  radius;     //!< Radius [micrometers]
  float  R_gj;       //!< Gap junction resistance [kOhm]
  float  sigma_i;    //!< Intracellular conductivity [Ohm-cm]
  double **nodes;    //!< Array of node_count rows of doubles in 3-tuples
};
typedef struct ch5m_pkje_cable ch5m_pkje_cable;

/**
* \struct ch5s_vector_grid
* \brief Describes a vector grid.
*/
struct ch5s_vector_grid {
  unsigned int time_steps;     //!< Number of time steps in the grid
  unsigned int max_time_steps; //!< Maximum number allocated Number of time steps in the grid
  float        time_delta;     //!< The change in time between time steps
  unsigned int num_vectors;    //!< The number of vectors represented in the grid
  unsigned int num_components; //!< The number of components in a vector (3 without scalar, 4 with scalar)
  const char   *label;         //!< An optional label describing the grid
  const char   *scalar_label;  //!< An optional label describing the 4th element of each vector
  const char   *time_units;    //!< An optional label describing the units used for time
  const char   *units;         //!< An optional label describing the units used for the grid data
  const char   *comments;      //!< An optional label with general comments about the grid
};
typedef struct ch5s_vector_grid ch5s_vector_grid;

/**
* \enum ch5s_nodal_type
* \brief An ordered enumeration of nodal data types.
*/
enum ch5s_nodal_type { CH5_SCALAR, CH5_DYN_PTS };
typedef enum ch5s_nodal_type ch5s_nodal_type;

/**
* \struct ch5s_nodal_grid
* \brief Describes a nodal grid and it's properties.
*/
struct ch5s_nodal_grid {
  unsigned int    time_steps;    //!< Number of time steps in the grid
  unsigned int    max_time_steps; //!< Maximum number allocated Number of time steps in the grid
  float           time_delta;  //!< The change in time between time steps
  float           t0;          //!< initial time
  unsigned int    num_nodes;   //!< The number of nodes represented in the grid
  unsigned int    node_width;  //!< The number of values per node (1 for CH5_SCALAR, 3 for CH5_DYN_PTS)
  ch5s_nodal_type type;        //!< The type of grid represented (CH5_SCALAR or CH5_DYN_PTS)
  const char      *label;      //!< An optional label describing the grid
  const char      *time_units; //!< An optional label describing the units used for time
  const char      *units;      //!< An optional label describing the units used for the grid data
  const char      *comments;   //!< An optional label with general comments about the grid
};
typedef struct ch5s_nodal_grid ch5s_nodal_grid;

/**
* \struct ch5s_series
* \brief Describes a time series of data with a user-defined format.
*/
struct ch5s_series {
  unsigned int time_steps;  //!< Number of time steps in the series
  unsigned int num_values;  //!< The number of values represented in the series (this does not include the first value - the time index - on each time step)
  const char   *label;      //!< An optional label describing the series
  const char   *quantities; //!< An optional label describing the quantities in the series
  const char   *units;      //!< An optional label describing the units used for the series
  const char   *comments;   //!< An optional label with general comments about the series
};
typedef struct ch5s_series ch5s_series;

/**
* \struct ch5s_aux_grid
* \brief Describes an auxiliary grid.
*/
struct ch5s_aux_grid {
  unsigned int time_steps;  //!< The number of time steps in the grid
  float        time_delta;  //!< The change in time between time steps
  float        t0;          //!< initial time 
  const char   *label;      //!< An optional label describing the grid
  const char   *time_units; //!< An optional label describing the units used for time
  const char   *units;      //!< An optional label describing the units used in the grid
  const char   *comments;   //!< An optional label with general comments about the grid
};
typedef struct ch5s_aux_grid ch5s_aux_grid;

/**
* \struct ch5s_aux_time_step
* \brief Describes individual time steps in an auxiliary grid.
*/
struct ch5s_aux_time_step {
  unsigned int num_points;        //!< The number of points stored at the time
  unsigned int num_elements;      //!< The number of elements stored at the time
  unsigned int max_element_width; //!< The maximum width of the elements stored
  bool         has_data;          //!< Flag indicating if there is data stored at the time
};
typedef struct ch5s_aux_time_step ch5s_aux_time_step;

/**
* \struct ch5s_text_file
* \brief Describes an individual text file.
*/
struct ch5_text_file {
  unsigned int  size;    //!< The number of strings
  unsigned int  storage; //!< The storage for the data
  const char   *label;   //!< An optional label describing/naming the file
};
typedef struct ch5_text_file ch5_text_file;

#if !defined(_CH5_H) && defined(__cplusplus)
}
#endif

#endif

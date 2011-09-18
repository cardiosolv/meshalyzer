/**
* \file ch5.h
* \brief Global include header for entire CH5 API.
*/

#ifndef _CH5_H
#define _CH5_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hdf5.h>
#include "file.h"
#include "generics.h"
#include "metadata.h"
#include "utils.h"
#include "text.h"
#include "types.h"

#include "model/cables.h"
#include "model/connections.h"
#include "model/elements.h"
#include "model/lons.h"
#include "model/points.h"
#include "model/purkinjes.h"
#include "model/regions.h"
#include "model/surfaces.h"

#include "simulation/aux_grids.h"
#include "simulation/nodal.h"
#include "simulation/time_series.h"
#include "simulation/vectors.h"

#ifdef __cplusplus
}
#endif

#endif

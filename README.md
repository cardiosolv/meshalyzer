# meshalyzer

Meshalyzer is a graphical program for display time dependent data on 3D finite elment meshes. Of course, it can show static data on fewer dimensions as well. 
It is developed to be compatible with the cardiac simulation environment [carpEntry](http://carp.medunigraz.at/carputils). 

It uses its own file formats which are simple and easily converted to/from more popiular formats like VTK.

## Features

* can work with very large data files (TB)
* offscreen rendering
* isosurfaces/lines
* cutting planes
* output images (with metadata) or sequences of images to make movies
* can save state
* link multiple instances to maintain view, colour scale, etc. across them
* can display time-dependent scalar and vector data
* display auxiliary grids
* fly through model views

## VTU format

Preliminary support for reading the VTK format for unstructured grids, VTU, has been included. You need to edit the **Makefile** manually to specify the directory containing the dynamically linked libraries (`VTK_LIBDIR`) and the directory with the VTK header files (`VTK_INCDIR`). They
are commented out by default.

For Mac users, the easiest is to install `vtk` via [homebrew](https://brew.sh/): `brew install vtk`

and then 

    VTK_LIBDIR = /usr/local/lib
    VTK_INCDIR = /usr/local/include/vtk-<version>

## Run-time issues

* Most of the time, a bad mesh or bad data file is responsible for crashes.
    Ensure that
    1. node indexing start at 0 and the number of nodes is correct
    2. there are no gaps in numbering
    3. the data file did not get truncated. Use  
    `igbhead --jive`  
    to correct if necessary.
    4. file formats are respected.

* It is possible that internally, a bad memory operation is performed. Try setting   
`export MALLOC_CHECK_=1`  
and see if it disappears.

* Make sure the proper dynamic libraries are getting linked. There can be conflicts with *fltk* or *vtk* if
  multiple versions are installed. In these cases a simple solution ids to write a wrapper script which 
  properly sets the environment variables like below:

  ```bash
  #!/bin/bash
  export LD_LIBRARY_PATH=/path_to_dylib1_dir:/path_to_dylib2_dir
  /path_to_meshalyzer/meshalyzer $*
  ```


#### Acknowledgement

The broadcast symbol was provided by [oNline Web Fonts](http://www.onlinewebfonts.com).

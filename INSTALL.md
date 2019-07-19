# Installation of meshalyzer #

Meshalyzer is regularly used on Linux and Mac machines. It has worked under Windows as well.

## Prerequisities ##

1. [libpng](http://libpng.org)
2. [FLTK 1.3.x](http://www.fltk.org/). Manually downloadad and installed is better since the prepackaged versions often have issues with fltk-config.
3. [glew](http://glew.sourceforge.net). 
2. [glut](https://www.opengl.org/resources/libraries/glut/)
3. To make the "windowless" rendering version, [OSMesa](https://www.mesa3d.org/osmesa.html)
4. To read in VTK VTU format meshes, install [VTK](https://www.vtk.org/) development files

Except for fltk, using your package manager is recommended for installing these packages if possible. On OSX, [homebrew](https://brew.sh/) works for me.

## Compiling ##

1. If VTK is installed, edit the **make.conf** to set the `VTK_INC` and `VTK_DIR` variables to point to the directories containing the VTK header files and libraries, respectively. Comment them out otherwise.

1. Type "make"

2. For the windowless version, type "make mesalyzer"

3. The compiled executables will be found in the **src/** directory.

3. Copy the binaries to a suitable location

## Documentation ##

To compile (LaTeX required)

1. cd manual
2. pdflatex manual
3. pdflatex manual

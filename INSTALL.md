# Installation of meshalyzer #

Meshalyzer is regularly used on Linux and Mac machines. It has worked under Windows as well.

## Prerequisities ##

1. [libpng](http://libpng.org)
2. [FLTK 1.3](http://www.fltk.org/). Manually downloadad and installed is better since the prepackaged versions often have issues with fltk-config.
2. [glut](https://www.opengl.org/resources/libraries/glut/)
3. To make the "windowless" rendering version, [OSMesa](https://www.mesa3d.org/osmesa.html)

Except for fltk, using your package manager is recommended for installing these packages if possible.

## Compiling ##

1. Type "make"

2. For the windowless version, type "make mesalyzer"

3. Copy the binaries to a suitable location

## Documentation ##

1. cd manual
2. pdflatex manual
3. pdflatex manual

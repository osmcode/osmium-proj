# Projection class for Libosmium

This repository contains code for projecting OSM locations to arbitrary
coordinate reference systems. It is based on the PROJ library. Unlike
the code which is included in libosmium which only supports older
PROJ versions, this one is for PROJ version 6 and above.

**This is untested code just provided as a starting point, because libosmium
doesn't support PROJ 6 and above.**

To use this include `projection.hpp` in your program and instantiate the
`osmium::geom::GeometryFactory` class with the class `PROJ_Projection` as
template parameter. See the libosmium documentation for details of how to
use the `GeometryFactory` class.

You need to compile with `libproj`.


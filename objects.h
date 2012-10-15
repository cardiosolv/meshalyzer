#ifndef OBJECTS_H
#define OBJECTS_H

typedef enum objects{
  Vertex, Cable, Cnnx, SurfEle, Surface, VolEle, RegionDef, Nothing, All } Object_t;

static const char *objnames[] = { "Vertex", "Cable", "Cnnx", "SurfaceEdge", "Surface", 
  "Volume", "RegionDef", "Nothing", "All" };

static const int maxobject=6;

#endif

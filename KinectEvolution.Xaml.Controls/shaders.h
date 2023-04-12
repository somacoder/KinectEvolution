// shared constants between shader and cpp files
#ifndef __SHADERS_H__
#define __SHADERS_H__

// NOTE these enums must match VizDepthVertexMode in NuiViz.h
#define VERTEX_MODE_POINT                   0 //Point               = 0, // depth rendered as POINTLIST
#define VERTEX_MODE_SURFACE                 1 //Surface             = 1, // surface mesh with Position|Tex|Color
#define VERTEX_MODE_SURFACE_WITH_NORMAL     2 //SurfaceWithNormal   = 2, // calculate normal for the surface
#define VERTEX_MODE_SURFACE_WITH_UV         3 //SurfaceWithUV       = 3, // use explicit UVs for the surface texture
#define VERTEX_MODE_POINTXYZ                4 //PointXYZ            = 4, // the same vertex format as surface, but rendered as POINTLIST
#define VERTEX_MODE_POINTSPRITE             5 //PointSprite         = 5, // point sprite rendering, requires d3d10 device
#define VERTEX_MODE_POINTSPRITE_WITH_NORMAL 6

#define RAMP_MODE_NONE                  0

#define SURFACE_TEXTURE_MODE_NONE       0 // no texture
#define SURFACE_TEXTURE_MODE_DIRECT     1 // direct texture fetch
#define SURFACE_TEXTURE_MODE_YUV        2 // YUV texture that requires conversion in shader

#define DEPTH_TRIANGLE_THRESHOLD_MM     200 // threshold in mm for adjacent points to form a triangle

#endif

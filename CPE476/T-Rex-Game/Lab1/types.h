#ifndef __TYPES_H__
#define __TYPES_H__

/* Color struct */
typedef struct color_struct {
   float r;//double r;
   float g;//double g;
   float b;//double b;
   //double f; // "filter" or "alpha"
} color_t;

typedef struct material_struct {
   color_t reflect;
   color_t refract;
   float refrIdx;
   float smoothness;
   float roundness;
   int objId;
} mat_t;

typedef struct bounding_box {
   float left;
   float right;
   float top;
   float bottom;
   float front;
   float back;
} bound_t;

#endif

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

int randRange(int low, int high);
float frandRange(float low, float high);
float randAngle();
int randRightAngle();
glm::vec3 randDirection();

float p2i_x(int p_x, int width, int height);
float p2i_y(int p_y, int width, int height);
float p2w_x(int p_x, int width, int height);
float p2w_y(int p_y, int width, int height);

#endif

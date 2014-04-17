#include "Utils.h"

int randRange(int low, int high) {
   return rand() % (high - low) + low;
}

float frandRange(float low, float high) {
   return (high - low) * static_cast<float>(rand()) / (static_cast<float>(RAND_MAX)) + low;
}

float randAngle() {
   return rand() / static_cast<float>(RAND_MAX/(360.0));
}

int randRightAngle() {
   return randAngle() > 180 ? 0 : 90;
}

glm::vec3 randDirection() {
   return glm::normalize(glm::vec3(frandRange(-1.0, 1.0), 0.0, frandRange(-1.0, 1.0)));
}


float p2i_x(int p_x, int width, int height) {
  float x_i = ( (float)p_x - ((width-1.0)/2.0) )*2.0/width;
  return(x_i);
}

float p2i_y(int p_y, int width, int height) {
  return( ( (float)p_y - ((height-1.0)/2.0) )*2.0/height);
}

float p2w_x(int p_x, int width, int height) {
  float x_i = ( (float)p_x - ((width-1.0)/2.0) )*2.0/width;
  return(((float)width/(float)height)*x_i);
}

float p2w_y(int p_y, int width, int height) {
  return( ( (float)p_y - ((height-1.0)/2.0) )*2.0/height);
}

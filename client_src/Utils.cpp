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

glm::vec3 orthogTo(glm::vec3 baseV) {
   float rand1 = frandRange(-1, 1);
   float rand2 = frandRange(-1, 1);
   glm::vec3 newVec;
   if (baseV.x) {
      float calc = -(baseV.y*rand1 + baseV.z*rand2) / baseV.x;
      newVec = glm::vec3(calc, rand1, rand2);
   } else if (baseV.y) {
      float calc = -(baseV.x*rand1 + baseV.z*rand2) / baseV.y;
      newVec = glm::vec3(rand1, calc, rand2);
   } else if (baseV.z) {
      float calc = -(baseV.x*rand1 + baseV.y*rand2) / baseV.z;
      newVec = glm::vec3(rand1, rand2, calc);
   } else
      printf("This should not happen!!!\n");

   return glm::normalize(newVec);
}

glm::mat4 matrixToAlign(glm::vec3 from, glm::vec3 to) {
   glm::vec3 axis = glm::cross(from, to);
   float dotted = glm::dot(from, to);
   float inRatio = glm::length(axis);   
   float angle = 0;
   if (dotted < 0) {
      angle = 180;
      inRatio = -inRatio;
   }
   angle += 180/M_PI * glm::asin(inRatio);
   glm::mat4 RotateMat = glm::rotate(glm::mat4(1.0f), angle, axis);

   return glm::rotate(glm::mat4(1.0f), angle, axis);
}


/*2D coord transforms - slightly off, but fine for user interaction for p2 */
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
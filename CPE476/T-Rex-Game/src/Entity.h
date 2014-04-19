#include <iostream>

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif

#ifdef __unix__
#include <GL/glut.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "glew32.lib")

#include <GL\glew.h>
#include <GL\glut.h>
#endif

#include <string>
#include <map>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" 
#include "glm/gtc/type_ptr.hpp" 
#include <algorithm>
#include "CMeshLoaderSimple.h"
#include "GLSL_helper.h"
#include "glglobals.h"

typedef struct Mesh {
  int triangleCount;
  GLuint vertBufObj, colBufObj, normBufObj;
} Mesh;

class Entity {
public:
  Entity(std::string meshfile) { 
    _id = _nextId++; 
    if (meshes.find(meshfile) == meshes.end()) {
      meshes[meshfile] = new Mesh(); 
      auto mesh = meshes[meshfile];
      CMeshLoader::loadVertexBufferObjectFromMesh(meshfile, mesh->triangleCount,
         mesh->vertBufObj, mesh->colBufObj, mesh->normBufObj);
    }

    _meshfile = meshfile;
  }

  virtual void step();
  void draw() {
    auto mesh = meshes[_meshfile];

    //TODO: SET MODEL

    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufObj);
    safe_glVertexAttribPointer(h_aPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);

    safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->normBufObj);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, mesh->triangleCount * 3);
  }

  int getId() { return _id; }
  
  glm::vec3 pos = glm::vec3(0.f, 0.f, 0.f);
  glm::vec3 vel = glm::vec3(0.f, 0.f, 0.f);
  double scale  = 1.f;
private:
  static int _nextId;
  static std::map<std::string, Mesh*> meshes;

  int _id;
  std::string _meshfile;
};


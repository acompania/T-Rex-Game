//#include "Entity.h"

#ifdef __unix__
// -holy crap!! These are all for glfw...
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/Xcursor/Xcursor.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "GLSL_helper.h"

//Handles to the shader data
GLint h_aPosition;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uShadeType;
GLint h_uCamPos;
GLint h_uSun;
GLint h_uLightColor;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;
GLint h_uTexUnit, h_aTexCoord;

int shader;

/*function to help load the shader  - note current version only loading a vertex shader */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
   GLuint VS, FS; //handles to shader Model
   GLint vCompiled, fCompiled, linked; //status of shader
   
   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);
   
   //load the source
   glShaderSource(VS, 1, &vShaderName, NULL);
   glShaderSource(FS, 1, &fShaderName, NULL);
   
   //compile shader and print log
   glCompileShader(VS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
   printShaderInfoLog(VS);

   //compile shader and print log
   glCompileShader(FS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
   printShaderInfoLog(FS);

   if (!vCompiled || !fCompiled) { 
      printf("Error compiling the shader %s", vShaderName);
      return 0;
   }
    
   //create a program Model and attach the compiled shader
   shader = glCreateProgram();
   glAttachShader(shader, VS);
   glAttachShader(shader, FS);
   
   glLinkProgram(shader);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(shader, GL_LINK_STATUS, &linked);
   printProgramInfoLog(shader);

   glUseProgram(shader);
   
   /* get handles to attribute data */
   h_aPosition = safe_glGetAttribLocation(shader, "aPosition");
   h_aNormal = safe_glGetAttribLocation(shader, "aNormal");
   h_uProjMatrix = safe_glGetUniformLocation(shader, "uProjMatrix");
   h_uViewMatrix = safe_glGetUniformLocation(shader, "uViewMatrix");
   h_uModelMatrix = safe_glGetUniformLocation(shader, "uModelMatrix");

   h_uShadeType = safe_glGetUniformLocation(shader, "uShadeType");
   h_uCamPos = safe_glGetUniformLocation(shader, "uCamPos");
   h_uSun = safe_glGetUniformLocation(shader, "uSun");

   h_uLightColor = safe_glGetUniformLocation(shader, "uLColor");
   h_uMatAmb = safe_glGetUniformLocation(shader, "uMat.aColor");
   h_uMatDif = safe_glGetUniformLocation(shader, "uMat.dColor");
   h_uMatSpec = safe_glGetUniformLocation(shader, "uMat.sColor");
   h_uMatShine = safe_glGetUniformLocation(shader, "uMat.shine");

   h_aTexCoord = safe_glGetAttribLocation(shader,  "aTexCoord");
   h_uTexUnit = safe_glGetUniformLocation(shader, "uTexUnit");

   printf("sucessfully installed shader %d\n", shader);
   return 1;
}

int main(void)
{
   GLFWwindow* window;

   /* Initialize the library */
   if (!glfwInit())
     return -1;

   /* Create a windowed mode window and its OpenGL context */
   window = glfwCreateWindow(640, 480, "T-Rex Gaiden", NULL, NULL);
   if (!window) {
     glfwTerminate();
     return -1;
   }
   
      /* Make the window's context current */
   glfwMakeContextCurrent(window);
   
   glClearColor(0.5, 0, 0, 1.0f);                       
   //glClearDepth (1.0f);     // Depth Buffer Setup
   //glDepthFunc (GL_LEQUAL); // The Type Of Depth Testing
   glEnable(GL_DEPTH_TEST);   // Enable Depth Testing
   
   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"../shaders/vert.glsl"), textFileRead((char *)"../shaders/frag.glsl")))   {
      printf("Error installing shader!\n");
      return 0;
   }
   
   glUseProgram(shader);

   /* Loop until the user closes the window */
   while (!glfwWindowShouldClose(window)) {
      /* Render here */

      /* Swap front and back buffers */
      glfwSwapBuffers(window);

      /* Poll for and process events */
      glfwPollEvents();
   }

   glfwTerminate();
   return 0;
}

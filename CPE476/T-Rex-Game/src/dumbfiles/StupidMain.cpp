#ifdef __unix__
// -holy crap!! These are all for glfw...
#include <GLFW/glfw3.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/Xcursor/Xcursor.h>
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "GLSL_helper.h"
#include <fstream>
#include "glglobals.h"
#include <iostream>
#include <string>

#include "CMeshLoaderSimple.h"

//#include "BunnyEntity.h"

#define uint unsigned int
#define FPS 30

typedef struct {
   GLuint vbo, cbo, nbo;
   int faceCount; 
} Model;


static int ShadeProg;
static glm::vec3 sunShade;
static int g_shadeType;
static int g_width = 1, g_height = 1;

static float g_pitch, g_yaw;
static glm::vec3 eye, target, up = glm::vec3(0.0, 1.0, 0.0), sunDir;

static BunnyEntity bunnyModel;

void loadStaticModel(Model* model, std::string name) {
   CMeshLoader::loadVertexBufferObjectFromMesh(name, model->faceCount,
    model->vbo, model->cbo, model->nbo);
}

void InitGeom() {   
   loadStaticModel(&bunnyModel, "../resources/bunny500.m");
}

/* projection matrix */
void SetProjectionMatrix() {
   glm::mat4 Projection = glm::perspective(90.0f, (float)g_width/g_height, 0.1f, 250.f);
   safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
   glUniform3f(h_uCamPos, eye.x, eye.y, eye.z);
   glm::mat4 View = glm::lookAt(eye, target, up);
   safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(View));
}

void SetModel(Model * model) {
  	glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(0, 0, 0));
  	glm::mat4 Scale = glm::scale( glm::mat4(1.0f), glm::vec3(1));

  	glm::mat4 all = Trans*Scale;
  	// send the model matrix
  	safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(all));
}

/* set the Model transform to the identity */
void SetModelI() {
  glm::mat4 tmp = glm::mat4(1.0f);
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(tmp));
}

void drawModel(Model * model) {
   SetModel(model);

   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, model->nbo);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glDrawArrays(GL_TRIANGLES, 0, model->faceCount);
}

/* Main display function */
void Draw (void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //Start our shader   
   glUseProgram(ShadeProg);
   
   SetModelI();
   /* Set up the projection and view matrices */
   SetProjectionMatrix();
   SetView();

   /* Set up the light's direction and color */
   glUniform3f(h_uLightColor, sunShade.x, sunShade.y, sunShade.z);
   glUniform3f(h_uSun, sunDir.x, sunDir.y, sunDir.z);

   // set the normal flag
   glUniform1i(h_uShadeType, g_shadeType);

 // ======================== draw square stuff =========================

      drawModel(&bunnyModel);

 // ================== end of bird stuff ====================

   //clean up 
   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);

   //disable the shader
   glUseProgram(0);
}



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
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);
   
   glLinkProgram(ShadeProg);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   printProgramInfoLog(ShadeProg);

   glUseProgram(ShadeProg);
   
   /* get handles to attribute data */
   h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
   h_aNormal = safe_glGetAttribLocation(ShadeProg, "aNormal");
   h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");

   h_uShadeType = safe_glGetUniformLocation(ShadeProg, "uShadeType");
   h_uCamPos = safe_glGetUniformLocation(ShadeProg, "uCamPos");
   h_uSun = safe_glGetUniformLocation(ShadeProg, "uSun");

   h_uLightColor = safe_glGetUniformLocation(ShadeProg, "uLColor");
   h_uMatAmb = safe_glGetUniformLocation(ShadeProg, "uMat.aColor");
   h_uMatDif = safe_glGetUniformLocation(ShadeProg, "uMat.dColor");
   h_uMatSpec = safe_glGetUniformLocation(ShadeProg, "uMat.sColor");
   h_uMatShine = safe_glGetUniformLocation(ShadeProg, "uMat.shine");

   h_aTexCoord = safe_glGetAttribLocation(ShadeProg,  "aTexCoord");
   h_uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");

   printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
}


void mouseMove(int x, int y) {
   directCamera(x, y);
}

void passiveMove(int x, int y) {
#ifdef __APPLE__
   directCamera(x, y);
#endif
}

void moveForward() {
   glm::vec3 gaze = target - eye;
   glm::vec3 W = normalize(gaze);

   eye += glm::vec3(.5*W.x, .5*W.y, .5*W.z);
   target += glm::vec3(.5*W.x, .5*W.y, .5*W.z);
}

void moveBackward() {
   glm::vec3 gaze = target - eye;
   glm::vec3 W = normalize(gaze);

   eye -= glm::vec3(.5*W.x, .5*W.y, .5*W.z);
   target -= glm::vec3(.5*W.x, .5*W.y, .5*W.z);
}

void strafeLeft() {
   glm::vec3 gaze = target - eye;
   glm::vec3 W = glm::normalize(-gaze);
   glm::vec3 U = glm::normalize(cross(up, W));

   eye -= glm::vec3(.4*U.x, .4*U.y, .4*U.z);
   target -= glm::vec3(.4*U.x, .4*U.y, .4*U.z);
}

void strafeRight() {
   glm::vec3 gaze = target - eye;
   glm::vec3 W = glm::normalize(-gaze);
   glm::vec3 U = glm::normalize(cross(up, W));

   eye += glm::vec3(.4*U.x, .4*U.y, .4*U.z);
   target += glm::vec3(.4*U.x, .4*U.y, .4*U.z);
}

void rise() {
   eye += glm::vec3(.1*up.x, .1*up.y, .1*up.z);
   target += glm::vec3(.1*up.x, .1*up.y, .1*up.z);
}

void fall() {
   eye -= glm::vec3(.1*up.x, .1*up.y, .1*up.z);
   target -= glm::vec3(.1*up.x, .1*up.y, .1*up.z);
}

//the keyboard callback to change the values to the transforms
void keyboard(unsigned char key, int x, int y ) {
   // printf("%c pressed\n", key);
   switch( key ) {
      case 'w':
         moveForward();
         break;
      case 's':
         moveBackward();
         break;
      case 'a':
         strafeLeft();
         break;
      case 'd':
         strafeRight();
         break;
      case 32: // space
         rise();
         break;
      case 'c':
         fall();
         break;
      case 'n':
         g_shadeType = g_shadeType == NORMAL ? PHONG : NORMAL;
         break;
      case 'q': case 'Q' :
         exit( EXIT_SUCCESS );
         break;
   }
   //glutPostRedisplay();
}

void tock(int value) {
   updateBoidPos();
   glutPostRedisplay();
   glutTimerFunc(TIMER_DELAY, tock, 0);
}

// ======================================================================= //
// =============================== Main ================================== //
// ======================================================================= //

int main( int argc, char *argv[] )
{
   glutInit( &argc, argv );
   glutInitWindowPosition( 200, 200 );
   glutInitWindowSize( 800, 500 );
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   glutCreateWindow("World of Awesome");
   glutReshapeFunc( ReshapeGL );
   glutDisplayFunc( Draw );
   glutKeyboardFunc( keyboard );
   glutMouseFunc( mouse );
   glutMotionFunc( mouseMove );
   glutPassiveMotionFunc( passiveMove );
   glutTimerFunc(TIMER_DELAY, tock, 0);

   g_width = g_height = 200;

#ifdef _WIN32 
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
      std::cerr << "Error initializing glew! " << glewGetErrorString(err) << std::endl;
      return 1;
   }
#endif
#ifdef __APPLE__
   glutSetCursor(GLUT_CURSOR_NONE); 
#endif

   backShade = glm::vec3(0.2,0.5,0.9);

   Initialize();
   
   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"shaders/vert.glsl"), textFileRead((char *)"shaders/frag.glsl")))   {
      printf("Error installing shader!\n");
      return 0;
   }

   InitGeom();


   g_shadeType = PHONG;

   g_pitch = 0;
   g_yaw = M_PI / 2;
   float tx = cos(g_pitch)*cos(g_yaw);
   float ty = sin(g_pitch);
   float tz = cos(g_pitch)*cos(M_PI/2 - g_yaw);
   eye = glm::vec3(0, 2.5, 0);
   target = eye + glm::vec3(tx, ty, tz);
   sunDir = normalize(vec3(-0.2, -1.0, 0.0));


   sunShade = glm::vec3(1.0, 1.0, 0.9);

   glutMainLoop();
   return 0;
}

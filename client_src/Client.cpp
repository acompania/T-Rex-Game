/*
  Base code for program 3 for CSC 471
  OpenGL, glut and GLSL application
  Loads in a .m mesh file into a VBO and draws it
  Uses glm for matrix transforms
  I. Dunn and Z. Wood  (original .m loader by H. Hoppe)
*/

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef __unix__
#include <GL/glut.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "glew32.lib")

#include <GL\glew.h>
#include <GL\glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "GLSL_helper.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "MStackHelp.h"
#include "Utils.h"
#include "TextureLoader.h"
#include "mesh.h"
#include <fstream>

#define NORMAL 1
#define PHONG 2
#define PITCH_LIMIT 1.3962634 // 80 DEGREES
#define BIRD_COUNT 50
#define SPACE 32

using namespace std;
using namespace glm;

Image TextureImage;

RGB myimage[64][64];
RGB* g_pixel;

typedef struct {
   GLuint vbo, tbo, nbo;
   uint vertCount;    
   int material;
} ObjModel;

ObjModel sphereModel, cubeModel, shipModel;

vec3 birdTarget;
static float timeCount = 0;

//flag and ID to toggle on and off the shader
int shade = 1;
int ShadeProg;
int TriangleCount;
int g_mat_id = 0;

static float g_width, g_height;
int g_startx = 0, g_starty = 0, g_endx = 0, g_endy = 0;
int g_mode;

vec3 backShade, sunShade;
int g_shadeType;

float g_pitch, g_yaw;
vec3 camPos, camTarg, camUp = vec3(0.0, 1.0, 0.0), sunDir;

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

//declare a matrix stack
RenderingHelper Stack;

#define TIMER_DELAY 30


// ======================================================================== //
// ============================ INIT GEOMETRY ============================= //
// ======================================================================== //


/* initialize the geomtry (including color)
   Change file name to load a different mesh file
*/
void loadStaticModel(ObjModel* obj, string name, int matNum) {
  std::ifstream modelFile(name.c_str());
   Model model;
   model.load(modelFile);
   obj->vertCount = model.meshes()[0].makeVBO(NULL, &obj->vbo, &obj->tbo, &obj->nbo);
   obj->material = matNum;
}

void InitGeom() {
   loadStaticModel(&sphereModel, "objs/sphere.obj", 0);
   loadStaticModel(&cubeModel, "objs/cube.obj", 0);
}

// ======================================================================== //
// =========================== INIT USED DATA ============================= //
// ======================================================================== //

/* helper function to set camUp material for shading */
void SetMaterial(int i) {

   glUseProgram(ShadeProg);
   switch (i) {
      case 0:  // ocean blue
        safe_glUniform3f(h_uMatAmb, 0.08,0.3,0.4);  
        safe_glUniform3f(h_uMatDif, 0.2, 0.4,  0.7);  
        safe_glUniform3f(h_uMatSpec,  0.3,  0.7,  0.8); 
        safe_glUniform1f(h_uMatShine, 20.0); 
        break;
   }
}

/* projection matrix */
void SetProjectionMatrix() {
   mat4 Projection = perspective(90.0f, (float)g_width/g_height, 0.1f, 250.f);
   safe_glUniformMatrix4fv(h_uProjMatrix, value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
   glUniform3f(h_uCamPos, camPos.x, camPos.y, camPos.z);
   mat4 View = lookAt(camPos, camTarg, camUp);
   safe_glUniformMatrix4fv(h_uViewMatrix, value_ptr(View));
}

/* Model transforms */
void SetModel() {
   safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(Stack.modelViewMatrix));
}

/* set the Model transform to the identity */
void SetModelI() {
  mat4 tmp = mat4(1.0f);
  safe_glUniformMatrix4fv(h_uModelMatrix, value_ptr(tmp));
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

/* Some OpenGL initialization */
void Initialize ()               // Any GL Init Code 
{
   // Start Of User Initialization
   glClearColor(backShade.x, backShade.y, backShade.z, 1.0f);                       
   // Black Background
   //glClearDepth (1.0f);     // Depth Buffer SetcamUp
   //glDepthFunc (GL_LEQUAL); // The Type Of Depth Testing
   glEnable(GL_DEPTH_TEST);   // Enable Depth Testing

   /* some matrix stack init */
   Stack.useModelViewMatrix();
   Stack.loadIdentity();

   /* texture specific settings */
   glEnable(GL_TEXTURE_2D);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   // camera and lighting initialization
   g_shadeType = PHONG;

   g_pitch = 0;
   g_yaw = M_PI / 2;
   float tx = cos(g_pitch)*cos(g_yaw);
   float ty = sin(g_pitch);
   float tz = cos(g_pitch)*cos(M_PI/2 - g_yaw);
   camPos = vec3(0, 2.5, 0);
   camTarg = camPos + vec3(tx, ty, tz);
   sunDir = normalize(vec3(-0.2, -1.0, 0.0));

   sunShade = vec3(1.0, 1.0, 0.9);
}


// ======================================================================= //
// ============================== DRAWING ================================ //
// ======================================================================= //


void drawModel(ObjModel * model) {
   SetModel();
   SetMaterial(model->material);

   // glEnable(GL_TEXTURE_2D);
   // glActiveTexture(GL_TEXTURE0);
   // glBindTexture(GL_TEXTURE_2D, 0);
   // safe_glUniform1i(h_uTexUnit, 0);

   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, model->nbo);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // safe_glEnableVertexAttribArray(h_aTexCoord);
   // glBindBuffer(GL_ARRAY_BUFFER, model->tbo);
   // safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); 

   glDrawArrays(GL_TRIANGLES, 0, model->vertCount);

   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);
   // safe_glDisableVertexAttribArray(h_aTexCoord);
   // glDisable(GL_TEXTURE_2D);
}

/* Main display function */
void Draw (void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //Start our shader   
   glUseProgram(ShadeProg);
   
   SetModelI();
   /* Set camUp the projection and view matrices */
   SetProjectionMatrix();
   SetView();

   /* Set camUp the light's direction and color */
   glUniform3f(h_uLightColor, sunShade.x, sunShade.y, sunShade.z);
   glUniform3f(h_uSun, sunDir.x, sunDir.y, sunDir.z);

   // set the normal flag
   glUniform1i(h_uShadeType, g_shadeType);

 // ===================== Draw Shtuff!! =========================

   Stack.pushMatrix();
   Stack.translate(vec3(5, 0, 0));
   Stack.scale(2);
   drawModel(&cubeModel);
   Stack.popMatrix();

   Stack.pushMatrix();
   Stack.translate(vec3(0, 0, 5));
   Stack.scale(2);
   drawModel(&sphereModel);
   Stack.popMatrix();

 //==============================================================

   //disable the shader
   glUseProgram(0);  
   glutSwapBuffers();
}

// ======================================================================= //
// ============================= CONTROLS ================================ //
// ======================================================================= //

/* Reshape */
void ReshapeGL (int width, int height) {
   g_width = (float)width;
   g_height = (float)height;
   glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));
   glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
   if (state == GLUT_DOWN) {
      g_startx = x;
      g_starty = g_height-y-1;
   }
   glutPostRedisplay();
}

void sendCameraInput(int x, int y) {

}

void mouseMove(int x, int y) {
   sendCameraInput(x, y);
}

void passiveMove(int x, int y) {
#ifdef __APPLE__
   sendCameraInput(x, y);
#endif
}

//the keyboard callback to change the values to the transforms
void keyboard(unsigned char key, int x, int y ) {
   //printf("%d pressed\n", key);

   switch( key ) {
      case SPACE:
         // do something
         break;
      case 'c':
         // do something
         break;
      case 'a':
         // do something
         break;
      case 'd':
         // do something
         break;
      case 's':
         // do something
         break;
      case 'w':
         // do something
         break;
      case 'n':
         g_shadeType = g_shadeType == NORMAL ? PHONG : NORMAL;
         glutPostRedisplay();
         break;
      case 'q': case 'Q':
         exit( EXIT_SUCCESS );
         break;
   }
}

void tock(int value) {
   //glutPostRedisplay();
   //glutTimerFunc(TIMER_DELAY, tock, 0);
}

// ======================================================================= //
// =============================== Main ================================== //
// ======================================================================= //

int main( int argc, char *argv[] ) {

   glutInit( &argc, argv );
   glutInitWindowPosition( 200, 200 );
   glutInitWindowSize( 800, 500 );
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   glutCreateWindow("T-Rex");
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

   backShade = vec3(0.2,0.5,0.9);

   Initialize();

   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"vert_shader.glsl"), textFileRead((char *)"frag_shader.glsl")))   {
      printf("Error installing shader!\n");
      return 0;
   }

   InitGeom();

   //LoadTexture((char *)"sky.bmp", 0, &TextureImage);

   glutMainLoop();

   return 0;
}


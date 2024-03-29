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
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include <fstream>

#define NORMAL 1
#define PHONG 2
#define PITCH_LIMIT 1.3962634 // 80 DEGREES
#define BIRD_COUNT 50

using namespace std;
using namespace glm;

typedef struct {
   GLuint vbo, ibo, tbo, nbo;
   uint indexCount;    
   int material;
} ObjModel;

ObjModel waveModel, birdModel, skyModel;

vec3 birdPositions[BIRD_COUNT];
vec3 birdVelocities[BIRD_COUNT];
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
vec3 eye, target, up = vec3(0.0, 1.0, 0.0), sunDir;

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

GLuint CubeBuffObj, CIndxBuffObj, TexBuffObj;
int g_CiboLen;


#define TIMER_DELAY 30


// ======================================================================== //
// ============================ INIT GEOMETRY ============================= //
// ======================================================================== //


/* initialize the geomtry (including color)
   Change file name to load a different mesh file
*/
void loadStaticModel(ObjModel* obj, string name) {
   // CMeshLoader::loadStaticObjectMesh(name, 
   //    model->vbo, model->ibo, model->tbo, model->nbo, model->indexCount);

   //std::ifstream modelFile(name);
   //Model model;
   //model.load(modelFile);
   //obj->indexCount = model.meshes()[0].makeVBO(NULL, &obj->vbo, &obj->tbo, &obj->nbo);
}

void InitGeom() {   
   loadStaticModel(&skyModel, "cube.obj");
   skyModel.material = 0;

   loadStaticModel(&birdModel, "sphere.obj");
   birdModel.material = 0;
}

// ======================================================================== //
// =========================== INIT USED DATA ============================= //
// ======================================================================== //

/* helper function to set up material for shading */
void SetMaterial(int i) {

   glUseProgram(ShadeProg);
   switch (i) {
      case 0:  // ocean blue
        safe_glUniform3f(h_uMatAmb, 0.08,0.3,0.4);  
        safe_glUniform3f(h_uMatDif, 0.2, 0.4,  0.7);  
        safe_glUniform3f(h_uMatSpec,  0.3,  0.7,  0.8); 
        safe_glUniform1f(h_uMatShine, 20.0); 
        break;

      case 1:  // sky
        safe_glUniform3f(h_uMatAmb, 0.0, 0.0, 0.0); 
        safe_glUniform3f(h_uMatDif, 0.0, 0.0, 0.0);  
        safe_glUniform3f(h_uMatSpec,  0.0,  0.0,  0.0); 
        safe_glUniform1f(h_uMatShine, 0.0); 
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
   glUniform3f(h_uCamPos, eye.x, eye.y, eye.z);
   mat4 View = lookAt(eye, target, up);
   safe_glUniformMatrix4fv(h_uViewMatrix, value_ptr(View));
}

/* Model transforms */
void SetModel() {
   //safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(Stack.modelViewMatrix));
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
   //glClearDepth (1.0f);     // Depth Buffer Setup
   //glDepthFunc (GL_LEQUAL); // The Type Of Depth Testing
   glEnable(GL_DEPTH_TEST);   // Enable Depth Testing


}


// ======================================================================= //
// ============================== DRAWING ================================ //
// ======================================================================= //


void drawModel(ObjModel * model) {
   SetModel();
   SetMaterial(model->material);

   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, model->vbo);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, model->nbo);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glDrawArrays(GL_TRIANGLES, 0, model->indexCount);
}

#define MIN_DIST 15
#define TARG_WEIGHT 0.25f
#define CENT_WEIGHT 0.3f
#define AVOID_WEIGHT 0.4f

void updateBoidPos() {
   birdTarget = vec3(10 * timeCount - 100, 
                     25 + 5.0 * sin(1.2 * timeCount + sin(timeCount)),
                     10.0 * sin(timeCount));


   vec3 centroid = vec3(0);
   for (int i = 0; i < BIRD_COUNT; i++)
      centroid += birdPositions[i];
   centroid /= BIRD_COUNT;

   for (int i = 0; i < BIRD_COUNT; i++) {
      vec3 toLeader = normalize(birdTarget - birdPositions[i]);
      vec3 toCentroid = normalize(centroid - birdPositions[i]);

      vec3 neighborsToMe = vec3(0);
      int neighborCount = 0;
      for (int j = 0; j < BIRD_COUNT; j++)
         if (j != i) {
            vec3 dist = birdPositions[i] - birdPositions[j];
            if (length(dist) < MIN_DIST) {
               neighborsToMe += vec3(dist.x, dist.y, dist.z);
               neighborCount++;
            }
         }
      if (neighborCount)
         neighborsToMe = normalize(neighborsToMe);

      birdVelocities[i] *= 0.93;
      birdVelocities[i] += TARG_WEIGHT * toLeader 
                    + CENT_WEIGHT * toCentroid
                    + AVOID_WEIGHT * neighborsToMe;
      birdPositions[i] += birdVelocities[i];
   }

   timeCount += 0.1;
   if (timeCount > 20)
      timeCount = 0;
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

 // ===================== Draw The sky =========================

 // ======================== bird stuff =========================

   // for (int i = 0; i < BIRD_COUNT; i++) {
   //    Stack.pushMatrix();
   //    Stack.translate(birdPositions[i]);
   //    drawModel(&birdModel);
   //    Stack.popMatrix();
   // }

 // ================== end of bird stuff ====================

   //clean up 
   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);

   //disable the shader
   glUseProgram(0);  
   glutSwapBuffers();
}

// ======================================================================= //
// ============================= CONTROLS ================================ //
// ======================================================================= //

/* Reshape */
void ReshapeGL (int width, int height)
{
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

void directCamera(int x, int y) {
   /*g_endx = x;
   g_endy = g_height-y-1;

   if (!g_startx || !g_endx) {
      g_startx = g_endx;
      g_starty = g_endy;
      return;
   }

   float startWX = p2w_x(g_startx, g_width, g_height);
   float startWY = p2w_y(g_starty, g_width, g_height);
   float endWX = p2w_x(g_endx, g_width, g_height);
   float endWY = p2w_y(g_endy, g_width, g_height);
   if (g_startx != g_endx || g_starty != g_endy) {
      g_pitch = g_pitch + (endWY - startWY) * M_PI / 2.0;
      g_yaw = g_yaw + (endWX - startWX) * M_PI / 2.0;
      if (g_pitch >= PITCH_LIMIT)
         g_pitch = PITCH_LIMIT;
      if (g_pitch <= -PITCH_LIMIT)
         g_pitch = -PITCH_LIMIT;
      if (g_yaw >= 2.0 * M_PI)
         g_yaw -= 2.0 * M_PI;
      if (g_yaw < 0)
         g_yaw += 2.0 * M_PI;

      float tx = cos(g_pitch)*cos(g_yaw);
      float ty = sin(g_pitch);
      float tz = cos(g_pitch)*cos(M_PI/2 - g_yaw);
      target = eye + vec3(tx, ty, tz);
   }

   g_startx = g_endx;
   g_starty = g_endy;

   if (fabs(g_width/2 - x) > g_width/4 || fabs(g_height/2 - y) > g_height/4) {
#ifdef __APPLE__
      int winX = glutGet(GLUT_WINDOW_X);
      int winY = glutGet(GLUT_WINDOW_Y);
      CGPoint warpPoint = CGPointMake(winX + g_width/2, winY + g_height/2);
      CGWarpMouseCursorPosition(warpPoint);
      CGAssociateMouseAndMouseCursorPosition(true);
#endif
      g_startx = g_width/2;
      g_starty = g_height/2;
   }
   glutPostRedisplay();*/
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
   vec3 gaze = target - eye;
   vec3 W = normalize(gaze);

   eye += vec3(.5*W.x, .5*W.y, .5*W.z);
   target += vec3(.5*W.x, .5*W.y, .5*W.z);
}

void moveBackward() {
   vec3 gaze = target - eye;
   vec3 W = normalize(gaze);

   eye -= vec3(.5*W.x, .5*W.y, .5*W.z);
   target -= vec3(.5*W.x, .5*W.y, .5*W.z);
}

void strafeLeft() {
   vec3 gaze = target - eye;
   vec3 W = normalize(-gaze);
   vec3 U = normalize(cross(up, W));

   eye -= vec3(.4*U.x, .4*U.y, .4*U.z);
   target -= vec3(.4*U.x, .4*U.y, .4*U.z);
}

void strafeRight() {
   vec3 gaze = target - eye;
   vec3 W = normalize(-gaze);
   vec3 U = normalize(cross(up, W));

   eye += vec3(.4*U.x, .4*U.y, .4*U.z);
   target += vec3(.4*U.x, .4*U.y, .4*U.z);
}

void rise() {
   eye += vec3(.1*up.x, .1*up.y, .1*up.z);
   target += vec3(.1*up.x, .1*up.y, .1*up.z);
}

void fall() {
   eye -= vec3(.1*up.x, .1*up.y, .1*up.z);
   target -= vec3(.1*up.x, .1*up.y, .1*up.z);
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

   backShade = vec3(0.2,0.5,0.9);

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
   eye = vec3(0, 2.5, 0);
   target = eye + vec3(tx, ty, tz);
   sunDir = normalize(vec3(-0.2, -1.0, 0.0));


   sunShade = vec3(1.0, 1.0, 0.9);

   glutMainLoop();
   return 0;
}


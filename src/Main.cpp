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

#include "TextEntity.h"
#include "CMeshLoaderSimple.h"
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "MStackHelp.h"
#include <chrono>
GLint h_aPosition;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uLightPos;
GLint h_uLightColor;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;
GLint h_uTexUnit, h_aTexCoord;

GLint h_uShadeType;
GLint h_uCamPos;
GLint h_uSun;

int shader;

//-----=-=-=-=--=-=-=-=-=-=-==--=-=-=-=-=-=-==-=--=-=-=-==--=-=-=-==-

#define NORMAL 1
#define PHONG 2
#define PITCH_LIMIT 1.3962634 // 80 DEGREES

using namespace std;
using namespace glm;

typedef struct {   // Component of a structure
   GLuint triBuffObj, colBuffObj, normalBuffObj;
   int triangleCount;    
   glm::vec3 pointToParent;   // point of attachment to its parent
} Model;

typedef struct Object{
   Model *base;
   glm::vec3 position;      // only used if this has no parent
   float scale;
   glm::vec3 direction;
   float speed;
   std::vector<glm::vec3> pointsToChildren;  // connection locations to its children
   std::vector<Object> children;        // connected children
   int material;         // index to a material
   int touched;
} Object;

Model bunnyModel, groundModel, flashlightModel;
Object ground, flashlight;
std::vector<Object> bunnies;

//flag and ID to toggle on and off the shader
int shade = 1;
int ShadeProg;
int TriangleCount;
int g_mat_id = 0;

static float g_width, g_height;
int g_startx = 0, g_starty = 0, g_endx = 0, g_endy = 0;
int g_mode;

glm::vec3 lightPos;
glm::vec3 backShade = glm::vec3(0,0,0);
glm::vec3 sunShade = glm::vec3(1.0, 1.0, 0.9);
int g_shadeType;

float g_pitch, g_yaw;
glm::vec3 eye, target, up = glm::vec3(0.0, 1.0, 0.0), lightDirection;

unsigned long lastTime, timePassed = 0, createBunnyCounter = 0;;

//Handles to the shader data
GLint h_uFlashlightDirection;
GLint h_uFlashlightPosition;

//declare a matrix stack
RenderingHelper Stack;

glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest){
   start = glm::normalize(start);
   dest = glm::normalize(dest);

   float cosTheta = glm::dot(start, dest);
   glm::vec3 rotationAxis;

   if (cosTheta < -1 + 0.001f){
     rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
     if (glm::length(rotationAxis) < 0.01 ) 
      rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);
      rotationAxis = glm::normalize(rotationAxis);
      return glm::angleAxis(180.0f, rotationAxis);
    }

    rotationAxis = glm::cross(start, dest);

    float s = sqrt( (1+cosTheta)*2 );
    float invs = 1 / s;

    return glm::quat(
        s * 0.5f, 
        rotationAxis.x * invs,
        rotationAxis.y * invs,
        rotationAxis.z * invs
    ); 
}

// Get rotation matrix from two vectors
glm::mat4 vec3sToMat4(const glm::vec3& object, const glm::vec3& target) {
    glm::vec3 delta =  (target-object);
    glm::vec3 desiredUp(0,1,0.00001); 
    glm::quat rot1 = RotationBetweenVectors(glm::vec3(0,0,1), delta);
    glm::vec3 right = glm::cross(delta, desiredUp);
    desiredUp = glm::cross(right, delta);
  
    glm::vec3 newUp = rot1 * glm::vec3(0.0f, 1.0f, 0.0f);
    glm::quat rot2 = RotationBetweenVectors(newUp, desiredUp);
    glm::quat targetOrientation = rot2 * rot1;
    glm::mat4 M=glm::toMat4(targetOrientation);
    M[3][0] = object.x;
    M[3][1] = object.y;
    M[3][2] = object.z;
    return M;
}

glm::mat4 dirToMat(const glm::vec3& direction) {
   return vec3sToMat4(glm::vec3(0.0f), direction);
}


// ======================================================================== //
// ============================ INIT GEOMETRY ============================= //
// ======================================================================== //

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

/* initialize the geomtry (including color)
   Change file name to load a different mesh file
*/
void loadModel(Model* Model, string name) {
   CMeshLoader::loadVertexBufferObjectFromMesh(name, 
      Model->triangleCount, Model->triBuffObj, 
      Model->colBuffObj, Model->normalBuffObj);
}

void createGround() {
   ground.base = &groundModel;

   ground.position = glm::vec3(0, 0, 0);
   ground.scale = 100;
   ground.direction = glm::vec3(1, 0, 0);
   ground.material = 4;
}

void createFlashlight() {
   flashlight.base = &flashlightModel;
   flashlight.position = glm::vec3(0, 1, 1);
   flashlight.scale = 1;
   flashlight.direction = glm::vec3(1, 0, 0);
   flashlight.material = 2;
}

void createBunny() {
   Object bunny;
   bunny.base = &bunnyModel;
   bunny.position = glm::vec3(frandRange(-50, 50), 2, frandRange(-50, 50));
   bunny.speed = frandRange(2, 15);
   bunny.scale = 5;
   bunny.direction = glm::vec3(frandRange(0, 2*M_PI), 0, frandRange(0, 2*M_PI));
   bunny.material = randRange(0, 4);
   bunny.touched = 0;
   bunnies.push_back(bunny);
}

Text* txt = new Text();
void showText() {
  txt->setText("Hello World!");
  // txt->pos.x = 10;
  // txt->pos.y = 50;
}

void InitGeom() {
   loadModel(&groundModel, "resources/ground.obj");
   loadModel(&flashlightModel, "resources/flashlight.obj");
   loadModel(&bunnyModel, "resources/bunny500.m");
   
   createGround();
   createFlashlight();
   showText();
}

// ======================================================================== //
// =========================== INIT USED DATA ============================= //
// ======================================================================== //

/* helper function to set up material for shading */
void SetMaterial(int i) {

   glUseProgram(ShadeProg);
   switch (i) {
      case 0:  // redish (big tree)
        safe_glUniform3f(h_uMatAmb, 0.0,0.0,0.0);  
        safe_glUniform3f(h_uMatDif, 0.25, 0.2,  0.1);  
        safe_glUniform3f(h_uMatSpec,  1.0,  0.813,  0.7); 
        safe_glUniform1f(h_uMatShine, 150.0); 
        break;
      case 1:  // greenish (leaf)
        safe_glUniform3f(h_uMatAmb, 0.0,0.0,0.0);
        safe_glUniform3f(h_uMatDif, 0.3, 0.4, 0.3);
        safe_glUniform3f(h_uMatSpec, 0.0, 0.0, 0.0);
        safe_glUniform1f(h_uMatShine, 1.0);
        break;
      case 2:  // red (flashlight)
        safe_glUniform3f(h_uMatAmb, 0.2,0.03,0.0);
        safe_glUniform3f(h_uMatDif, 0.5,0.1,0.0);
        safe_glUniform3f(h_uMatSpec, 0.9,0.7,0.0);
        safe_glUniform1f(h_uMatShine, 50.0);
        break;
      case  3:  // brown (skinny tree)
        safe_glUniform3f(h_uMatAmb, 0.0,0.0,0.0); 
        safe_glUniform3f(h_uMatDif, 0.25,  0.25, 0.15); 
        safe_glUniform3f(h_uMatSpec,  .9, 1.0, 0.6); 
        safe_glUniform1f(h_uMatShine, 120.0); 
        break;  
      case  4:  // dull light green (ground)
        safe_glUniform3f(h_uMatAmb, 0.0,0.0,0.0);  
        safe_glUniform3f(h_uMatDif, 0.45,  0.55,  0.4); 
        safe_glUniform3f(h_uMatSpec,  0.05,  0.2,  0.0); 
        safe_glUniform1f(h_uMatShine, 4.0); 
        break;
      case  5:  // Yellooooowww
        safe_glUniform3f(h_uMatAmb, 0.2,0.2,0.0);  
        safe_glUniform3f(h_uMatDif, 0.5,  0.6,  0.1); 
        safe_glUniform3f(h_uMatSpec,  0.8,  0.9,  0.1); 
        safe_glUniform1f(h_uMatShine, 200.0); 
   }
}

/* projection matrix */
void SetProjectionMatrix() {
   glm::mat4 Projection = perspective(90.0f, (float)g_width/g_height, 0.1f, 100.f);
   safe_glUniformMatrix4fv(h_uProjMatrix, value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
   glUniform3f(h_uCamPos, eye.x, eye.y, eye.z);
   glUniform3f(h_uFlashlightDirection, lightDirection.x, lightDirection.y, lightDirection.z);
   glm::mat4 View = lookAt(eye, target, up);
   safe_glUniformMatrix4fv(h_uViewMatrix, value_ptr(View));
}

/* Model transforms */
void SetModel() {
   safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(Stack.modelViewMatrix));
}

/* set the Model transform to the identity */
void SetModelI() {
  glm::mat4 tmp = glm::mat4(1.0f);
  safe_glUniformMatrix4fv(h_uModelMatrix, value_ptr(tmp));
}

//-=--=-==--=-=--=-=-==--=-=-==--=-=-=-=-=-=-=-=-=-=--=-=-=--=-==--=-=


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

// -=-=-=--=-=-=-=-==-=-=--==-=-=--=-=-=-=-=-=-=-=-=-=-=-=


void drawObject(Object * obj, Object * parent, int index) {
   Stack.pushMatrix();
   
   Stack.rotateWith(dirToMat(obj->direction));
   Stack.scale(obj->scale, obj->scale, obj->scale);
   Stack.translate(obj->position);
   
   SetModel();
   SetMaterial(obj->material);
   
   // set up access to the vertices and normals
   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, obj->base->triBuffObj);
   safe_glVertexAttribPointer(h_aPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, obj->base->normalBuffObj);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   // Finally draw it
   glDrawArrays(GL_TRIANGLES, 0, obj->base->triangleCount*3);

   Stack.popMatrix();
}

/* Main display function */
int score = 0;
int shitty_fps = 30;

void Draw(void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //Start our shader   
   glUseProgram(ShadeProg);

   /* Set up the projection and view matrices */
   SetProjectionMatrix();
   SetView();

   /* Set up the light's position and color */
   glUniform3f(h_uFlashlightPosition, flashlight.position.x, flashlight.position.y, flashlight.position.z);
   glUniform3f(h_uLightColor, sunShade.x, sunShade.y, sunShade.z);

   // set the normal flag
   glUniform1i(h_uShadeType, g_shadeType);

   // Draw Everything
   drawObject(&ground, NULL, 0);
   for (int i=0; i < bunnies.size(); i++)
      drawObject(&bunnies[i], NULL, 0);
   drawObject(&flashlight, NULL, 0);

   //clean up 
   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);

   //disable the shader
   glUseProgram(0);  

   txt->draw(score, bunnies.size(), shitty_fps);

   //glutSwapBuffers();
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
}

/*2D coord transforms - slightly off, but fine for user interaction for p2 */
float p2i_x(int p_x) {
  float x_i = ( (float)p_x - ((g_width-1.0)/2.0) )*2.0/g_width;
  return(x_i);
}

float p2i_y(int p_y) {
  return( ( (float)p_y - ((g_height-1.0)/2.0) )*2.0/g_height);
}

float p2w_x(int p_x) {
  float x_i = ( (float)p_x - ((g_width-1.0)/2.0) )*2.0/g_width;
  return(((float)g_width/(float)g_height)*x_i);
}

float p2w_y(int p_y) {
  return( ( (float)p_y - ((g_height-1.0)/2.0) )*2.0/g_height);
}

void mouse(int button, int state, int x, int y) {
   if (state == GLUT_DOWN) {
      g_startx = x;
      g_starty = g_height-y-1;
   }
}

glm::mat4 matrixToAlign(glm::vec3 from, glm::vec3 to) {
   glm::vec3 axis = cross(from, to);
   float dotted = dot(from, to);
   float inRatio = glm::length(axis);   
   float angle = 0;
   if (dotted < 0) {
      angle = 180;
      inRatio = -inRatio;
   }
   angle += 180/M_PI * glm::asin(inRatio);
   glm::mat4 RotateMat = glm::rotate(glm::mat4(1.0f), angle, axis);

   return rotate(glm::mat4(1.0f), angle, axis);
}

void directCamera(int x, int y) {
   g_endx = x;
   g_endy = g_height-y-1;

   if (!g_startx || !g_endx) {
      g_startx = g_endx;
      g_starty = g_endy;
      return;
   }

   float startWX = p2w_x(g_startx);
   float startWY = p2w_y(g_starty);
   float endWX = p2w_x(g_endx);
   float endWY = p2w_y(g_endy);
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
      target = eye + glm::vec3(tx, ty, tz);

      glm::vec3 gaze = target - eye;
      glm::vec3 W = normalize(-gaze);
      glm::vec3 U = normalize(cross(up, W));
      glm::vec3 V = cross(W, U);
      flashlight.position = eye + -W + U - V;
      flashlight.direction = glm::normalize(gaze);
      lightDirection = -W;
   }

   g_startx = g_endx;
   g_starty = g_endy;
   
#ifdef __APPLE__
   if (fabs(g_width/2 - x) > g_width/4 || fabs(g_height/2 - y) > g_height/4) {
      int winX = glutGet(GLUT_WINDOW_X);
      int winY = glutGet(GLUT_WINDOW_Y);
      CGPoint warpPoint = CGPointMake(winX + g_width/2, winY + g_height/2);
      CGWarpMouseCursorPosition(warpPoint);
      CGAssociateMouseAndMouseCursorPosition(true);

      g_startx = g_width/2;
      g_starty = g_height/2;
   }
#endif
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

   eye += glm::vec3(.5*W.x, 0, .5*W.z);
   target += glm::vec3(.5*W.x, 0, .5*W.z);
   flashlight.position += glm::vec3(.5*W.x, 0, .5*W.z);
}

void moveBackward() {
   glm::vec3 gaze = target - eye;
   glm::vec3 W = normalize(gaze);

   eye -= glm::vec3(.5*W.x, 0, .5*W.z);
   target -= glm::vec3(.5*W.x, 0, .5*W.z);
   flashlight.position -= glm::vec3(.5*W.x, 0, .5*W.z);
}

void strafeLeft() {
   glm::vec3 gaze = target - eye;
   glm::vec3 W = normalize(-gaze);
   glm::vec3 U = normalize(cross(up, W));

   eye -= glm::vec3(.4*U.x, 0, .4*U.z);
   target -= glm::vec3(.4*U.x, 0, .4*U.z);
   flashlight.position -= glm::vec3(.4*U.x, 0, .4*U.z);
}

void strafeRight() {
   glm::vec3 gaze = target - eye;
   glm::vec3 W = normalize(-gaze);
   glm::vec3 U = normalize(cross(up, W));

   eye += vec3(.4*U.x, 0, .4*U.z);
   target += vec3(.4*U.x, 0, .4*U.z);
   flashlight.position += vec3(.4*U.x, 0, .4*U.z);
}

void detectCollisions() {
   for (int i = 0; i < bunnies.size(); i++) {
      if (!bunnies[i].touched) {
      
         // hit edge of square
         if (bunnies[i].position.x < -50) {
            bunnies[i].position.x = -50;
            bunnies[i].direction *= -1;
         } else if (bunnies[i].position.x > 50) {
            bunnies[i].position.x = 50;
            bunnies[i].direction *= -1;
         }  
         if (bunnies[i].position.z < -50) {
            bunnies[i].position.z = -50;
            bunnies[i].direction *= -1;
         } else if (bunnies[i].position.z > 50) {
            bunnies[i].position.z = 50;
            bunnies[i].direction *= -1;
         }
         
         // hit another bunny
         for (int j = 0; j < bunnies.size(); j++) {
            float dist = glm::distance(bunnies[i].position, bunnies[j].position);
            if (i != j && dist < 5) {
               bunnies[i].direction *= -1;
               bunnies[i].position += normalize(bunnies[i].direction) * glm::vec3(0.2);
               if (!bunnies[j].touched) {
                  bunnies[j].direction *= -1;
                  bunnies[j].position += normalize(bunnies[j].direction) * glm::vec3(0.2);
               }
            }
         }
         
         // hit player
         float dist = glm::distance(bunnies[i].position, eye);
         if (dist < 5) {
            bunnies[i].touched = 1;
            bunnies[i].speed = 0;
            bunnies[i].material = 5;
            // update score
            score += 1;
         }
      }
   }
}

void tick(int stupid) {
   unsigned long curTime = std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::system_clock::now().time_since_epoch()).count();
   
   int delta = curTime - lastTime;
   shitty_fps = delta == 0 ? 30 : 1000 / delta;
   timePassed += delta;
   createBunnyCounter += delta;
   if (createBunnyCounter > 1500) {
      createBunny();
      createBunnyCounter -= 1500;
   }
   
   // update position
   for (int i = 0; i < bunnies.size(); i++)   
      bunnies[i].position += glm::normalize(bunnies[i].direction) * (bunnies[i].speed * delta / 1000.0f);
      
   detectCollisions();

   lastTime = curTime;
   glutPostRedisplay();
   glutTimerFunc(20, tick, 0);
}

//-=--=-=-=-=-=-=-=-==--=-=-=-=-=-==-=-=-=-=-=--==--=-=-==--=-=--=-=


static void error_callback(int error, const char* description) {
    fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   
   if (action == GLFW_PRESS) {
      printf("%d pressed\n", key);
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
         case 'n':
            g_shadeType = g_shadeType == NORMAL ? PHONG : NORMAL;
            break;
         case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
      }
   }
}

int main(void)
{
   GLFWwindow* window;
   glfwSetErrorCallback(error_callback);

   // Initialization code-----
   InitGeom();
   glClearColor(0.5, 0.5, 0, 1.0f);                       
   glEnable(GL_DEPTH_TEST);   // Enable Depth Testing   GLFWwindow* window;
      /* some matrix stack init */
   Stack.useModelViewMatrix();
   Stack.loadIdentity();
   // -------------------------
   
   /* Initialize the library */
   if (!glfwInit())
     return -1;

   /* Create a windowed mode window and its OpenGL context */
   window = glfwCreateWindow(640, 480, "T-Rex Gaiden", NULL, NULL);
   if (!window) {
     glfwTerminate();
     return -1;
   }
   glfwMakeContextCurrent(window);

   // Init the shader ---------
   getGLversion();
   if (!InstallShader(textFileRead((char *)"shaders/vert.glsl"), 
    textFileRead((char *)"shaders/frag.glsl"))) {
      printf("Error installing shader!\n");
      return 0;
   }
   glUseProgram(shader);
   //--------------------------
   
   // window events -----------
   glfwSetKeyCallback(window, key_callback);
   glfw
   // -------------------------

   // GLFW MAIN LOOP
   while (!glfwWindowShouldClose(window)) {

        Draw();

        //glEnd();
        glfwSwapBuffers(window);
        glfwPollEvents();
   }

   glfwTerminate();
   return 0;
}

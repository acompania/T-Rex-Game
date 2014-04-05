
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#include <fftw3.h>

#include <stdlib.h>
#include <stdio.h>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr

#include "briskhelper.h"
#include "composer.h"

using namespace std;

#define MAX_POINTS 20

static float g_width = 400;
static float g_height = 400;
static float g_Win = 1.0;
static int g_delay = 40;


//position and color data handles
GLuint triBuffObj;

//flag and ID to toggle on and off the shader
int shade = 1;
int ShadeProg;

//Handles to the shader data
GLint h_aPosition;
GLint h_uWinSize;
GLint h_uTimeBlock;
GLint h_uFreqBlock;
GLint h_uCancBlock;
GLint h_uMaxTimeAmp;
GLint h_uMaxFreqAmp;


/* the vertex data */
// static GLfloat timeBlock[FRAMES_PER_BUFFER];
static GLfloat freqBlock[FRAMES_PER_BUFFER];
// static GLfloat cancBlock[FRAMES_PER_BUFFER];

static GLfloat maxTimeAmp, maxFreqAmp;

static GLfloat vertexPos[] = {
 -1, -1,
 -1, 1,
 1, 1,
 1, -1,
};

/* initialize the geomtry (including color)*/
void InitGeom() {
   glGenBuffers(1, &triBuffObj);

   glBindBuffer(GL_ARRAY_BUFFER, triBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPos), vertexPos, GL_STREAM_DRAW);
}


/*function to help load the shader */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
   GLuint VS; //handles to shader object
   GLuint FS; //handles to frag shader object
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
      printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
      return 0;
   }
    
   //create a program object and attach the compiled shader
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
   h_uWinSize = safe_glGetUniformLocation(ShadeProg, "uWinSize");
   // h_uTimeBlock = safe_glGetUniformLocation(ShadeProg, "uTimeBlock");
   h_uFreqBlock = safe_glGetUniformLocation(ShadeProg, "uFreqBlock");
   // h_uCancBlock = safe_glGetUniformLocation(ShadeProg, "uCancBlock");
   h_uMaxFreqAmp = safe_glGetUniformLocation(ShadeProg, "uMaxFreqAmp");
   // h_uMaxTimeAmp = safe_glGetUniformLocation(ShadeProg, "uMaxTimeAmp");

   printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
   
}

/* Some OpenGL initialization */
void Initialize ()               // Any GL Init Code 
{
   // Start Of User Initialization
   glClearColor (1.0f, 1.0f, 1.0f, 1.0f);
   glPointSize(4);
   // Black Background
   glClearDepth (1.0f); // Depth Buffer Setup
   glDepthFunc (GL_LEQUAL);   // The Type Of Depth Testing
   glEnable (GL_DEPTH_TEST);// Enable Depth Testing
}

/* Main display function */
void Draw (void)
{
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           
   //Start our shader   
   glUseProgram(ShadeProg);

   //data set up to access the vertices and color
   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, triBuffObj);
   safe_glVertexAttribPointer(h_aPosition, 2, GL_FLOAT, GL_FALSE, 0, 0);

   glUniform2f(h_uWinSize, g_width, g_height);
   glUniform1f(h_uMaxTimeAmp, maxTimeAmp);
   glUniform1f(h_uMaxFreqAmp, maxFreqAmp);
   // glUniform1fv(h_uTimeBlock, FRAMES_PER_BUFFER, timeBlock);
   glUniform1fv(h_uFreqBlock, FRAMES_PER_BUFFER, freqBlock);
   // glUniform1fv(h_uCancBlock, FRAMES_PER_BUFFER, cancBlock);

   //actually draw the data
   glDrawArrays(GL_POLYGON, 0, 4);

   //clean up 
   safe_glDisableVertexAttribArray(h_aPosition);
   //disable the shader
   glUseProgram(0);  
   glutSwapBuffers();
}

/* Reshape */
void ReshapeGL (int width, int height)                      
{
   g_width = width;
   g_height = height;
   glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));           
}

void findMaxes() {
   // maxTimeAmp = 0;
   // for (int i = 0; i < FRAMES_PER_BUFFER; i++)
   //    if (fabs(timeBlock[i]) > maxTimeAmp)
   //       maxTimeAmp = fabs(timeBlock[i]);
   maxFreqAmp = 0;
   for (int i = 0; i < FRAMES_PER_BUFFER; i++)
      if (freqBlock[i] > maxFreqAmp)
         maxFreqAmp = freqBlock[i];
   // for (int i = 0; i < FRAMES_PER_BUFFER; i++)
   //    if (cancBlock[i] > maxFreqAmp)
   //       maxFreqAmp = cancBlock[i];
}

void Tick (int blah) {
   // getTimeBuff(timeBlock);
   getFreqBuff(freqBlock);
   // getCancBuff(cancBlock);
   findMaxes();
   glutPostRedisplay();
   glutTimerFunc(g_delay, Tick, 0);
}

//the keyboard callback
void keyboard(unsigned char key, int x, int y ){
   switch( key ) {
    case 'p':
         togglePlaying();
         break;
      case 'r':
         toggleRecording();
         break;
      case 32: // space
         togglePlaying();
         toggleRecording();
         break;
      case 'n':
         nextMode();
         break;
      case 'a':
         decreaseLFOAmp();
         break;
      case 's':
         increaseLFOAmp();
         break;
      case 'z':
         decreaseLFORate();
         break;
      case 'x':
         increaseLFORate();
         break;
      case 'd':
         decreaseFlangeDepth();
         break;
      case 'f':
         increaseFlangeDepth();
         break;
      case 'c':
         decreaseFlangeRate();
         break;
      case 'v':
         increaseFlangeRate();
         break;
      case '=':
         increaseTempo();
         break;
      case '-':
         decreaseTempo();
         break;
      case 'q': case 'Q' :
         takedown();
         exit( EXIT_SUCCESS );
         break;
   }
}

int main(int argc, char *argv[]) {
   glutInit( &argc, argv );
   glutInitWindowPosition( 20, 20 );
   glutInitWindowSize( 400, 200 );
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   glutCreateWindow("Whistling Madness");
   glutReshapeFunc( ReshapeGL );
   glutDisplayFunc( Draw );
   glutKeyboardFunc( keyboard );
   glutTimerFunc(0, Tick, 0);
   Initialize();
   
   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"vertShader.glsl"), textFileRead((char *)"fragShader.glsl"))) {
      printf("Error installing shader!\n");
      return 0;
   }
   InitGeom();
   
   setup();

   glutMainLoop();
   return 0;
}
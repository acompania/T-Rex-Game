struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

attribute vec3 aClr;
attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aUV;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uInverseTranspose;
uniform vec3 uLightPos;
uniform vec3 uLColor;
uniform vec3 uCamPos;
uniform Material uMat;

varying vec3 vColor;
varying vec3 vPos;
varying vec3 vNorm;
varying vec2 vTexCoord;
void main() {
  vec4 vPosition, transNorm;
  
  /* First model transforms */
  vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1);
  vPos = vec3(vPosition.x, vPosition.y, vPosition.z);
  vPosition = uViewMatrix* vPosition;
  gl_Position = uProjMatrix*vPosition;
  transNorm = uInverseTranspose * vec4(aNormal.x, aNormal.y, aNormal.z, 0.0);
  /* TO DO REPLACE THIS WITH lighting!!!! */
  vNorm = normalize(vec3(transNorm.x, transNorm.y, transNorm.z));
  vTexCoord = aUV;
  vColor = aClr;
}

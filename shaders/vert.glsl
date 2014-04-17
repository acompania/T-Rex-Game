
struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

attribute vec3 aPosition;
attribute vec3 aNormal;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uCamPos;
uniform vec3 uFlashlightPosition;

uniform int uShadeType;

varying vec3 vColor;
varying vec4 vPosition;
varying vec4 vNormal;
varying vec3 light;
varying vec3 view;
varying vec3 reflection;

void main() {
   vec4 normal4D, position4D;
   vec3 diffuse, specular, ambient, camDiff;
   float NL, VR, distAtten, camDist;

   vNormal = uModelMatrix * normalize(vec4(aNormal, 0));
   vPosition = uModelMatrix * vec4(aPosition, 1);

   if (uShadeType == 1)
      vColor = vec3(vNormal);
   else {
      light = normalize(uFlashlightPosition - vec3(vPosition));
      view = normalize(uCamPos - vec3(vPosition));
      reflection = -light + 2.0 * dot(light, vec3(vNormal)) * vec3(vNormal);
   }

   gl_Position = uProjMatrix * uViewMatrix * vPosition;
}

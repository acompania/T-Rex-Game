
struct Material {
   vec3 aColor;
   vec3 dColor;
   vec3 sColor;
   float shine;
};

attribute vec2 aTexCoord;
attribute vec3 aPosition;
attribute vec3 aNormal;

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uCamPos;
uniform vec3 uSun;

uniform int uShadeType;

varying vec3 vColor;
varying vec4 vNormal;

varying vec3 view;
varying vec3 reflection;

varying vec2 vTexCoord;

void main() {
   vec4 position;
   vec3 diffuse, specular, ambient, camDiff;
   float NL, VR, distAtten, camDist;

   vNormal = uModelMatrix * normalize(vec4(aNormal, 0));
   position = uModelMatrix * vec4(aPosition, 1);

   if (uShadeType == 1)
      vColor = vec3(vNormal);
   else {
      view = normalize(uCamPos - vec3(position));
      reflection = -uSun + 2.0 * dot(uSun, vec3(vNormal)) * vec3(vNormal);
      reflection = -reflection;
   }

   vTexCoord = aTexCoord;
   gl_Position = uProjMatrix * uViewMatrix * position;
}

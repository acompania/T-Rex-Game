
struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

uniform vec3 uLColor;
uniform int uShadeType;
uniform Material uMat;
uniform vec3 uFlashlightPosition;
uniform vec3 uFlashlightDirection;

varying vec3 vColor;
varying vec4 vPosition;
varying vec4 vNormal;
varying vec3 light;
varying vec3 view;
varying vec3 reflection;

void main() {
   vec3 diffuse, specular, ambient, rColor, norm, IL;
   float NL, VR, intensity;

   norm = normalize(vec3(vNormal));

   if (uShadeType == 1)
      gl_FragColor = vec4(vColor, 1.0);
   else {
      diffuse = uLColor * dot(vec3(norm), normalize(light)) * uMat.dColor;
      specular = uLColor * pow(max(dot(view, reflection)/(length(view)*length(reflection)), 0.0), uMat.shine) * uMat.sColor;
      ambient = uLColor * uMat.aColor;

      IL = normalize(vec3(vPosition) - uFlashlightPosition);
      intensity = pow(dot(IL, uFlashlightDirection), 3.0);
      if (intensity < cos(1.35))
         rColor = vec3(0);
      else
         rColor = intensity*(diffuse + specular + ambient);

      gl_FragColor = vec4(rColor, 1.0);
   }
}

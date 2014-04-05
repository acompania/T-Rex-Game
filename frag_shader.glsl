
struct Material {
   vec3 aColor;
   vec3 dColor;
   vec3 sColor;
   float shine;
};

uniform vec3 uLColor;
uniform int uShadeType;
uniform Material uMat;
uniform vec3 uSun;
uniform sampler2D uTexUnit;

varying vec2 vTexCoord;
varying vec3 vColor;
varying vec4 vNormal;
varying vec3 view;
varying vec3 reflection;

void main() {
   vec3 diffuse, specular, ambient, rColor, norm, IL;
   float NL, VR, intensity;

   vec4 texColor0 = vec4(vColor.x, vColor.y, vColor.z, 1);
   vec4 texColor1 = texture2D(uTexUnit, vTexCoord);

   gl_FragColor = vec4(vTexCoord.s, vTexCoord.t, 0, 1);

   norm = normalize(vec3(vNormal));

   if (uShadeType == 1)
      gl_FragColor = vec4(vColor, 1.0);
   else {
      diffuse = uLColor * dot(vec3(norm), normalize(-uSun)) * uMat.dColor;
      specular = uLColor * pow(max(dot(view, reflection)/(length(view)*length(reflection)), 0.0), uMat.shine) * uMat.sColor;
      ambient = uLColor * uMat.aColor;

      rColor = diffuse + specular + ambient;

      gl_FragColor = 0.5 * (vec4(rColor, 1.0) + vec4(texColor1[0], texColor1[1], texColor1[2], 1));
   }

   //gl_FragColor = vec4(texColor1[0], texColor1[1], texColor1[2], 1);
}

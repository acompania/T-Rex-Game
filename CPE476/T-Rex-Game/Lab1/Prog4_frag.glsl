struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

varying vec3 vColor;
varying vec3 vPos;
varying vec3 vNorm;
varying vec2 vTexCoord;

uniform int uMode;
uniform vec3 uLightPos;
uniform vec3 uLColor;
uniform vec3 uCamPos;
uniform Material uMat;
uniform sampler2D uTexUnit;

void main() {
  vec3 diffuse, specular, ambient;
  vec3 norm = normalize(vNorm);
  vec3 light = normalize(uLightPos - vPos);
  vec3 rVec = reflect(-light, norm);
  vec3 view = normalize(uCamPos - vPos);
  vec4 texColor = texture2D(uTexUnit, vTexCoord);
  if (uMode == 1) {
     diffuse = uLColor*max(dot(vNorm,light),0.0)*uMat.dColor;
     specular = uLColor*pow(max(dot(rVec,view),0.0),uMat.shine)*uMat.sColor;
     ambient = uLColor*uMat.aColor;
  }
  else {
     diffuse = uLColor*max(dot(vNorm,light),0.0)*vNorm;
     specular = uLColor*pow(max(dot(rVec,view),0.0),uMat.shine)*vNorm;
     ambient = uLColor*vNorm;
  }
  vec3 phong = (diffuse + specular + ambient);
  gl_FragColor = vec4(phong.x, phong.y, phong.z, 1.0) * vec4(texColor.xyz, 1.0);
}

layout (std140) uniform Matrices {
 
    mat4 projMatrix;
    mat4 viewModelMatrix;
};
 
attribute vec3 position;
attribute vec2 texCoord;
 
varying vec4 vertexPos;
varying vec2 TexCoord;
 
void main()
{
    TexCoord = vec2(texCoord);
    gl_Position = projMatrix * viewModelMatrix* vec4(position,1.0);
}
uniform sampler2D texUnit;
 
varying vec2 TexCoord;
 
void main()
{
	texture2D(texUnit, TexCoord)
}
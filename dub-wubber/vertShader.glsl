attribute vec2 aPosition;

void main() {
  gl_Position = vec4(aPosition.x, aPosition.y, 0, 1);
}

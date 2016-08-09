//
// 背景一枚絵
// 

$version$

in vec4 ciPosition;
in vec2 ciTexCoord0;

out vec2 TexCoord0;


void main(void) {
  gl_Position	= ciPosition;
  TexCoord0   = ciTexCoord0;
}

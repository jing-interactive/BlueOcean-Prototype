//
// 水面シェーダー
//
$version$
$precision$

uniform sampler2D	uTex0;
uniform sampler2D	uTex1;
uniform ivec2 ciWindowSize;
uniform vec2 offset;
uniform float wave;
                                                          
in vec2 TexCoord0;

out vec4 oColor;


void main(void) {
  vec4 n = texture(uTex1, TexCoord0 + offset) - vec4(0.5, 0.5, 0, 0);
  
  oColor = texture(uTex0, gl_FragCoord.xy / ciWindowSize + n.xy * wave);
}

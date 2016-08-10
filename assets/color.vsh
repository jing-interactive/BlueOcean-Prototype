//
// カラー + 光源
//
$version$

uniform mat4 ciModelViewProjection;
uniform mat3 ciNormalMatrix;

uniform vec4 LightPosition;
uniform vec4 LightAmbient;
uniform vec4 LightDiffuse;

in vec4 ciPosition;
in vec3 ciNormal;
in vec4 ciColor;

out vec4 Color;


void main(void) {
  vec4 position = ciModelViewProjection * ciPosition;

  // 簡単なライティングの計算
  vec3 normal = normalize(ciNormalMatrix * ciNormal);
  vec3 light  = normalize((LightPosition * position.w - position * LightPosition.w).xyz);

  float diffuse = max(dot(light, normal), 0.0);

  gl_Position = position;
  Color = (LightAmbient + LightDiffuse * diffuse) * ciColor;
  // Color       = clamp(LightAmbient + LightDiffuse * diffuse,
  //                     vec4(0, 0, 0, 0), vec4(1, 1, 1, 1)) * ciColor;
}

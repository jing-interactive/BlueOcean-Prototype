//
// カラー + 光源
//
$version$

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelView;
uniform mat3 ciNormalMatrix;

uniform vec4 LightPosition;
uniform vec4 LightAmbient;
uniform vec4 LightDiffuse;

in vec4 ciPosition;
in vec3 ciNormal;
in vec4 ciColor;

out vec4 Color;


void main(void) {
  vec4 position = ciModelView * ciPosition;

  // 簡単なライティングの計算
  vec3 normal = normalize(ciNormalMatrix * ciNormal);
  vec3 light  = normalize((LightPosition * position.w - position * LightPosition.w).xyz);

  float diffuse = max(dot(light, normal), 0.0);

  gl_Position = ciModelViewProjection * ciPosition;
  Color = (LightAmbient + LightDiffuse * diffuse) * ciColor;
}

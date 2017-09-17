#pragma once

//
// 細々した実装
//

#include <cinder/Ray.h>
#include <cinder/TriMesh.h>
#include <cinder/gl/GlslProg.h>
#include "Light.hpp"
#include "Relic.hpp"
#include "shader.hpp"


namespace ngs {

// 比較関数(a < b を計算する)
// SOURCE:http://tankuma.exblog.jp/11670448/
template <typename T>
struct LessVec {
  bool operator()(const T& lhs, const T& rhs) const {
    for (size_t i = 0; i < lhs.size(); ++i) {
      if (lhs[i] < rhs[i]) return true;
      if (lhs[i] > rhs[i]) return false;
    }

    return false;
  }
};


std::pair<bool, float> intersect(const ci::Ray& ray, const ci::TriMesh& mesh) {
  const auto& vertex = mesh.getPositions<3>();
  const auto& indicies = mesh.getIndices();

  bool  cross       = false;
  float cross_min_z = std::numeric_limits<float>::max();
    
  for (size_t i = 0; i < indicies.size(); i += 3) {
    float cross_z;
    if (ray.calcTriangleIntersection(vertex[indicies[i]], vertex[indicies[i + 1]], vertex[indicies[i + 2]], &cross_z)) {
      cross = true;
      cross_min_z = std::min(cross_z, cross_min_z);
    }
  }

  return std::make_pair(cross, cross_min_z);
}

std::tuple<bool, float, ci::vec3> intersect(const ci::Ray& ray, const std::vector<Relic>& relics, const float sea_level) {
  bool  cross       = false;
  float cross_min_z = std::numeric_limits<float>::max();
  ci::vec3 cross_pos;

  for (const auto& relic : relics) {
    ci::vec3 p(relic.position);
    // 遺物のマーカーは海上に浮いている
    p.y = std::max(p.y, sea_level);
      
    ci::AxisAlignedBox aabb(p, p + ci::vec3(1, 1, 1));

    float min_z, max_z;
    if (aabb.intersect(ray, &min_z, &max_z) > 0) {
      cross = true;
      if (min_z < cross_min_z) {
        cross_min_z = min_z;
        cross_pos   = p;
      }
    }
  }
    
  return std::make_tuple(cross, cross_min_z, cross_pos);
}


Light createLight(const ci::JsonTree& params) {
  Light light;

  light.direction = Json::getVec<ci::vec4>(params["direction"]);
  light.ambient   = Json::getColorA<float>(params["ambient"]);
  light.diffuse   = Json::getColorA<float>(params["diffuse"]);
  light.specular  = Json::getColorA<float>(params["specular"]);

  return light;
}

ci::gl::GlslProgRef createShader(const std::string& vtx_shader, const std::string& frag_shader) {
  auto shader = readShader(vtx_shader, frag_shader);
  return ci::gl::GlslProg::create(shader.first, shader.second);
}


float getVerticalFov(const float aspect, const float fov, const float near_z) {
  if (aspect < 1.0) {
    // 画面が縦長になったら、幅基準でfovを求める
    // fovとnear_zから投影面の幅の半分を求める
    float half_w = std::tan(ci::toRadians(fov / 2)) * near_z;

    // 表示画面の縦横比から、投影面の高さの半分を求める
    float half_h = half_w / aspect;

    // 投影面の高さの半分とnear_zから、fovが求まる
    return ci::toDegrees(std::atan(half_h / near_z) * 2);
  }
  else {
    // 横長の場合、fovは固定
    return fov;
  }
}


std::vector<Touch> createTouchInfo(const std::vector<ci::app::TouchEvent::Touch>& touches) {
  std::vector<Touch> app_touches;
  for (const auto& t : touches) {
    app_touches.push_back({ t.getId(),
                            t.getPos(),
                            t.getPrevPos(),
                            false
                          });
  }

  return app_touches;
}

}

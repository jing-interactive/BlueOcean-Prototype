#pragma once

//
// 細々した実装
//

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
    
  for (u_int i = 0; i < indicies.size(); i += 3) {
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

}

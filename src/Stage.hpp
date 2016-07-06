#pragma once

//
// 広大なステージ
//  高さ情報を持つだけでは表現力に限界がある
//  効率的に複雑な地形を表現するには..
//

#include <vector>
#include <cinder/Rand.h>
#include <cinder/Color.h>


namespace ngs {

// 地形１ブロック
struct Terrain {
  ci::vec3  pos;
  ci::Color color;
};


class Stage {
  std::vector<Terrain> terrains_;


public:
  // FIXME:奥行きはdeepなのか??
  Stage(int width, int deep) {

    // 適当に高さ情報を生成
    for (int z = 0; z < deep; ++z) {
      for (int x = 0; x < width; ++x) {
        Terrain t;
        int y = ci::randFloat(0, 3);
        t.pos = ci::vec3(x, y, z);

        // 色は高さで決める
        t.color = ci::hsvToRgb(ci::vec3(1.0f - y / 3.0f, 1.0f, 1.0f));

        terrains_.push_back(t);
      }
    }
  }

  
  const std::vector<Terrain>& terrains() const {
    return terrains_;
  }

};

}

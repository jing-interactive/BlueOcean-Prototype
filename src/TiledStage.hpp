#pragma once

//
// タイル状に並んだステージ
//

#include <map>
#include <cinder/Perlin.h>
#include "Stage.hpp"


namespace ngs {

// 比較関数(a < b を計算する)
// SOURCE:http://tankuma.exblog.jp/11670448/
struct LessVec {
  bool operator()(const ci::ivec2& lhs, const ci::ivec2& rhs) const {
    if (lhs.x < rhs.x) return true;
    if (lhs.x > rhs.x) return false;
    
    if (lhs.y < rhs.y) return true;
    // if (lhs.y > rhs.y) return false;

    return false;
  }
};


class TiledStage {
  int block_size_;
  
  ci::Perlin random_;
  float random_scale_;
  float height_scale_;
  
  std::map<ci::ivec2, Stage, LessVec> stages;
  

public:
  TiledStage(const int block_size, const ci::Perlin& random,
             const float random_scale, const float height_scale)
    : block_size_(block_size),
      random_(random),
      random_scale_(random_scale),
      height_scale_(height_scale)
  {}


  bool hasStage(const ci::ivec2& pos) const {
    return stages.count(pos);
  }
  
  const Stage& getStage(const ci::ivec2& pos) {
    if (!hasStage(pos)) {
      stages.insert(std::make_pair(pos,
                                   Stage(block_size_, block_size_,
                                         pos.x, pos.y,
                                         random_,
                                         random_scale_, height_scale_)));
    }
    
    return stages.at(pos);
  }
  
};

}

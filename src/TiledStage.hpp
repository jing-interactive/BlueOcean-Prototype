#pragma once

//
// タイル状に並んだステージ
//

#include <map>
#include <cinder/Perlin.h>
#include "StageObjFactory.hpp"
#include "Stage.hpp"
#include "Misc.hpp"


namespace ngs {

class TiledStage {
  int block_size_;
  
  ci::Perlin random_;
  float random_scale_;
  float height_scale_;

  StageObjFactory stageobj_factory_;
  
  std::map<ci::ivec2, Stage, LessVec<ci::ivec2>> stages;
  

public:
  TiledStage(const ci::JsonTree& params,
             const int block_size, const ci::Perlin& random,
             const float random_scale, const float height_scale)
    : block_size_(block_size),
      random_(random),
      random_scale_(random_scale),
      height_scale_(height_scale),
      stageobj_factory_(params)
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
                                         stageobj_factory_,
                                         random_scale_, height_scale_)));
    }
    
    return stages.at(pos);
  }
  
};

}

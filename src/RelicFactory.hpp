#pragma once

//
// 遺物生成工場
//

#include <cinder/Rand.h>
#include "JsonUtil.hpp"


namespace ngs {

class RelicFactory {
  float probability_;
  

public:
  RelicFactory(const ci::JsonTree& params)
    : probability_(params.getValueForKey<float>("probability"))
  {}

  bool create(const int height) const {
    float probability = ci::randFloat();
    return probability < probability_;
  }

};

}

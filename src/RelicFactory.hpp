#pragma once

//
// 遺物生成工場
//

#include <cinder/Rand.h>
#include "JsonUtil.hpp"
#include "Relic.hpp"


namespace ngs {

class RelicFactory {
  float probability_;

  double search_required_time_;
  

public:
  RelicFactory(const ci::JsonTree& params)
    : probability_(params.getValueForKey<float>("probability")),
      search_required_time_(params.getValueForKey<double>("search_required_time"))
  {}

  
  std::pair<bool, Relic> create(const ci::ivec3& pos) const {
    float probability = ci::randFloat();
    if (probability > probability_) {
      return std::make_pair(false, Relic());
    }

    // FIXME:いい感じにレア度を計算
    float rare = ci::randFloat();
    double search_required_time = search_required_time_ * (rare * 2 + 1.0);
    
    Relic relic = {
      pos,
      "",
      false,
      false,
      search_required_time,
      0.0,
      rare
    };
    
    return std::make_pair(true, relic);
  }

};

}

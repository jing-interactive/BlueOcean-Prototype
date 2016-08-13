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

  // 高さごとの生成確率
  std::map<int, float> height_probability_;

  double search_required_time_;
  

public:
  RelicFactory(const ci::JsonTree& params)
    : probability_(params.getValueForKey<float>("probability")),
      search_required_time_(params.getValueForKey<double>("search_required_time"))
  {
    // 高さごとの生成確率
    for (const auto& p : params["height_probability"]) {
      ci::ivec2 v = Json::getVec<ci::ivec2>(p["height"]);
      float probability = p.getValueForKey<float>("probability");
      assert(v.x <= v.y);
      for (int i = v.x; i <= v.y; ++i) {
        height_probability_.insert(std::make_pair(i, probability));
      }
    }
  }

  
  std::pair<bool, Relic> create(const ci::ivec3& pos) const {
    float probability = ci::randFloat();

    float p = height_probability_.count(pos.y) ? height_probability_.at(pos.y)
                                               : probability_;
    
    if (probability > p) {
      return std::make_pair(false, Relic());
    }

    // FIXME:いい感じにレア度を計算
    float rare = ci::randFloat();
    double search_required_time = search_required_time_ * (rare * 1.5 + 1.0);
    
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

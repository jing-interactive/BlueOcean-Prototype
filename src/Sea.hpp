#pragma once

//
// 潮の状態
//

namespace ngs {

class Sea {
  // 潮の満ち引きの速度
  ci::vec2 tide_speed_;
  ci::vec2 tide_level_;

  
public:
  Sea(const ci::JsonTree& params)
    : tide_speed_(Json::getVec<ci::vec2>(params["tide_speed"])),
      tide_level_(Json::getVec<ci::vec2>(params["tide_level"]))
  {}


  float getLevel(const double duration) const {
    float t = (std::sin(duration * tide_speed_.x) + std::sin(duration * tide_speed_.y)) * 0.25 + 0.5;
    return glm::mix(tide_level_.x, tide_level_.y, t);
  }


  // デバッグ用途
  ci::vec2& tideSpeed() {
    return tide_speed_;
  }

  ci::vec2& tideLevel() {
    return tide_level_;
  }
  
};

}

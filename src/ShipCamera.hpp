#pragma once

//
// 船を中心としたカメラ
//

namespace ngs {

class ShipCamera {
  Event<Arguments>& event_;

  // 到着時と移動時の距離
  ci::vec2 distance_;
  
  ci::vec3 position_;
  float current_distance_;

  
public:
  ShipCamera(Event<Arguments>& event, ci::JsonTree& params)
    : event_(event),
      distance_(Json::getVec<ci::vec2>(params["ship_camera.distance"])),
      current_distance_(distance_.x)
  {}

  void start() {
    current_distance_ = distance_.y;
  }

  void update(const ci::vec3& target_position) {
    position_ = target_position;
  }

  void arrived() {
    current_distance_ = distance_.x;
  }


  const ci::vec3& getPosition() const {
    return position_;
  }

  
  float getDistance() const {
    return current_distance_;
  }

};

}

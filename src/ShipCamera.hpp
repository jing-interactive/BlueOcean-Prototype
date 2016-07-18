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
  ci::quat rotation_;
  float current_distance_;
  float target_distance_;

  
public:
  ShipCamera(Event<Arguments>& event, ci::JsonTree& params)
    : event_(event),
      distance_(Json::getVec<ci::vec2>(params["ship_camera.distance"])),
      current_distance_(distance_.x),
      target_distance_(current_distance_)
  {
  }

  void start() {
    target_distance_ = distance_.y;
  }

  void update(const ci::vec3& target_position) {
    // ゆっくり値を更新する
    current_distance_ += (target_distance_ - current_distance_) * 0.1f;
    position_ += (target_position - position_) * 0.1f;
  }

  void arrived() {
    target_distance_ = distance_.x;
  }


  const ci::vec3& getPosition() const {
    return position_;
  }

  const ci::quat& getRotation() const {
    return rotation_;
  }
  
  float getDistance() const {
    return current_distance_;
  }

};

}

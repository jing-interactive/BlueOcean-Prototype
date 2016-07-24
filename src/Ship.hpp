#pragma once

//
// 大海原を旅する船
//

#include <cinder/ObjLoader.h>
#include "Arguments.hpp"
#include "Event.hpp"
#include "JsonUtil.hpp"
#include "Time.hpp"


namespace ngs {

class Ship {
  Event<Arguments>& event_;
  
  ci::vec3 position_;
  ci::quat rotation_;
  ci::vec3 scaling_;

  ci::Color color_;
  ci::gl::BatchRef model_;

  std::vector<ci::ivec3> route_;

  float speed_;
  size_t route_index_;

  bool do_route_;
  // 移動開始時間
  Time route_start_time_;


  // 現在時間の移動位置を求める
  void calcCurrentPosition(const Time& current_time, const float sea_level) {
    auto duration = current_time.getDuration() - route_start_time_.getDuration();
    
    double total_time = (route_.size() - 1) / speed_;
    if (duration.count() >= total_time) {
      // 終点に着いた
      position_ = route_.back();
      do_route_ = false;

      event_.signal("ship_arrival", Arguments());
    }
    else {
      int index = duration.count() / speed_;
      double t = (duration.count() - speed_ * index) / speed_;
      
      // ２点間の線形補間を利用した移動
      ci::vec3 start = route_[index];
      ci::vec3 end   = route_[index + 1];
      position_ = glm::mix(start, end, t);
      
      // 移動量から向きを決定
      auto d = glm::normalize(end - start);
      rotation_ = glm::rotation(ci::vec3(0, 0, -1), d);
    }

    position_.y = sea_level;
  }
  
  
public:
  Ship() = delete;
  
  Ship(Event<Arguments>& event, ci::JsonTree& params)
    : event_(event),
      position_(Json::getVec<ci::vec3>(params["ship.position"])),
      scaling_(Json::getVec<ci::vec3>(params["ship.scaling"])),
      color_(Json::getColor<float>(params["ship.color"])),
      speed_(params.getValueForKey<float>("ship.speed")),
      do_route_(false)
  {
    ci::ObjLoader loader(Asset::load("ship.obj"));

    auto shader = ci::gl::getStockShader(ci::gl::ShaderDef().color().lambert());
    model_ = ci::gl::Batch::create(loader, shader);
  }


  const ci::vec3& getPosition() const {
    return position_;
  }

  void setPosition(const ci::vec3& position) {
    position_ = position;
  }

  const ci::quat& getRotation() const {
    return rotation_;
  }

  void setRotation(const ci::quat& rotation) {
    rotation_ = rotation;
  }
  
  
  void setRoute(std::vector<ci::ivec3> route) {
    route_ = std::move(route);
  }

  const std::vector<ci::ivec3>& getRoute() const {
    return route_;
  }


  // 経路による移動開始
  void start(const Time& start_time) {
    do_route_ = true;
    route_start_time_ = start_time;
  }

  
  void update(const Time& current_time, const float sea_level) {
    position_.y = sea_level;

    if (do_route_) {
      calcCurrentPosition(current_time, sea_level);
    }
  }

  void draw() {
    // TIPS:マス目の中央に位置するようオフセットを加えている
    ci::mat4 transform = glm::translate(position_ + ci::vec3(0.5, 0, 0.5))
                       * glm::mat4_cast(rotation_)
                       * glm::scale(scaling_);

    ci::gl::setModelMatrix(transform);
    
    ci::gl::color(color_);
    model_->draw();
  }

};

}

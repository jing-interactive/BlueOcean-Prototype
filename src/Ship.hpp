#pragma once

//
// 大海原を旅する船
//

#include <cinder/ObjLoader.h>
#include "Arguments.hpp"
#include "Event.hpp"
#include "JsonUtil.hpp"


namespace ngs {

class Ship {
  Event<Arguments>& event_;
  
  ci::vec3 position_;
  ci::quat rotation_;
  ci::vec3 scaling_;

  ci::Color color_;
  ci::gl::BatchRef model_;

  std::vector<ci::ivec3> route_;

  ci::vec3 start_route_;
  ci::vec3 target_route_;
  float t_value_;
  float speed_;
  size_t route_index_;

  bool do_route_;
  
  
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

  
  void setRoute(std::vector<ci::ivec3> route) {
    route_ = std::move(route);
  }

  const std::vector<ci::ivec3>& getRoute() const {
    return route_;
  }


  // 経路による移動開始
  void start() {
    t_value_     = 0.0;
    route_index_ = 0;

    start_route_ = position_;
    const auto& pos = route_[route_index_];
    target_route_.x = pos.x;
    target_route_.y = position_.y;
    target_route_.z = pos.z;

    do_route_ = true;
  }
  
  
  void update(const float sea_level) {
    position_.y = sea_level;

    if (do_route_) {
      t_value_ += speed_;

      // ２点間の線形補間を利用した移動
      auto pos = glm::mix(start_route_, target_route_, std::min(t_value_, 1.0f));
      // 移動量から向きを決定
      auto d = glm::normalize(pos - position_);
      if (glm::length(d) > 0.0f) {
        rotation_ = glm::rotation(ci::vec3(0, 0, -1), d);
      }
      
      position_ = pos;
      
      if (t_value_ >= 1.0f) {
        t_value_ = 0.0f;
        route_index_ += 1;
        if (route_index_ == route_.size()) {
          // ゴールに到着
          do_route_ = false;

          // イベント送信
          event_.signal("ship_arrival", Arguments());
        }
        else {
          // 次の地点へ
          start_route_ = position_;
          const auto& pos = route_[route_index_];
          target_route_.x = pos.x;
          target_route_.y = position_.y;
          target_route_.z = pos.z;
        }
      }
    }
  }

  void draw() {
    ci::gl::pushModelView();

    // TIPS:マス目の中央に位置するようオフセットを加えている
    ci::gl::translate(position_ + ci::vec3(0.5, 0, 0.5));
    ci::gl::rotate(rotation_);
    ci::gl::scale(scaling_);

    ci::gl::color(color_);
    model_->draw();
    
    ci::gl::popModelView();
  }

};

}

#pragma once

//
// 大海原を旅する船
//

#include <cinder/ObjLoader.h>
#include "Arguments.hpp"
#include "Event.hpp"
#include "JsonUtil.hpp"
#include "Light.hpp"
#include "Waypoint.hpp"


namespace ngs {

class Ship {
  Event<Arguments>& event_;
  
  ci::vec3 position_;
  ci::quat rotation_;
  ci::vec3 scaling_;

  ci::Color color_;
  ci::gl::BatchRef model_;
  ci::gl::GlslProgRef shader_;
  
  std::vector<Waypoint> route_;

  float height_;
  
  float speed_;

  bool do_route_;


  // 現在時間の移動位置を求める
  void calcCurrentPosition(const double duration) {
    if (duration <= route_[0].duration) {
      // まだ移動開始時刻ではない
      position_ = route_[0].pos;
      height_   = position_.y;
    }
    else if (duration >= route_.back().duration) {
      // 終点に着いた
      position_ = route_.back().pos;
      height_   = position_.y;
      do_route_ = false;

      event_.signal("ship_arrival", Arguments());
    }
    else {
      double required_time = getRequiredTime();

      size_t index;
      for (index = 0; index < (route_.size() - 1); ++index) {
        if (route_[index].duration < duration
            && route_[index + 1].duration >= duration) {
          break;
        }
      }

      float t = 1.0 - std::min(route_[index + 1].duration - duration, required_time) / required_time;

      // ２点間の線形補間を利用した移動
      ci::vec3 start = route_[index].pos;
      ci::vec3 end   = route_[index + 1].pos;
      position_ = glm::mix(start, end, t);
      height_   = position_.y;

      // 移動量から向きを決定
      // TIPS:Y軸回転のみ
      start.y = 0;
      end.y   = 0;
      auto d = glm::normalize(end - start);
      rotation_ = glm::rotation(ci::vec3(0, 0, -1), d);
    }
  }
  
  
public:
  Ship(Event<Arguments>& event, ci::JsonTree& params)
    : event_(event),
      position_(Json::getVec<ci::vec3>(params["position"])),
      scaling_(Json::getVec<ci::vec3>(params["scaling"])),
      color_(Json::getColor<float>(params["color"])),
      speed_(params.getValueForKey<float>("speed")),
      do_route_(false)
  {
    ci::ObjLoader loader(Asset::load("ship.obj"));

    shader_ = createShader("color", "color");
    model_  = ci::gl::Batch::create(loader, shader_);
  }


  const ci::vec3& getPosition() const {
    return position_;
  }

  void setPosition(const ci::vec3& position) {
    position_ = position;
  }

  void setHeight(const float height) {
    height_ = height;
  }

  float getHeight() const {
    return height_;
  }

  const ci::quat& getRotation() const {
    return rotation_;
  }

  void setRotation(const ci::quat& rotation) {
    rotation_ = rotation;
  }
  
  
  void setRoute(std::vector<Waypoint> route) {
    route_ = std::move(route);
  }

  const std::vector<Waypoint>& getRoute() const {
    return route_;
  }

  // １ブロック移動するための所要時間
  double getRequiredTime() const {
    return 1.0 / speed_;
  }


  // 経路による移動開始
  void start() {
    do_route_ = true;
  }

  
  void update(const double duration, const float sea_level) {
    if (do_route_) {
      calcCurrentPosition(duration);
    }
    position_.y = std::max(sea_level, height_);
  }

  void draw(const Light& light) {
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);
    
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

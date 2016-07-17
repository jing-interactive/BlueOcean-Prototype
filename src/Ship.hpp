#pragma once

//
// 大海原を旅する船
//

#include <cinder/ObjLoader.h>
#include "JsonUtil.hpp"


namespace ngs {

class Ship {
  ci::vec3 position_;
  ci::quat rotation_;
  ci::vec3 scaling_;

  bool has_target_;
  
  ci::vec3 target_position_;

  ci::Color color_;
  ci::gl::BatchRef model_;

  std::vector<ci::ivec3> route_;
  
  
public:
  Ship(ci::JsonTree& params)
    : position_(Json::getVec<ci::vec3>(params["ship.position"])),
      scaling_(Json::getVec<ci::vec3>(params["ship.scaling"])),
      has_target_(false),
      color_(Json::getColor<float>(params["ship.color"]))
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
    
  }
  
  
  void update(const float sea_level) {
    position_.y = sea_level;
  }

  void draw() {
    ci::gl::pushModelView();

    ci::gl::translate(position_ + ci::vec3(0.5, 0, 0.5));
    ci::gl::rotate(rotation_);
    ci::gl::scale(scaling_);

    ci::gl::color(0.8, 0.0, 0.0);
    model_->draw();
    
    ci::gl::popModelView();
  }

};

}

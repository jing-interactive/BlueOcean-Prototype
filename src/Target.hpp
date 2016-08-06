#pragma once

//
// 探索先カーソル表示
//

namespace ngs {

class Target {
  ci::vec3 position_;
  ci::vec3 scaling_;

  float target_y_;
  
  ci::Color color_;
  ci::gl::BatchRef model_;
  ci::gl::GlslProgRef shader_;

  bool active_;
  


public:
  Target(ci::JsonTree& params)
    : scaling_(Json::getVec<ci::vec3>(params["scaling"])),
      color_(Json::getColor<float>(params["color"])),
      active_(false)
  {
    ci::ObjLoader loader(Asset::load("target.obj"));

    shader_ = createShader("color", "color");
    model_  = ci::gl::Batch::create(loader, shader_);
  }


  void setPosition(const ci::ivec3& position) {
    // TIPS:マス目の中央に位置するようオフセットを加えている
    position_ = ci::vec3(position) + ci::vec3(0.5, 0, 0.5);
    target_y_ = position.y;
    active_   = true;
  }

  void arrived() {
    active_ = false;
  }


  void update(const double duration, const float sea_level) {
    if (!active_) return;

    position_.y = std::max(target_y_, sea_level);
  }

  void setupLight(const Light& light) {
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);
  }

  void draw(const Light& light) {
    if (!active_) return;

    setupLight(light);
    
    ci::mat4 transform = glm::translate(position_)
                       * glm::scale(scaling_);

    ci::gl::setModelMatrix(transform);
    ci::gl::color(color_);

    model_->draw();
  }
  
};

}
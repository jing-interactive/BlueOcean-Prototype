#pragma once

//
// 探索先カーソル表示
//

namespace ngs {

class Target {
  ci::vec3 position_;
  ci::vec3 scaling_;
  ci::vec3 offset_;

  float target_y_;

  ci::vec2 roll_;
  ci::vec2 pitch_;

  ci::quat swing_;
  
  ci::gl::GlslProgRef shader_;
  ci::gl::VboMeshRef model_;

  bool active_;
  


  void setupLight(const Light& light) {
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);
  }

  
public:
  Target(const ci::JsonTree& params)
    : scaling_(Json::getVec<ci::vec3>(params["scaling"])),
      offset_(Json::getVec<ci::vec3>(params["offset"])),
      roll_(Json::getVec<ci::vec2>(params["roll"])),
      pitch_(Json::getVec<ci::vec2>(params["pitch"])),
      active_(false)
  {
    shader_ = createShader("color", "color");
    model_  = ci::gl::VboMesh::create(PLY::load("target.ply"));
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

    if (position_.y == sea_level) {
      swing_ = ci::quat(ci::vec3(std::sin(std::sin(duration * pitch_.x) * M_PI) * pitch_.y,
                                 0,
                                 std::sin(std::sin(duration * roll_.x) * M_PI) * roll_.y));
    }
  }

  void draw(const Light& light) {
    if (!active_) return;

    setupLight(light);
    
    ci::mat4 transform = glm::translate(position_)
                       * glm::mat4_cast(swing_)
                       * glm::scale(scaling_)
                       * glm::translate(offset_);

    ci::gl::setModelMatrix(transform);

    ci::gl::ScopedGlslProg shader(shader_);
    ci::gl::draw(model_);
  }
  
};

}

#pragma once

//
// ステージ上の遺物の描画
//

namespace ngs {

class RelicDrawer {
  float range_;
  
  ci::Color color_;
  ci::gl::BatchRef model_;
  ci::gl::GlslProgRef shader_;

  ci::quat rotation_;

  ci::vec3 rotate_speed_;
  

public:
  RelicDrawer(const ci::JsonTree& params)
    : color_(Json::getColor<float>(params["color"])),
      rotate_speed_(Json::getVec<ci::vec3>(params["rotate_speed"]))
  {
    // 計算量を減らすため２乗した値を保存
    float range = params.getValueForKey<float>("range");
    range_ = range * range;
    
    ci::ObjLoader loader(Asset::load("relic.obj"));
    
    auto shader_prog = readShader("color", "color");
    shader_ = ci::gl::GlslProg::create(shader_prog.first, shader_prog.second);
    model_ = ci::gl::Batch::create(loader, shader_);
  }

  
  void setupLight(const Light& light) {
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);
  }

  
  void update() {
    rotation_ = rotation_ * ci::quat(rotate_speed_);
  }
  
  void draw(const std::vector<Relic>& relics, const ci::vec3& center, const float sea_level) {
    if (relics.empty()) return;

    ci::gl::color(color_);

    for (const auto& relic : relics) {
      // 船からの距離によるクリッピング
      float dx = relic.position.x - center.x;
      float dz = relic.position.z - center.z;
      if ((dx * dx + dz * dz) > range_) continue;
      
      ci::gl::pushModelMatrix();

      ci::vec3 pos(relic.position.x, std::max(float(relic.position.y), sea_level), relic.position.z);
      
      // TIPS:マス目の中央に位置するようオフセットを加えている
      ci::mat4 transform = glm::translate(pos + ci::vec3(0.5, 0.5, 0.5))
        * glm::mat4_cast(rotation_);
      ci::gl::multModelMatrix(transform);
      
      model_->draw();
      ci::gl::popModelMatrix();
    }
  }

};

}

#pragma once

//
// 経路描画
//

namespace ngs {

class RouteDrawer {
  ci::Color color_;
  
  ci::gl::GlslProgRef shader_;
  ci::gl::BatchRef model_;


public:
  RouteDrawer(const ci::JsonTree& params)
    : color_(Json::getColor<float>(params["color"]))
  {
    auto shader_prog = readShader("color", "color");
    shader_ = ci::gl::GlslProg::create(shader_prog.first, shader_prog.second);

    ci::ObjLoader loader(Asset::load("route.obj"));
    model_ = ci::gl::Batch::create(loader, shader_);
  }

             
  void setupLight(const Light& light) {
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);
  }

  void draw(const std::vector<Waypoint>& route, const float sea_level) {
    for (const auto& waypoint : route) {
      ci::vec3 pos(waypoint.pos.x, std::max(sea_level, float(waypoint.pos.y)), waypoint.pos.z);

      // TIPS:マス目の中央に位置するようオフセットを加えている
      ci::mat4 transform = glm::translate(pos + ci::vec3(0.5, 0.5, 0.5));
      ci::gl::setModelMatrix(transform);
      
      model_->draw();
    }
  }
    
};

}

#pragma once

//
// 経路描画
//

namespace ngs {

class RouteDrawer {
  ci::Color color_;
  
  ci::gl::GlslProgRef shader_;
  ci::gl::VboMeshRef model_;

  
  void setupLight(const Light& light) {
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);
  }

  
public:
  RouteDrawer(const ci::JsonTree& params)
    : color_(Json::getColor<float>(params["color"]))
  {
    shader_ = createShader("color", "color");

    ci::ObjLoader loader(Asset::load("route.obj"));
    model_ = ci::gl::VboMesh::create(loader);
  }
             

  void draw(const std::vector<Waypoint>& route, const Light& light, const float sea_level) {
    if (route.empty()) return;
    
    setupLight(light);

    ci::gl::ScopedGlslProg shader(shader_);
    ci::gl::color(color_);

    for (size_t i = 0; i < route.size() - 1; ++i) {
      const auto& waypoint = route[i];
      ci::vec3 pos(waypoint.pos.x, std::max(sea_level, float(waypoint.pos.y)), waypoint.pos.z);

      // TIPS:マス目の中央に位置するようオフセットを加えている
      ci::mat4 transform = glm::translate(pos + ci::vec3(0.5, 0.5, 0.5));
      ci::gl::setModelMatrix(transform);
      
      ci::gl::draw(model_);
    }
  }
    
};

}

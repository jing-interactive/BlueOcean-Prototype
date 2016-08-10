#pragma once

//
// 収集したアイテム
//

#include "PLY.hpp"


namespace ngs {

class Item {
  ci::gl::VboMeshRef model_;
  ci::gl::GlslProgRef shader_;

  ci::AxisAlignedBox aabb_;


public:
  Item() = default;
  
  Item(const ci::JsonTree& param)
    : shader_(createShader("color", "color"))
  {
    ci::TriMesh mesh = PLY::load(param.getValueForKey<std::string>("file"));
    aabb_  = mesh.calcBoundingBox();
    model_ = ci::gl::VboMesh::create(mesh);
  }

  
  const ci::AxisAlignedBox& getAABB() const { return aabb_; }

  
  void draw(const Light&light) {
    if (!model_) return;

    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);

    ci::gl::ScopedGlslProg shader(shader_);
    ci::gl::draw(model_);
  }
  
};

}

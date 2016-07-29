#pragma once

// 
// Stage描画
//

#include "TiledStage.hpp"
#include "Light.hpp"
#include "Misc.hpp"


namespace ngs {

class StageDrawer {
  std::map<ci::ivec2, ci::gl::BatchRef, LessVec<ci::ivec2>> meshes_;

  ci::gl::Texture2dRef texture_;
  ci::gl::GlslProgRef	shader_;

  
public:
  StageDrawer() {
    // FIXME:WindowsではMagFilterにGL_NEARESTを指定すると描画が乱れる
    texture_ = ci::gl::Texture2d::create(ci::loadImage(Asset::load("stage.png")),
                                         ci::gl::Texture2d::Format()
                                         .wrap(GL_CLAMP_TO_EDGE)
                                         .minFilter(GL_NEAREST)
                                         // .magFilter(GL_NEAREST)
                                         );

    auto shader = readShader("texture", "texture");
    shader_ = ci::gl::GlslProg::create(shader.first, shader.second);
  }

  void clear() {
    meshes_.clear();
  }


  void setupLight(const Light& light) {
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);
  }
  
  void draw(const ci::ivec2& pos, const Stage& stage) {
    if (meshes_.count(pos) == 0) {
      ci::gl::BatchRef mesh = ci::gl::Batch::create(stage.getLandMesh(), shader_);
      meshes_.insert(std::make_pair(pos, mesh));
    }

    texture_->bind();
    meshes_.at(pos)->draw();
  }
  
};

}

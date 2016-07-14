#pragma once

// 
// Stage描画
//

#include "TiledStage.hpp"


namespace ngs {

class StageDrawer {
  std::map<ci::ivec2, ci::gl::BatchRef, LessVec> meshes_;

  ci::gl::Texture2dRef land_texture_;
  ci::gl::GlslProgRef	 land_shader_;

  
public:
  StageDrawer() = default;

  void setup() {
    // FIXME:WindowsではMagFilterにGL_NEARESTを指定すると描画が乱れる
    land_texture_ = ci::gl::Texture2d::create(ci::loadImage(Asset::load("stage.png")),
                                              ci::gl::Texture2d::Format()
                                              .wrap(GL_CLAMP_TO_EDGE)
                                              .minFilter(GL_NEAREST)
                                              // .magFilter(GL_NEAREST)
                                              );
    auto lambert = ci::gl::ShaderDef().texture().lambert();
    land_shader_ = ci::gl::getStockShader(lambert);
  }

  void clear() {
    meshes_.clear();
  }
  
  void draw(const ci::ivec2& pos, const Stage& stage) {
    if (meshes_.count(pos) == 0) {
      ci::gl::BatchRef mesh = ci::gl::Batch::create(stage.getLandMesh(), land_shader_);
      meshes_.insert(std::make_pair(pos, mesh));
    }

    land_texture_->bind();
    meshes_.at(pos)->draw();
  }
  
};

}

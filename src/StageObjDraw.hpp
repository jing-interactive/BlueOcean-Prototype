#pragma once

//
// ステージ上の物体の描画
//

#include <cinder/ObjLoader.h>
#include "StageObj.hpp"
#include "StageObjMesh.hpp"
#include "Light.hpp"


namespace ngs {

class StageObjDrawer {
  StageObjMesh mesh_creater_;
  ci::gl::Texture2dRef texture_;
  ci::gl::GlslProgRef	shader_;

  std::map<ci::ivec2, ci::gl::BatchRef, LessVec<ci::ivec2>> meshes_;


public:
  StageObjDrawer() {
    // FIXME:WindowsではMagFilterにGL_NEARESTを指定すると描画が乱れる
    texture_ = ci::gl::Texture2d::create(ci::loadImage(Asset::load("stage_obj.png")),
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
    if (stage.getStageObjects().empty()) return;
    
    if (meshes_.count(pos) == 0) {
      ci::gl::BatchRef mesh = mesh_creater_.createBatch(stage.getStageObjects(), shader_);
      meshes_.insert(std::make_pair(pos, mesh));
    }

    texture_->bind();
    meshes_.at(pos)->draw();
  }
    
};

}

#pragma once

// 
// Stage描画
//

#include "TiledStage.hpp"
#include "Light.hpp"
#include "Misc.hpp"


namespace ngs {

class StageDrawer {
  std::map<ci::ivec2, ci::gl::VboMeshRef, LessVec<ci::ivec2>> meshes_;

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

    shader_ = createShader("texture", "texture");
  }

  void clear() {
    meshes_.clear();
  }


  void setupLight(const Light& light) {
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);
  }


  const ci::gl::GlslProgRef& getShader() const {
    return shader_;
  }

  
  void draw(const ci::ivec2& pos, const Stage& stage) {
    if (meshes_.count(pos) == 0) {
      auto mesh = ci::gl::VboMesh::create(stage.getLandMesh());
      meshes_.insert(std::make_pair(pos, mesh));
    }

    texture_->bind();
    ci::gl::draw(meshes_.at(pos));
  }


  // 不要な地形データを破棄する
  void garbageCollection(const ci::ivec2& center, const ci::ivec2& size) {
    std::map<ci::ivec2, ci::gl::VboMeshRef, LessVec<ci::ivec2>> meshes;
    
    for (int z = -size.y; z < size.y; ++z) {
      for (int x = -size.x; x < size.x; ++x) {
        ci::ivec2 pos(x, z);
        const auto& it = meshes_.find(pos);
        if (it == std::end(meshes_)) continue;
        
        meshes.insert(std::make_pair(it->first, it->second));
      }
    }
    
    std::swap(meshes_, meshes);
  }
  
};

}

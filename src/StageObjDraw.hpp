#pragma once

//
// ステージ上の物体の描画
//

#include <cinder/ObjLoader.h>
#include "StageObj.hpp"


namespace ngs {

class StageObjDrawer {
  ci::gl::Texture2dRef texture_;
  ci::gl::GlslProgRef	 shader_;

  Holder<ci::gl::BatchRef> batches_;


public:
  StageObjDrawer() {
    // FIXME:WindowsではMagFilterにGL_NEARESTを指定すると描画が乱れる
    texture_ = ci::gl::Texture2d::create(ci::loadImage(Asset::load("stage_obj.png")),
                                         ci::gl::Texture2d::Format()
                                         .wrap(GL_CLAMP_TO_EDGE)
                                         .minFilter(GL_NEAREST)
                                         // .magFilter(GL_NEAREST)
                                         );
    
    auto lambert = ci::gl::ShaderDef().texture().lambert();
    shader_ = ci::gl::getStockShader(lambert);
  }

  void draw(const std::vector<StageObj>& objects) {
    texture_->bind();
    
    for (const auto& obj : objects) {
      const auto& name = obj.getName();
      
      if (!batches_.hasObject(name)) {
        // FIXME:まさかのここでのファイル読み込み
        ci::ObjLoader loader(Asset::load(name));
        ci::gl::BatchRef mesh = ci::gl::Batch::create(loader, shader_);
        batches_.add(name, mesh);
      }

      ci::gl::pushModelMatrix();

      ci::gl::multModelMatrix(obj.getTransfomation());
      batches_.getForKey(name)->draw();

      ci::gl::popModelMatrix();
    }
  }
    
};

}

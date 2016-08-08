#pragma once

//
// 収集したアイテム
//

namespace ngs {

class Item {
  ci::gl::GlslProgRef shader_;
  ci::gl::Texture2dRef texture_;
  ci::gl::VboMeshRef model_;


public:
  Item() = default;
  
  Item(const ci::JsonTree& param) {
    // FIXME:WindowsではMagFilterにGL_NEARESTを指定すると描画が乱れる??
    texture_ = ci::gl::Texture2d::create(ci::loadImage(Asset::load("item.png")),
                                         ci::gl::Texture2d::Format()
                                         .wrap(GL_CLAMP_TO_EDGE)
                                         .minFilter(GL_NEAREST)
                                         // .magFilter(GL_NEAREST)
                                         );

    shader_ = createShader("texture", "texture");
    
    ci::ObjLoader loader(Asset::load(param.getValueForKey<std::string>("file")));
    model_ = ci::gl::VboMesh::create(loader);
  }

  void draw(const Light& light) {
    if (!model_) return;
    
    shader_->uniform("LightPosition", light.direction);
    shader_->uniform("LightAmbient",  light.ambient);
    shader_->uniform("LightDiffuse",  light.diffuse);

    ci::gl::ScopedGlslProg shader(shader_);
    texture_->bind();
    ci::gl::draw(model_);
  }
  
};

}

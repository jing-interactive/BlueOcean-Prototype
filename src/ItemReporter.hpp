#pragma once

//
// 見つけたアイテムを報告する画面
//

#include <cinder/Arcball.h>
#include "Item.hpp"


namespace ngs {

class ItemReporter {
  // アイテム表示用専用カメラ
  ci::CameraPersp camera_;
  float fov_;
  float near_z_;

  Light light_;

  ci::gl::GlslProgRef shader_;
  ci::gl::Texture2dRef texture_;
  ci::gl::VboMeshRef  model_;

  ci::vec3 agree_translate_;
  ci::quat agree_rotate_;
  ci::vec3 agree_scale_;

  ci::vec3 bg_translate_;
  ci::quat bg_rotate_;
  
  Item item_;

  ci::Arcball arcball_;

  ci::vec3 translate_;
  
  
  
public:
  ItemReporter(const ci::JsonTree& params)
    : fov_(params.getValueForKey<float>("camera.fov")),
      near_z_(params.getValueForKey<float>("camera.near_z")),
      light_(createLight(params["light"])),
      agree_translate_(Json::getVec<ci::vec3>(params["agree_translate"])),
      agree_rotate_(Json::getQuat(params["agree_rotate"])),
      agree_scale_(Json::getVec<ci::vec3>(params["agree_scale"])),
      bg_translate_(Json::getVec<ci::vec3>(params["bg_translate"])),
      bg_rotate_(Json::getVec<ci::vec3>(params["bg_rotate"])),
      translate_(Json::getVec<ci::vec3>(params["translate"]))
  {
    int width  = ci::app::getWindowWidth();
    int height = ci::app::getWindowHeight();

    camera_ = ci::CameraPersp(width, height,
                              fov_,
                              near_z_,
                              params.getValueForKey<float>("camera.far_z"));

    camera_.setEyePoint(Json::getVec<ci::vec3>(params["camera.position"]));
    camera_.setViewDirection(Json::getVec<ci::vec3>(params["camera.direction"]));

    // FIXME:WindowsではMagFilterにGL_NEARESTを指定すると描画が乱れる??
    texture_ = ci::gl::Texture2d::create(ci::loadImage(Asset::load("item.png")),
                                         ci::gl::Texture2d::Format()
                                         .wrap(GL_CLAMP_TO_EDGE)
                                         .minFilter(GL_NEAREST)
                                         // .magFilter(GL_NEAREST)
                                         );

    shader_ = createShader("texture", "texture");

    ci::ObjLoader loader(Asset::load("agree.obj"));
    model_ = ci::gl::VboMesh::create(loader);
    
    arcball_ = ci::Arcball(&camera_, ci::Sphere(Json::getVec<ci::vec3>(params["arcball.center"]),
                                                params.getValueForKey<float>("arcball.radius")));
  }


  void loadItem(const ci::JsonTree& params) {
    item_ = Item(params);
  }


  void mouseDown(ci::app::MouseEvent &event) {
    arcball_.mouseDown(event);
  }

  void mouseDrag(ci::app::MouseEvent &event) {
    arcball_.mouseDrag(event);
  }

  
  void resize(const float aspect) {
    camera_.setAspectRatio(aspect);
    camera_.setFov(getVerticalFov(aspect, fov_, near_z_));
  }

  
  void update() {
  }

  void draw() {
    ci::gl::ScopedMatrices matricies;
    ci::gl::ScopedGlslProg shader(shader_);
    
    ci::gl::setMatrices(camera_);
    
    shader_->uniform("LightPosition", light_.direction);
    shader_->uniform("LightAmbient",  light_.ambient);
    shader_->uniform("LightDiffuse",  light_.diffuse);

    texture_->bind();
    
    ci::gl::enableDepth(true);
    ci::gl::enable(GL_CULL_FACE);
    ci::gl::clear(GL_DEPTH_BUFFER_BIT);

    ci::gl::pushModelMatrix();
    
    ci::gl::translate(agree_translate_);
    ci::gl::rotate(agree_rotate_);
    ci::gl::scale(agree_scale_);
    
    ci::gl::draw(model_);
    ci::gl::popModelMatrix();

    ci::gl::rotate(arcball_.getQuat());
    ci::gl::translate(translate_);
    item_.draw();
  }

};

}

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
  Light light_;

  Item item_;

  ci::Arcball arcball_;

  ci::vec3 translate_;
  
  
  
public:
  ItemReporter(const ci::JsonTree& params)
    : light_(createLight(params["light"])),
      translate_(Json::getVec<ci::vec3>(params["translate"]))
  {
    int width  = ci::app::getWindowWidth();
    int height = ci::app::getWindowHeight();

    camera_ = ci::CameraPersp(width, height,
                              params.getValueForKey<float>("camera.fov"),
                              params.getValueForKey<float>("camera.near_z"),
                              params.getValueForKey<float>("camera.far_z"));

    camera_.setEyePoint(Json::getVec<ci::vec3>(params["camera.position"]));
    camera_.setViewDirection(Json::getVec<ci::vec3>(params["camera.direction"]));

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

  
  void update() {
  }

  void draw() {
    ci::gl::ScopedMatrices matricies;
    
    ci::gl::setMatrices(camera_);
    ci::gl::enableDepth(true);
    ci::gl::enable(GL_CULL_FACE);

    ci::gl::clear(GL_DEPTH_BUFFER_BIT);

    ci::gl::rotate(arcball_.getQuat());
    ci::gl::translate(translate_);
    
    item_.draw(light_);
  }

};

}

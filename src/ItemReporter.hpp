#pragma once

//
// 見つけたアイテムを報告する画面
//

#include <cinder/Arcball.h>
#include <cinder/ObjLoader.h>
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

  ci::AxisAlignedBox aabb_;
  
  ci::vec3 bg_translate_;
  ci::quat bg_rotate_;
  
  Item item_;

  ci::Arcball arcball_;
  bool draged_;

  ci::vec3 offset_;
  ci::vec3 translate_;
  
  
  
public:
  ItemReporter(const ci::JsonTree& params)
    : fov_(params.getValueForKey<float>("camera.fov")),
      near_z_(params.getValueForKey<float>("camera.near_z")),
      light_(createLight(params["light"])),
      bg_translate_(Json::getVec<ci::vec3>(params["bg_translate"])),
      bg_rotate_(Json::getVec<ci::vec3>(params["bg_rotate"])),
      draged_(false),
      offset_(Json::getVec<ci::vec3>(params["offset"])),
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

    ci::TriMesh mesh(ci::ObjLoader(Asset::load("item_reporter.obj")));

    ci::mat4 transform = glm::translate(bg_translate_);
    aabb_ = mesh.calcBoundingBox(transform);

    model_ = ci::gl::VboMesh::create(mesh);
    
    arcball_ = ci::Arcball(&camera_, ci::Sphere(Json::getVec<ci::vec3>(params["arcball.center"]),
                                                params.getValueForKey<float>("arcball.radius")));
  }


  void loadItem(const ci::JsonTree& params) {
    item_ = Item(params);
  }


  void touchesBegan(const int touching_num, const std::vector<Touch>& touches) {
    // arcball_.mouseDown(event);
    draged_ = false;
  }

  void touchesMoved(const int touching_num, const std::vector<Touch>& touches) {
    // arcball_.mouseDrag(event);
    draged_ = true;
  }

  void touchesEnded(const int touching_num, const std::vector<Touch>& touches) {
    // スクリーン座標→正規化座標
    ci::vec2 pos = touches[0].getPos();
    float sx = pos.x / ci::app::getWindowWidth();
    float sy = 1.0f - pos.y / ci::app::getWindowHeight();
    
    ci::Ray ray = camera_.generateRay(sx, sy,
                                      camera_.getAspectRatio());

    if (aabb_.intersects(ray)) {
      // 終了
      DOUT << "Finish item reporter." << std::endl;
    }
    
    draged_ = false;
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
    
    ci::gl::translate(bg_translate_);
    // ci::gl::rotate(bg_rotate_);
    
    ci::gl::draw(model_);
    ci::gl::popModelMatrix();

    ci::gl::translate(offset_);
    ci::gl::rotate(arcball_.getQuat());
    ci::gl::translate(translate_);
    
    item_.draw();
  }

};

}

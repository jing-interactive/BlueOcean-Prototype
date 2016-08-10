#pragma once

//
// 見つけたアイテムを報告する画面
//

#include <cinder/ObjLoader.h>
#include <cinder/Easing.h>
#include <cinder/Timeline.h>
#include <cinder/Tween.h>
#include "Item.hpp"


namespace ngs {

class ItemReporter {
  Event<Arguments>& event_;
  
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

  uint32_t touch_id_;
  bool draged_;
  ci::quat drag_rotate_;

  ci::vec3 offset_;
  ci::vec3 translate_;

  ci::TimelineRef timeline_;
  ci::Anim<float> tween_scale_;

  // UI有効
  bool active_;
  
  
  
public:
  ItemReporter(Event<Arguments>& event,
               const ci::JsonTree& params)
    : event_(event),
      fov_(params.getValueForKey<float>("camera.fov")),
      near_z_(params.getValueForKey<float>("camera.near_z")),
      light_(createLight(params["light"])),
      bg_translate_(Json::getVec<ci::vec3>(params["bg_translate"])),
      bg_rotate_(Json::getVec<ci::vec3>(params["bg_rotate"])),
      draged_(false),
      drag_rotate_(Json::getQuat(params["drag_rotate"])),
      offset_(Json::getVec<ci::vec3>(params["offset"])),
      translate_(Json::getVec<ci::vec3>(params["translate"])),
      timeline_(ci::Timeline::create()),
      tween_scale_(0.0f),
      active_(false)
  {
    int width  = ci::app::getWindowWidth();
    int height = ci::app::getWindowHeight();
    // FIXME:生成時に画角を考慮する必要がある
    float fov = getVerticalFov(width / float(height), fov_, near_z_);
    
    camera_ = ci::CameraPersp(width, height,
                              fov,
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

    // 少しオフセットを加えたAABBをクリック判定に使う
    auto bb = mesh.calcBoundingBox();
    aabb_ = ci::AxisAlignedBox(bb.getMin(),
                               bb.getMax() + ci::vec3(0, -31, 0)).transformed(transform);

    model_ = ci::gl::VboMesh::create(mesh);

    // 開始演出
    timeline_->apply(&tween_scale_, 0.0f, 1.0f, 0.5f, ci::EaseOutBack())
      .finishFn([this]() {
          active_ = true;
        });
  }


  void loadItem(const ci::JsonTree& params) {
    item_ = Item(params);
  }


  void touchesBegan(const int touching_num, const std::vector<Touch>& touches) {
    if (touching_num == 1) {
      // 最初のを覚えておく
      touch_id_ = touches[0].getId();
      draged_ = false;
    }
  }

  void touchesMoved(const int touching_num, const std::vector<Touch>& touches) {
    if (touches.size() > 1) return;
    if (touches[0].getId() != touch_id_) return;
    
    ci::vec2 d{ touches[0].getPos() -  touches[0].getPrevPos() };
    float l = length(d);
    if (l > 0.0f) {
      d = normalize(d);
      ci::vec3 v(d.y, d.x, 0.0f);
      ci::quat r = glm::angleAxis(l * 0.01f, v);
      drag_rotate_ = r * drag_rotate_;
        
      draged_ = true;
    }
  }

  void touchesEnded(const int touching_num, const std::vector<Touch>& touches) {
    if (!active_) return;
    
    if ((touching_num == 0) && !draged_) {
      if (touches[0].getId() != touch_id_) return;
    
      // スクリーン座標→正規化座標
      ci::vec2 pos = touches[0].getPos();
      float sx = pos.x / ci::app::getWindowWidth();
      float sy = 1.0f - pos.y / ci::app::getWindowHeight();
    
      ci::Ray ray = camera_.generateRay(sx, sy,
                                        camera_.getAspectRatio());

      if (aabb_.intersects(ray)) {
        // 終了
        DOUT << "Finish item reporter." << std::endl;

        active_ = false;
        // 終了演出
        timeline_->apply(&tween_scale_, 0.0f, 0.5f, ci::EaseInBack())
          .finishFn([this]() {
              event_.signal("close_item_reporter", Arguments());
            });
      }
    
      draged_ = false;
    }
  }

  
  void resize(const float aspect) {
    camera_.setAspectRatio(aspect);
    camera_.setFov(getVerticalFov(aspect, fov_, near_z_));
  }

  
  void update() {
    timeline_->step(1 / 60.0);
  }

  void draw() {
    auto vp = ci::gl::getViewport();

    ci::vec2 size(ci::vec2(vp.second) * tween_scale_());
    ci::vec2 offset((ci::vec2(vp.second) - size) / 2.0f);

    ci::gl::ScopedViewport viewportScope(offset, size);
    
    ci::gl::ScopedMatrices matricies;
    ci::gl::setMatrices(camera_);

    // ci::gl::pushModelMatrix();
    {
      ci::gl::ScopedGlslProg shader(shader_);
      ci::gl::ScopedTextureBind texture(texture_);
    
      shader_->uniform("LightPosition", light_.direction);
      shader_->uniform("LightAmbient",  light_.ambient);
      shader_->uniform("LightDiffuse",  light_.diffuse);

      ci::gl::enableDepth(true);
      ci::gl::enable(GL_CULL_FACE);
      ci::gl::clear(GL_DEPTH_BUFFER_BIT);

      ci::gl::pushModelMatrix();
    
      ci::gl::translate(bg_translate_);
      // ci::gl::rotate(bg_rotate_);
    
      ci::gl::draw(model_);
      ci::gl::popModelMatrix();

      ci::gl::translate(offset_);
      ci::gl::rotate(drag_rotate_);
      ci::gl::translate(translate_);
    
      item_.draw(light_);
    }
    // ci::gl::popModelMatrix();
    
    // ci::gl::enableDepth(false);
    // ci::gl::color(1, 0, 0);
    // ci::gl::drawStrokedCube(aabb_);
  }

};

}

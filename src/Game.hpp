#pragma once

//
// 試作版アプリ
//

#include <cinder/gl/gl.h>
#include <cinder/Camera.h>
#include <cinder/params/Params.h>
#include <cinder/Perlin.h>
#include <cinder/Ray.h>
#include <cinder/Frustum.h>
#include "Event.hpp"
#include "Arguments.hpp"
#include "Asset.hpp"
#include "Params.hpp"
#include "Shader.hpp"
#include "Holder.hpp"
#include "StageObj.hpp"
#include "TiledStage.hpp"
#include "StageDraw.hpp"
#include "StageObjDraw.hpp"
#include "Ship.hpp"
#include "ShipCamera.hpp"
#include "Route.hpp"
#include "Time.hpp"


namespace ngs {

class Game {
  enum {
    BLOCK_SIZE = 64,
    
    FBO_WIDTH  = 512,
    FBO_HEIGHT = 512,
  };

  Event<Arguments> event_;
  
  ci::JsonTree params_;
  
  ci::CameraPersp camera;

  float fov;
  float near_z;
  float far_z;

  ci::vec2 mouse_prev_pos;
  int touch_num;

  ci::quat rotate_;
  ci::vec3 translate_;
  float z_distance_;
  bool camera_modified_;

  // カメラの操作感
  float camera_rotation_sensitivity_;
  float camera_translation_sensitivity_;
  
  ci::Color bg_color;
  
  // Stage生成時の乱数調整用
  int octave;
  int seed;
  float ramdom_scale;
  float height_scale;
  ci::Perlin random;

  // 陸地
  TiledStage stage;

  // 海面
  float sea_level_;
  ci::ColorA sea_color_;
  ci::vec2 sea_offset_;
  ci::vec2 sea_speed_;
  float sea_wave_;

  ci::gl::Texture2dRef sea_texture_;
  ci::gl::GlslProgRef	sea_shader_;
  ci::gl::BatchRef sea_mesh_;
  
  ci::gl::FboRef fbo_;
  
  StageDrawer stage_drawer_;
  StageObjDrawer stageobj_drawer_;

  bool picked_;
  ci::AxisAlignedBox picked_aabb_;
  ci::vec3 picked_pos_;

  // 船
  Ship ship_;
  ShipCamera ship_camera_;

  // 経路
  bool has_route_;
  Time route_start_time_;

  // 時間管理
  Time start_time_;

  
  // デバッグ用
  bool disp_stage_;
  bool disp_stage_obj_;
  bool disp_sea_;




  
#if !defined (CINDER_COCOA_TOUCH)
  // iOS版はダイアログの実装が無い
  ci::params::InterfaceGlRef params;
#endif


  float getVerticalFov() {
    float aspect = ci::app::getWindowAspectRatio();
    camera.setAspectRatio(aspect);

    if (aspect < 1.0) {
      // 画面が縦長になったら、幅基準でfovを求める
      // fovとnear_zから投影面の幅の半分を求める
      float half_w = std::tan(ci::toRadians(fov / 2)) * near_z;

      // 表示画面の縦横比から、投影面の高さの半分を求める
      float half_h = half_w / aspect;

      // 投影面の高さの半分とnear_zから、fovが求まる
      return ci::toDegrees(std::atan(half_h / near_z) * 2);
    }
    else {
      // 横長の場合、fovは固定
      return fov;
    }
  }

  void createStage() {
    stage = TiledStage(params_, BLOCK_SIZE, random, ramdom_scale, height_scale);
    stage_drawer_.clear();
    stageobj_drawer_.clear();
  }

  void createSeaMesh() {
    ci::TriMesh mesh;
    ci::vec2 size(BLOCK_SIZE, BLOCK_SIZE);

    ci::vec3 p[] = {
      {      0, 0,      0 },
      { size.x, 0,      0 },
      {      0, 0, size.y },
      { size.x, 0, size.y },
    };
    ci::vec2 uv[] = {
      { 0, 0 },
      { 1, 0 },
      { 0, 1 },
      { 1, 1 }
    };
    
    mesh.appendPositions(&p[0], 4);
    mesh.appendTexCoords0(&uv[0], 4);
    mesh.appendTriangle(0, 2, 1);
    mesh.appendTriangle(1, 2, 3);

    auto shader = readShader("water", "water");
    sea_shader_ = ci::gl::GlslProg::create(shader.first, shader.second);
    sea_shader_->uniform("uTex0", 0);
    sea_shader_->uniform("uTex1", 1);
    sea_texture_ = ci::gl::Texture2d::create(ci::loadImage(Asset::load("water_normal.png")),
                                             ci::gl::Texture2d::Format().wrap(GL_REPEAT));
    sea_mesh_ = ci::gl::Batch::create(mesh, sea_shader_);
  }
  
  
  // ダイアログ関連
#if defined (CINDER_COCOA_TOUCH)
  void createDialog() {}
  void drawDialog() {}
#else
  void createDialog() {
    // 各種パラメーター設定
    params = ci::params::InterfaceGl::create("Preview params", ci::app::toPixels(ci::ivec2(250, 600)));

    params->addParam("Fov", &fov).min(1.0f).max(180.0f).updateFn([this]() {
        camera.setFov(fov);
      });

    params->addParam("Near Z", &near_z).min(0.1f).updateFn([this]() {
        camera.setNearClip(near_z);
      });
    params->addParam("Far Z", &far_z).min(0.1f).updateFn([this]() {
        camera.setFarClip(far_z);
      });

    params->addSeparator();

    params->addParam("BG", &bg_color);

    params->addSeparator();

    params->addParam("Octave", &octave).min(0).max(255)
      .updateFn([this]() {
          createStage();
        });
    params->addParam("Seed", &seed)
      .updateFn([this]() {
          createStage();
        });
    params->addParam("Random Scale", &ramdom_scale).min(0.001f).step(0.001f)
      .updateFn([this]() {
          createStage();
        });
    params->addParam("Height Scale", &height_scale).min(1.0f).step(0.1f)
      .updateFn([this]() {
          createStage();
        });

    params->addSeparator();

    params->addParam("Sea Color", &sea_color_);
    params->addParam("Sea Speed x", &sea_speed_.x).step(0.00001f);
    params->addParam("Sea Speed y", &sea_speed_.y).step(0.00001f);
    params->addParam("Sea Wave", &sea_wave_).step(0.001f);
    params->addParam("Sea Level", &sea_level_).step(0.25f);

    params->addSeparator();

    params->addParam("Disp Stage",    &disp_stage_);
    params->addParam("Disp StageObj", &disp_stage_obj_);
    params->addParam("Disp Sea",      &disp_sea_);
    
  }

  void drawDialog() {
    params->draw();
  }
#endif


  std::pair<bool, float> intersect(const ci::Ray& ray, const ci::TriMesh& mesh) {
    const auto& vertex = mesh.getPositions<3>();
    const auto& indicies = mesh.getIndices();

    bool  cross       = false;
    float cross_min_z = std::numeric_limits<float>::max();
    
    for (u_int i = 0; i < indicies.size(); i += 3) {
      float cross_z;
      if (ray.calcTriangleIntersection(vertex[indicies[i]], vertex[indicies[i + 1]], vertex[indicies[i + 2]], &cross_z)) {
        cross = true;
        cross_min_z = std::min(cross_z, cross_min_z);
      }
    }

    return std::make_pair(cross, cross_min_z);
  }


  void pickStage(const ci::vec2& pos) {
    // スクリーン座標→正規化座標
    float x = pos.x / ci::app::getWindowWidth();
    float y = 1.0f - pos.y / ci::app::getWindowHeight();
    
    ci::Ray ray = camera.generateRay(x, y,
                                     camera.getAspectRatio());

    picked_ = false;
    float cross_min_z = std::numeric_limits<float>::max();

    // 海面と交差する必要がある
    float sea_z;
    if (!ray.calcPlaneIntersection(ci::vec3(0, sea_level_, 0), ci::vec3(0, 1, 0), &sea_z)) return;
    ci::vec3 sea_pos = ray.calcPosition(sea_z);
    // TIPS:誤差があると経路検索で目的地にたどり着かないw
    sea_pos.y = sea_level_;

    // どの区画をクリックしたか判定
    float z;
    if (!ray.calcPlaneIntersection(ci::vec3(0, 0, 0), ci::vec3(0, 1, 0), &z)) return;
    ci::vec3 p = ray.calcPosition(z);

    // 中央ブロックの座標
    ci::ivec2 center_pos(p.x / BLOCK_SIZE, p.z / BLOCK_SIZE);
    for (int z = (center_pos.y - 2); z < (center_pos.y + 3); ++z) {
      for (int x = (center_pos.x - 2); x < (center_pos.x + 3); ++x) {
        // Rayを平行移動
        ci::Ray t_ray = ray;
        t_ray.setOrigin(ray.getOrigin() + ci::vec3(x * -BLOCK_SIZE, 0, z * -BLOCK_SIZE));
          
        const auto& s = stage.getStage(ci::ivec2(x, z));

        float cross_z[2];
        if (!s.getAABB().intersect(t_ray, &cross_z[0], &cross_z[1])) continue;
        if (cross_z[0] >= cross_min_z) continue;
          
        // TriMeshを調べて交差点を特定する
        auto result = intersect(t_ray, s.getLandMesh());
        if (result.first && result.second < cross_min_z) {
          picked_ = true;

          // Pick座標を保持
          cross_min_z = result.second;
          picked_pos_ = ray.calcPosition(result.second);

          // AABBも保持
          ci::mat4 m = glm::translate(ci::mat4(1.0), ci::vec3(x * BLOCK_SIZE, 0, z * BLOCK_SIZE));
          picked_aabb_ = s.getAABB().transformed(m);
        }
      }
    }

    if (!picked_) return;

    if (picked_pos_.y > sea_pos.y) {
      // 海面より高い場所はピックできない
      picked_ = false;
    }
    else {
      picked_pos_ = sea_pos;
    }
  }


  // 回転操作
  void handlingRotation(const ci::vec2& current_pos, const ci::vec2& prev_pos) {
    ci::vec2 d{ current_pos - prev_pos };
    float l = ci::length(d);
    if (l > 0.0f) {
      // d = ci::normalize(d);
      // ci::vec3 v{ -d.y, -d.x, 0.0f };
      // ci::quat r = glm::angleAxis(l * 0.01f, v);
      ci::quat rx(ci::vec3(-d.y * camera_rotation_sensitivity_, 0.0f, 0.0f));
      ci::quat ry(ci::vec3(0.0f, -d.x * camera_rotation_sensitivity_, 0.0f));
      rotate_ = ry * rotate_ * rx;
    }
  }

  // 平行移動操作
  void handlingTranslation(const ci::vec2& current_pos, const ci::vec2& prev_pos) {
    ci::vec2 d{ current_pos - prev_pos };
    ci::vec3 v{ d.x, 0.0f, d.y };

    float t = std::tan(ci::toRadians(fov) / 2.0f) * z_distance_;
    auto p = (rotate_ * v) * t * camera_translation_sensitivity_;
    translate_.x -= p.x;
    translate_.z -= p.z;
  }

  // ズーミング操作
  void handlingZooming(const float zooming) {
    float t = std::tan(ci::toRadians(fov) / 2.0f) * z_distance_;
    z_distance_ = std::max(z_distance_- zooming * t, near_z);
  }

  
  // 陸地の描画
  void drawStage(const ci::ivec2& center_pos, const ci::Frustum& frustum) {
    ci::gl::setMatrices(camera);
    ci::gl::disableAlphaBlending();
    
    for (int z = (center_pos.y - 2); z < (center_pos.y + 3); ++z) {
      for (int x = (center_pos.x - 2); x < (center_pos.x + 3); ++x) {
        ci::vec3 pos(ci::vec3(x * BLOCK_SIZE, 0, z * BLOCK_SIZE));

        // 視錐台カリング
        ci::ivec2 stage_pos(x, z);
        const auto& s = stage.getStage(stage_pos);
        const auto& b = s.getAABB();
        ci::AxisAlignedBox aabb(b.getMin() + pos, b.getMax() + pos);
        if (!frustum.intersects(aabb)) continue;

        ci::mat4 transform = glm::translate(pos);
        ci::gl::setModelMatrix(transform);

        if (disp_stage_) {
          stage_drawer_.draw(stage_pos, s);
        }
        if (disp_stage_obj_) {
          stageobj_drawer_.draw(stage_pos, s);
        }
      }
    }
  }

  // 経路表示
  void drawRoute() {
    ci::gl::setModelMatrix(ci::mat4(1.0f));
    
    ci::gl::color(1, 0, 0);
    const auto& route = ship_.getRoute();
    for (const auto& r : route) {
      ci::vec3 pos(r);
      ci::gl::drawCube(pos + ci::vec3(0.5, 0.5, 0.5), ci::vec3(0.2, 0.2, 0.2));
    }
  }


  // 各種コールバックを登録
  void registerCallbacks() {
    event_.connect("ship_arrival",
                   [this](const Connection&, const Arguments&) {
        ci::app::console() << "ship_arrival" << std::endl;
        has_route_ = false;
        ship_camera_.arrived();
      });
    
  }

  // セーブデータから色々復元
  void restoreFromRecords() {
    auto path = getDocumentPath() / "record.json";
    if (!ci::fs::is_regular_file(path)) return;
    
    ci::JsonTree record(ci::loadFile(path));

    // ゲーム開始時刻を復元
    start_time_ = Time(record.getValueForKey<double>("start_time"));

    ship_.setPosition(Json::getVec<ci::vec3>(record["ship.position"]));
    ship_.setRotation(Json::getVec<ci::quat>(record["ship.rotation"]));

    // カメラ
    rotate_     = Json::getVec<ci::quat>(record["camera.rotate"]);
    translate_  = ship_.getPosition();
    z_distance_ = record.getValueForKey<float>("camera.z_distance");
    
    // 船の経路
    has_route_ = record.getValueForKey<bool>("has_route");
    if (has_route_) {
      const auto& route = record["route"];
      std::vector<ci::ivec3> ship_route;
      for (const auto& r : route) {
        auto pos = Json::getVec<ci::ivec3>(r);
        ship_route.push_back(pos);
      }

      route_start_time_ = Time(record.getValueForKey<double>("route_start_time"));

      ship_.setRoute(ship_route);
      ship_.start(route_start_time_);

      // カメラ設定
      ship_camera_.start();
      ship_camera_.update(ship_.getPosition());
      translate_  = ship_camera_.getPosition();
      z_distance_ = ship_camera_.getDistance();
    }
  }

  
public:
  Game()
    : params_(Params::load("params.json")),
      fov(params_.getValueForKey<float>("camera.fov")),
      near_z(params_.getValueForKey<float>("camera.near_z")),
      far_z(params_.getValueForKey<float>("camera.far_z")),
      camera_rotation_sensitivity_(params_.getValueForKey<float>("app.camera_rotation_sensitivity")),
      camera_translation_sensitivity_(params_.getValueForKey<float>("app.camera_translation_sensitivity")),
      touch_num(0),
      z_distance_(params_.getValueForKey<float>("camera.z_distance")),
      camera_modified_(false),
      octave(params_.getValueForKey<float>("stage.octave")),
      seed(params_.getValueForKey<float>("stage.seed")),
      ramdom_scale(params_.getValueForKey<float>("stage.random_scale")),
      height_scale(params_.getValueForKey<float>("stage.height_scale")),
      random(octave, seed),
      stage(params_, BLOCK_SIZE, random, ramdom_scale, height_scale),
      sea_level_(params_.getValueForKey<float>("stage.sea_level")),
      sea_color_(Json::getColorA<float>(params_["stage.sea_color"])),
      sea_wave_(params_.getValueForKey<float>("stage.sea_wave")),
      picked_(false),
      ship_(event_, params_),
      ship_camera_(event_, params_),
      has_route_(false),
      disp_stage_(true),
      disp_stage_obj_(true),
      disp_sea_(true)
  {
    int width  = ci::app::getWindowWidth();
    int height = ci::app::getWindowHeight();

    camera = ci::CameraPersp(width, height,
                             fov,
                             near_z, far_z);

    camera.setEyePoint(ci::vec3());
    camera.setViewDirection(ci::vec3{ 0.0f, 0.0f, -1.0f });

    rotate_ = glm::angleAxis(ci::toRadians(45.0f), ci::vec3(0.0f, 1.0f, 0.0f))
      * glm::angleAxis(ci::toRadians(-45.0f), ci::vec3(1.0f, 0.0f, 0.0f));

    createDialog();

    bg_color = ci::Color(0, 0, 0);

    createSeaMesh();

    // FBO
    auto format = ci::gl::Fbo::Format()
      .colorTexture()
      ;
    fbo_ = ci::gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format);

    ci::gl::enableDepthRead();
    ci::gl::enableDepthWrite();
    ci::gl::enable(GL_CULL_FACE);

    registerCallbacks();

    // 記録ファイルがあるなら読み込んでみる
    restoreFromRecords();
  }

  void resize() {
    camera.setFov(getVerticalFov());
    touch_num = 0;
  }

  void resetTouch() {
    touch_num = 0;
  }
  

  void mouseDown(ci::app::MouseEvent event) {
    // マルチタッチ判定中は無視
    if (touch_num > 1) return;

    if (event.isLeft()) {
      ci::ivec2 pos = event.getPos();
      mouse_prev_pos = pos;
    }
  }

  void mouseDrag(ci::app::MouseEvent event) {
    if (touch_num > 1) return;
    if (!event.isLeftDown()) return;

    ci::vec2 mouse_pos = event.getPos();

    if (event.isShiftDown()) {
      handlingTranslation(mouse_pos, mouse_prev_pos);
    }
    else if (event.isControlDown()) {
      float d = mouse_pos.y - mouse_prev_pos.y;
      handlingZooming(d * 0.5f);
    }
    else {
      handlingRotation(mouse_pos, mouse_prev_pos);
    }
    camera_modified_ = true;
    mouse_prev_pos = mouse_pos;
  }

  void mouseWheel(ci::app::MouseEvent event) {
    // OSX:マルチタッチ操作の時に呼ばれる
    if (touch_num > 1) return;
    handlingZooming(-event.getWheelIncrement() * 2.0);
  }

  void mouseUp(ci::app::MouseEvent event) {
    if (event.isLeft()) {
      if (!camera_modified_) {
        // クリックした位置のAABBを特定
        ci::ivec2 pos = event.getPos();
        pickStage(pos);

        if (picked_) {
          // 経路探索
          ci::ivec3 start = ship_.getPosition();
          ci::ivec3 end   = glm::floor(picked_pos_);

          auto route = Route::search(start, end, stage);
          ship_.setRoute(route);
          route_start_time_ = Time();
          ship_.start(route_start_time_);
          ship_camera_.start();
          has_route_ = true;
        }
      }
      camera_modified_ = false;
    }
  }

  void touchesBegan(ci::app::TouchEvent event) {
    const auto& touches = event.getTouches();
    touch_num += touches.size();
  }

  void touchesMoved(ci::app::TouchEvent event) {
    const auto& touches = event.getTouches();

#if defined (CINDER_COCOA_TOUCH)
    if (touch_num == 1) {
      handlingRotation(touches[0].getPos(),
                       touches[0].getPrevPos());
      camera_modified_ = true;
      return;
    }
#endif
    if (touches.size() < 2) return;

    auto v1 = touches[0].getPos() - touches[1].getPos();
    auto v2 = touches[0].getPrevPos() - touches[1].getPrevPos();

    float ld = ci::length(v1) - ci::length(v2);
    
    if (std::abs(ld) < 3.0f) {
      handlingTranslation(touches[0].getPos(), touches[0].getPrevPos());
    }
    else {
      handlingZooming(ld * 0.1f);
    }
    camera_modified_ = true;
  }
  
  void touchesEnded(ci::app::TouchEvent event) {
    const auto& touches = event.getTouches();

    // 最悪マイナス値にならないよう
    touch_num = std::max(touch_num - int(touches.size()), 0);
    if (!touch_num) camera_modified_ = false;

  }

  
  void update() {
    Time current_time;
    
    ship_camera_.update(ship_.getPosition());
    
    // カメラ位置の計算
    translate_.y = sea_level_;
    if (has_route_) {
      translate_  += (ship_camera_.getPosition() - translate_) * 0.1f;
      z_distance_ += (ship_camera_.getDistance() - z_distance_) * 0.1f;
    }
    auto pos = rotate_ * ci::vec3(0, 0, z_distance_) + translate_;
    
    camera.setEyePoint(pos);
    camera.setOrientation(rotate_);

    sea_offset_ += sea_speed_;

    ship_.update(current_time, sea_level_);
  }
  
  void draw() {
    ci::gl::setMatrices(camera);
    ci::gl::disableAlphaBlending();

    // 陸地の描画
    // 画面中央の座標をレイキャストして求めている
    ci::Ray ray = camera.generateRay(0.5f, 0.5f,
                                     camera.getAspectRatio());
    
    float z;
    if (ray.calcPlaneIntersection(ci::vec3(0, 0, 0), ci::vec3(0, 1, 0), &z)) {
      ci::vec3 p = ray.calcPosition(z);

      ci::Frustum frustum(camera);

      // 中央ブロックの座標
      ci::ivec2 pos(p.x / BLOCK_SIZE, p.z / BLOCK_SIZE);
      {
        // 海面演出のためにFBOへ描画
        ci::gl::ScopedViewport viewportScope(ci::ivec2(0), fbo_->getSize());
        ci::gl::ScopedFramebuffer fboScope(fbo_);
        ci::gl::clear();

        drawStage(pos, frustum);
      }
      
      ci::gl::clear(bg_color);

      // 海面の描画
      if (disp_sea_) {
        fbo_->getColorTexture()->bind(0);
        sea_texture_->bind(1);
        sea_shader_->uniform("offset", sea_offset_);
        sea_shader_->uniform("wave", sea_wave_);
        sea_shader_->uniform("color", sea_color_);
        
        for (int z = (pos.y - 2); z < (pos.y + 3); ++z) {
          for (int x = (pos.x - 2); x < (pos.x + 3); ++x) {
            // 視錐台カリング
            ci::vec3 pos(ci::vec3(x * BLOCK_SIZE, sea_level_, z * BLOCK_SIZE));
            ci::AxisAlignedBox aabb(pos, pos + ci::vec3(BLOCK_SIZE, 0, BLOCK_SIZE));
            if (!frustum.intersects(aabb)) continue;

            ci::mat4 transform = glm::translate(pos);
            ci::gl::setModelMatrix(transform);

            sea_mesh_->draw();
          }
        }
      }
      
      drawStage(pos, frustum);
      ship_.draw();

      if (picked_) {
        ci::gl::setModelMatrix(ci::mat4(1.0f));

        ci::gl::color(1, 0, 0);
        ci::gl::drawStrokedCube(picked_aabb_);
        ci::gl::drawSphere(picked_pos_, 0.1f);
      }

      if (has_route_) {
        drawRoute();
      }
    }
    
    // ダイアログ表示
    drawDialog();
  }

  // アプリ終了時
  void cleanup() {
    DOUT << "cleanup" << std::endl;

    ci::JsonTree object;

    // 開始時間
    auto duration = start_time_.getDuration();
    object.pushBack(ci::JsonTree("start_time", duration.count()));

    // 船の経路
    object.pushBack(ci::JsonTree("has_route", has_route_));
    if (has_route_) {
      auto route = ci::JsonTree::makeArray("route");
      for (const auto& r : ship_.getRoute()) {
        ci::JsonTree value = Json::createFromVec(r);
        route.pushBack(value);
      }

      object.pushBack(route);

      object.pushBack(ci::JsonTree("route_start_time", route_start_time_.getDuration().count()));
    }

    // 船の情報
    ci::JsonTree ship_info = ci::JsonTree::makeObject("ship");
    {
      auto position = Json::createFromVec("position", ship_.getPosition());
      ship_info.pushBack(position);
    }
    {
      auto rotation = Json::createFromVec("rotation", ship_.getRotation());
      ship_info.pushBack(rotation);
    }
    object.pushBack(ship_info);

    // カメラ情報
    ci::JsonTree camera_info = ci::JsonTree::makeObject("camera");
    {
      auto rotate = Json::createFromVec("rotate", rotate_);
      camera_info.pushBack(rotate);

      camera_info.pushBack(ci::JsonTree("z_distance", z_distance_));
    }
    object.pushBack(camera_info);
    
    object.write(getDocumentPath() / "record.json");
  }
};

}

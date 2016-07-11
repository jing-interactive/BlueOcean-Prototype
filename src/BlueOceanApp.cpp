//
// BlueOcean プロトタイプ
// 

#include "Defines.hpp"
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <cinder/Camera.h>
#include <cinder/params/Params.h>
#include <cinder/Perlin.h>
#include <cinder/Ray.h>
#include "Shader.hpp"
#include "TiledStage.hpp"
#include "StageDraw.hpp"


namespace ngs {

class GameApp : public ci::app::App {
  enum {
    BLOCK_SIZE = 64,
    
    FBO_WIDTH  = 512,
    FBO_HEIGHT = 512,
  };
  
  ci::CameraPersp camera;

  float fov;
  float near_z;
  float far_z;

  ci::ivec2 mouse_prev_pos;
  int touch_num;

  ci::quat rotate;
  ci::vec3 translate;
  float z_distance;

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
  float sea_level;
  ci::ColorA sea_color;
  ci::vec2 sea_offset_;
  ci::vec2 sea_speed_;
  float sea_wave_;

  ci::gl::Texture2dRef sea_texture_;
  ci::gl::GlslProgRef	sea_shader_;
  ci::gl::BatchRef sea_mesh_;
  
  ci::gl::FboRef fbo;
  
  StageDrawer stage_drawer_;

  bool picked_;
  ci::AxisAlignedBox picked_aabb_;
  ci::vec3 picked_pos_;
  
  
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
    stage = TiledStage(BLOCK_SIZE, random, ramdom_scale, height_scale);
    stage_drawer_.clear();
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
    sea_texture_ = ci::gl::Texture2d::create(ci::loadImage(ci::app::loadAsset("water_normal.png")),
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
    params = ci::params::InterfaceGl::create("Preview params", ci::app::toPixels(ci::ivec2(200, 400)));

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

    // params->addParam("Sea Color", &sea_color);
    params->addParam("Sea Speed x", &sea_speed_.x).step(0.00001f);
    params->addParam("Sea Speed y", &sea_speed_.y).step(0.00001f);
    params->addParam("Sea Wave", &sea_wave_).step(0.001f);
    params->addParam("Sea Level", &sea_level).step(0.25f);
    
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
    float x = pos.x / getWindowWidth();
    float y = 1.0f - pos.y / getWindowHeight();
    
    ci::Ray ray = camera.generateRay(x, y,
                                     camera.getAspectRatio());

    picked_ = false;
    float cross_min_z = std::numeric_limits<float>::max();
    
    float z;
    if (ray.calcPlaneIntersection(ci::vec3(0, 0, 0), ci::vec3(0, 1, 0), &z)) {
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
          if (s.getAABB().intersect(t_ray, &cross_z[0], &cross_z[1])) {
            if (cross_z[0] < cross_min_z) {
              // TriMeshを調べて交差点を特定する
              auto result = intersect(t_ray, s.getLandMesh());
              if (result.first && result.second < cross_min_z) {
                picked_ = true;

                cross_min_z = result.second;
                picked_pos_ = ray.calcPosition(result.second);

                // AABBも保持
                ci::mat4 m = glm::translate(ci::mat4(1.0), ci::vec3(x * BLOCK_SIZE, 0, z * BLOCK_SIZE));
                picked_aabb_ = s.getAABB().transformed(m);
              }
            }
          }
        }
      }
    }
  }


public:
  GameApp()
    : z_distance(200.0f),
      octave(4),
      seed(0),
      ramdom_scale(0.05f),
      height_scale(25.0f),
      random(octave, seed),
      sea_level(7.5f),
      sea_color(1, 1, 1, 0),
      stage(BLOCK_SIZE, random, ramdom_scale, height_scale)
  {}

  
  void setup() override {
#if defined (CINDER_COCOA_TOUCH)
    // 縦横画面両対応
    getSignalSupportedOrientations().connect([]() { return ci::app::InterfaceOrientation::All; });
#endif

    ci::gl::enableVerticalSync(true);
    
    fov    = 5.0f;
    near_z = 20.0f;
    far_z  = 3000.0f;

    int width  = ci::app::getWindowWidth();
    int height = ci::app::getWindowHeight();

    camera = ci::CameraPersp(width, height,
                             fov,
                             near_z, far_z);

    camera.setEyePoint(ci::vec3());
    camera.setViewDirection(ci::vec3{ 0.0f, 0.0f, -1.0f });

    rotate = glm::angleAxis(ci::toRadians(45.0f), ci::vec3(0.0f, 1.0f, 0.0f))
      * glm::angleAxis(ci::toRadians(-45.0f), ci::vec3(1.0f, 0.0f, 0.0f));

    createDialog();

    bg_color = ci::Color(0, 0, 0);

    touch_num = 0;
    // アクティブになった時にタッチ情報を初期化
    getSignalDidBecomeActive().connect([this](){ touch_num = 0; });

    // 表示用のデータを準備
    stage_drawer_.setup();

    createSeaMesh();

    // FBO
    auto format = ci::gl::Fbo::Format()
      .colorTexture()
      ;
    fbo = ci::gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format);

    // sea_speed_ = ci::vec2(0.0004f, 0.0006f);
    // sea_wave_ = 0.0541f;
    sea_wave_ = 0.0f;

    picked_ = false;
    
    ci::gl::enableDepthRead();
    ci::gl::enableDepthWrite();
    ci::gl::enable(GL_CULL_FACE);
  }

  void resize() override {
    camera.setFov(getVerticalFov());
    touch_num = 0;
  }

  void mouseDown(ci::app::MouseEvent event) override {
    if (touch_num > 1) return;

    if (event.isLeft()) {
      // TIPS:マウスとワールド座標で縦方向の向きが逆
      auto pos = event.getPos();
      mouse_prev_pos = pos;

      // クリックした位置のAABBを特定
      pickStage(pos);
    }
  }
  
  void mouseDrag(ci::app::MouseEvent event) override {
    if (touch_num > 1) return;

    if (!event.isLeftDown()) return;

    auto mouse_pos = event.getPos();

    if (event.isShiftDown()) {
      auto d = mouse_pos - mouse_prev_pos;
      ci::vec3 v(d.x, 0.0f, d.y);

      float t = std::tan(ci::toRadians(fov) / 2.0f) * z_distance;
      auto p = (rotate * v) * t * 0.005f;
      translate.x += p.x;
      translate.z += p.z;
    }
    else if (event.isControlDown()) {
      float d = mouse_pos.y - mouse_prev_pos.y;

      float t = std::tan(ci::toRadians(fov) / 2.0f) * z_distance;
      z_distance = std::max(z_distance - d * t * 0.008f, near_z);
    }
    else {
      ci::vec2 d(mouse_pos - mouse_prev_pos);
      float l = length(d);
      if (l > 0.0f) {
        d = normalize(d);
        ci::vec3 v(-d.y, -d.x, 0.0f);
        ci::quat r = glm::angleAxis(l * 0.01f, v);
        rotate = rotate * r;
      }

    }
    mouse_prev_pos = mouse_pos;
  }
  
  void mouseWheel(ci::app::MouseEvent event) override {
    // OSX:マルチタッチ操作の時に呼ばれる
    if (touch_num > 1) return;

    // 距離に応じて比率を変える
    float t = std::tan(ci::toRadians(fov) / 2.0f) * z_distance;
    z_distance = std::max(z_distance + event.getWheelIncrement() * t * 0.5f, near_z);
  }

  void keyDown(ci::app::KeyEvent event) override {
    int key_code = event.getCode();
  }

  void touchesBegan(ci::app::TouchEvent event) override {
    const auto& touches = event.getTouches();
    touch_num += touches.size();
  }
  
  void touchesMoved(ci::app::TouchEvent event) override {
//  if (touch_num < 2) return;

    const auto& touches = event.getTouches();

#if defined (CINDER_COCOA_TOUCH)
    if (touch_num == 1) {
      ci::vec2 d{ touches[0].getPos() -  touches[0].getPrevPos() };
      float l = length(d);
      if (l > 0.0f) {
        d = normalize(d);
        ci::vec3 v(-d.y, -d.x, 0.0f);
        ci::quat r = glm::angleAxis(l * 0.01f, v);
        rotate = rotate * r;
      }

      return;
    }
#endif
    if (touches.size() < 2) return;

    ci::vec3 v1{ touches[0].getX(), 0.0f, touches[0].getY() };
    ci::vec3 v2{ touches[1].getX(), 0.0f, touches[1].getY() };
    ci::vec3 v1_prev{ touches[0].getPrevX(), 0.0f, touches[0].getPrevY() };
    ci::vec3 v2_prev{ touches[1].getPrevX(), 0.0f, touches[1].getPrevY() };

    ci::vec3 d = v1 - v1_prev;

    float l = length(v2 - v1);
    float l_prev = length(v2_prev - v1_prev);
    float ld = l - l_prev;

    // 距離に応じて比率を変える
    float t = std::tan(ci::toRadians(fov) / 2.0f) * z_distance;

    if (std::abs(ld) < 3.0f) {
      auto p = (rotate * d) * t * 0.005f;
      translate.x += p.x;
      translate.z += p.z;
    }
    else {
      z_distance = std::max(z_distance - ld * t * 0.025f, 0.01f);
    }
  }
  
  void touchesEnded(ci::app::TouchEvent event) override {
    const auto& touches = event.getTouches();

    // 最悪マイナス値にならないよう
    touch_num = std::max(touch_num - int(touches.size()), 0);
  }  

  
  void update() override {
    // カメラ位置の計算
    auto pos = rotate * ci::vec3(0, 0, z_distance) - translate;
    camera.setEyePoint(pos);
    camera.setOrientation(rotate);

    sea_offset_ += sea_speed_;
  }

  
  void draw() override {
    ci::gl::setMatrices(camera);
    ci::gl::disableAlphaBlending();

    // 陸地の描画
    // 画面中央の座標をレイキャストして求めている
    ci::Ray ray = camera.generateRay(0.5f, 0.5f,
                                     camera.getAspectRatio());
    
    float z;
    if (ray.calcPlaneIntersection(ci::vec3(0, 0, 0), ci::vec3(0, 1, 0), &z)) {
      ci::vec3 p = ray.calcPosition(z);

      // 中央ブロックの座標
      ci::ivec2 pos(p.x / BLOCK_SIZE, p.z / BLOCK_SIZE);
      {
        // 海面演出のためにFBOへ描画
        ci::gl::ScopedViewport viewportScope(ci::ivec2(0), fbo->getSize());
        ci::gl::ScopedFramebuffer fboScope(fbo);
        ci::gl::clear();

        drawStage(pos);
      }
      
      ci::gl::clear(bg_color);

      // 海面の描画
      {
        fbo->getColorTexture()->bind(0);
        sea_texture_->bind(1);
        sea_shader_->uniform("offset", sea_offset_);
        sea_shader_->uniform("wave", sea_wave_);
        
        ci::gl::color(sea_color);
        for (int z = (pos.y - 2); z < (pos.y + 3); ++z) {
          for (int x = (pos.x - 2); x < (pos.x + 3); ++x) {
            ci::gl::pushModelView();

            ci::gl::translate(ci::vec3(x * BLOCK_SIZE, sea_level, z * BLOCK_SIZE));
            sea_mesh_->draw();

            ci::gl::popModelView();
          }
        }
      }
      
      drawStage(pos);

      if (picked_) {
        ci::gl::color(1, 0, 0);
        ci::gl::drawStrokedCube(picked_aabb_);
        ci::gl::drawSphere(picked_pos_, 0.1f);
      }
    }
    
    // ダイアログ表示
    drawDialog();
  }


  // 陸地の描画
  void drawStage(const ci::ivec2& center_pos) {
    ci::gl::setMatrices(camera);
    ci::gl::disableAlphaBlending();
    
    for (int z = (center_pos.y - 2); z < (center_pos.y + 3); ++z) {
      for (int x = (center_pos.x - 2); x < (center_pos.x + 3); ++x) {
        ci::gl::pushModelView();

        ci::gl::translate(ci::vec3(x * BLOCK_SIZE, 0.0f, z * BLOCK_SIZE));
        ci::ivec2 pos(x, z);
        stage_drawer_.draw(pos, stage.getStage(pos));
        
        ci::gl::popModelView();
      }
    }
  }
  
};

}


// FIXME:なんかいくない
enum {
  WINDOW_WIDTH  = 960,
  WINDOW_HEIGHT = 640,
};

// アプリのラウンチコード
CINDER_APP(ngs::GameApp, 
           ci::app::RendererGl,
           [](ci::app::App::Settings* settings) {
             // 画面サイズを変更する
             settings->setWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
             // Retinaディスプレイ対応
             settings->setHighDensityDisplayEnabled(false);
             
             // マルチタッチ有効
             settings->setMultiTouchEnabled(true);
           })

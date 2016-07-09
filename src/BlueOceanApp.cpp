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
#include "TiledStage.hpp"
#include "StageDraw.hpp"


namespace ngs {

class GameApp : public ci::app::App {
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

  ci::gl::Texture2dRef sea_texture_;
  ci::gl::GlslProgRef	sea_shader_;
  ci::gl::BatchRef sea_mesh_;

  StageDrawer stage_drawer_;

  
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
    stage = TiledStage(32, random, ramdom_scale, height_scale);
    stage_drawer_.clear();
  }

  void createSeaMesh() {
    ci::TriMesh mesh;
    ci::vec2 size(32, 32);

    ci::vec3 p[] = {
      {      0, 0,      0 },
      { size.x, 0,      0 },
      {      0, 0, size.y },
      { size.x, 0, size.y },
    };
    
    mesh.appendPositions(&p[0], 4);
    mesh.appendTriangle(0, 2, 1);
    mesh.appendTriangle(1, 2, 3);
    
    auto color = ci::gl::ShaderDef().color();
    sea_shader_ = ci::gl::getStockShader(color);
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

    params->addParam("Sea Color", &sea_color);
    params->addParam("Sea Level", &sea_level).step(0.25f);
    
  }

  void drawDialog() {
    params->draw();
  }
#endif


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
      stage(32, random, ramdom_scale, height_scale)
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

    rotate = glm::angleAxis(ci::toRadians(45.0f), ci::vec3(1.0f, 0.0f, 0.0f))
      * glm::angleAxis(ci::toRadians(45.0f), ci::vec3(0.0f, 1.0f, 0.0f));

    createDialog();

    bg_color = ci::Color(0, 0, 0);

    touch_num = 0;
    // アクティブになった時にタッチ情報を初期化
    getSignalDidBecomeActive().connect([this](){ touch_num = 0; });

    // 表示用のデータを準備
    stage_drawer_.setup();

    createSeaMesh();
    
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
    }
  }
  
  void mouseDrag(ci::app::MouseEvent event) override {
    if (touch_num > 1) return;

    if (!event.isLeftDown()) return;

    auto mouse_pos = event.getPos();

    if (event.isShiftDown()) {
      auto d = mouse_pos - mouse_prev_pos;
      ci::vec3 v(d.x, -d.y, 0.0f);

      float t = std::tan(ci::toRadians(fov) / 2.0f) * z_distance;
      translate += v * t * 0.004f;
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
        ci::vec3 v(d.y, d.x, 0.0f);
        ci::quat r = glm::angleAxis(l * 0.01f, v);
        rotate = r * rotate;
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
        ci::vec3 v(d.y, d.x, 0.0f);
        ci::quat r = glm::angleAxis(l * 0.01f, v);
        rotate = r * rotate;
      }

      return;
    }
#endif
    if (touches.size() < 2) return;

    ci::vec3 v1{ touches[0].getX(), -touches[0].getY(), 0.0f };
    ci::vec3 v2{ touches[1].getX(), -touches[1].getY(), 0.0f };
    ci::vec3 v1_prev{ touches[0].getPrevX(), -touches[0].getPrevY(), 0.0f };
    ci::vec3 v2_prev{ touches[1].getPrevX(), -touches[1].getPrevY(), 0.0f };

    ci::vec3 d = v1 - v1_prev;

    float l = length(v2 - v1);
    float l_prev = length(v2_prev - v1_prev);
    float ld = l - l_prev;

    // 距離に応じて比率を変える
    float t = std::tan(ci::toRadians(fov) / 2.0f) * z_distance;

    if (std::abs(ld) < 3.0f) {
      translate += d * t * 0.005f;
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
  }
  
  void draw() override {
    ci::gl::clear(bg_color);
    ci::gl::setMatrices(camera);

    ci::gl::translate(ci::vec3(0, 0.0, -z_distance));
    ci::gl::translate(translate);
    ci::gl::rotate(rotate);
    // ci::gl::scale(ci::vec3(1, 0.8, 1));

    // 陸地の描画
    ci::gl::disableAlphaBlending();
    for (int z = 0; z < 5; ++z) {
      for (int x = 0; x < 5; ++x) {
        ci::gl::pushModelView();

        ci::gl::translate(ci::vec3(x * 32.0f, 0.0f, z * 32.0f));

        ci::ivec2 pos(x, z);
        stage_drawer_.draw(pos, stage.getStage(pos));
        
        ci::gl::popModelView();
      }
    }

    // 海面の描画
    ci::gl::enableAlphaBlending();
    for (int z = 0; z < 5; ++z) {
      for (int x = 0; x < 5; ++x) {
        ci::gl::pushModelView();

        ci::gl::translate(ci::vec3(x * 32.0f, sea_level, z * 32.0f));
        ci::gl::color(sea_color);
        sea_mesh_->draw();

        ci::gl::popModelView();
      }
    }
    
    // ダイアログ表示
    drawDialog();
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

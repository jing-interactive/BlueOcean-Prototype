//
// BlueOcean プロトタイプ
// 

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>
#include <cinder/Camera.h>
#include <cinder/params/Params.h>
#include <cinder/Rand.h>
#include "Stage.hpp"
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
  
  Stage stage;

  ci::gl::BatchRef cube;

  
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
  
  // ダイアログ関連
#if defined (CINDER_COCOA_TOUCH)
  void createDialog() {}
  void drawDialog() {}
#else
  void createDialog() {
    // 各種パラメーター設定
    params = ci::params::InterfaceGl::create("Preview params", ci::app::toPixels(ci::ivec2(200, 400)));

    params->addParam("FOV", &fov).min(1.0f).max(180.0f).updateFn([this]() {
        camera.setFov(fov);
      });

    params->addParam("NEAR Z", &near_z).min(0.1f).updateFn([this]() {
        camera.setNearClip(near_z);
      });
    params->addParam("FAR Z", &far_z).min(0.1f).updateFn([this]() {
        camera.setFarClip(far_z);
      });

    params->addSeparator();

    params->addParam("BG", &bg_color);
  }

  void drawDialog() {
    params->draw();
  }
#endif


public:
  GameApp()
    : stage(50, 50),
      z_distance(100.0f)
  {
  }

  
  void setup() override {
#if defined (CINDER_COCOA_TOUCH)
    // 縦横画面両対応
    getSignalSupportedOrientations().connect([]() { return ci::app::InterfaceOrientation::All; });
#endif

    ci::gl::enableVerticalSync(true);
    
    fov    = 5.0f;
    near_z = 10.0f;
    far_z  = 1000.0f;

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

    // 表示用のモデルデータ
    auto lambert = ci::gl::ShaderDef().lambert().color();
    ci::gl::GlslProgRef	shader = ci::gl::getStockShader(lambert);
    cube = ci::gl::Batch::create(ci::geom::Cube(), shader);
    
    ci::gl::enableAlphaBlending();
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
      z_distance = std::max(z_distance - ld * t * 0.01f, 0.01f);
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

    StageDraw::draw(stage.terrains(), cube);
    
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

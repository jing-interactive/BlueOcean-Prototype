//
// BlueOcean プロトタイプ
// 

#include "Defines.hpp"
#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include "Params.hpp"
#include "JsonUtil.hpp"
#include "Touch.hpp"
#include "Misc.hpp"
#include "Worker.hpp"


namespace ngs {

class App : public ci::app::App {
  // タッチ操作中の数
  int touch_num_;
  ci::vec2 mouse_prev_pos_;
  
  std::unique_ptr<Worker> worker_;

  
public:
  App()
    : touch_num_(0),
      worker_(std::unique_ptr<Worker>(new Worker))
  {
#if defined (CINDER_COCOA_TOUCH)
    // 縦横画面両対応
    getSignalSupportedOrientations().connect([]() {
        return ci::app::InterfaceOrientation::All;
      });
#endif

    // アクティブになった時にタッチ情報を初期化
    getSignalDidBecomeActive().connect([this]() {
        DOUT << "SignalDidBecomeActive" << std::endl;
        // game_->resetTouch();
        touch_num_ = 0;
      });

    // 非アクティブ時
    getSignalWillResignActive().connect([this]() {
        DOUT << "SignalWillResignActive" << std::endl;
      });
    
#if defined (CINDER_COCOA_TOUCH)
    // バックグラウンド移行
    getSignalDidEnterBackground().connect([this]() {
        DOUT << "SignalDidEnterBackground" << std::endl;
      });
#endif
  }
  

private:
  void resize() override {
    float aspect = ci::app::getWindowAspectRatio();
    
    worker_->resize(aspect);
  }

  
  void mouseDown(ci::app::MouseEvent event) override {
    // タッチ判定との整合性を取るため左クリック以外は無視
    if (!event.isLeft()) return;
    
    Touch touch(std::numeric_limits<uint32_t>::max(),
                event.getPos(),
                event.getPos());

    mouse_prev_pos_ = event.getPos();

    worker_->touchesBegan(1, { touch });
  }
  
  void mouseDrag(ci::app::MouseEvent event) override {
    if (!event.isLeftDown()) return;

    Touch touch(std::numeric_limits<uint32_t>::max(),
                event.getPos(),
                mouse_prev_pos_);

    mouse_prev_pos_ = event.getPos();

    worker_->touchesMoved(1, { touch });
  }

  void mouseUp(ci::app::MouseEvent event) override {
    if (!event.isLeft()) return;
    
    Touch touch(std::numeric_limits<uint32_t>::max(),
                event.getPos(),
                mouse_prev_pos_);

    worker_->touchesEnded(0, { touch });
  }

#if 0
  void mouseWheel(ci::app::MouseEvent event) override {
    DOUT << "mouseWheel" << std::endl;
    // game_->mouseWheel(event);
  }
#endif

  void touchesBegan(ci::app::TouchEvent event) override {
    const auto& touches = event.getTouches();
    touch_num_ += touches.size();

#if defined (CINDER_MAC)
    // シングルタッチ操作は無視
    if (touch_num_ == 1) return;
#endif

    auto app_touches = createTouchInfo(touches);
    worker_->touchesBegan(touch_num_, app_touches);
  }
  
  void touchesMoved(ci::app::TouchEvent event) override {
#if defined (CINDER_MAC)
    // シングルタッチ操作は無視
    if (touch_num_ == 1) return;
#endif
    
    const auto& touches = event.getTouches();
    auto app_touches = createTouchInfo(touches);
    worker_->touchesMoved(touch_num_, app_touches);
  }
  
  void touchesEnded(ci::app::TouchEvent event) override {
    const auto& touches = event.getTouches();
    touch_num_ = std::max(touch_num_ - int(touches.size()), 0);

    int num = touch_num_;
#if defined (CINDER_MAC)
    // シングルタッチは無視
    if ((touches.size() == 1) && (touch_num_ == 0)) return;

    if (num == 1) num = 0;
#endif

    auto app_touches = createTouchInfo(touches);
    worker_->touchesEnded(num, app_touches);
  }


  void keyDown(ci::app::KeyEvent event) override {
    int key_code   = event.getCode();
    char key_chara = event.getChar();

    switch (key_code) {
    case ci::app::KeyEvent::KEY_r:
      // ソフトリセット
      // TIPS:Workerが２つ存在してしまう状況を避けるため、先にresetしている
      worker_.reset();
      worker_ = std::unique_ptr<Worker>(new Worker);
      break;
    }

    if ((key_chara < '0') || (key_chara > '9')) {
      // JsonTree::hasChild では数字の場合に自動的にインデックス値
      // で判断してくれるので、数字キーは除外してから処理

      const auto& params = worker_->getParams();
      
      // paramsに書かれたsignalを発生
      // TIPS:大文字小文字の区別をしている
      auto debug_signal = std::string(1, key_chara);
      if (params["debug_signal"].hasChild(debug_signal, true)) {
        auto message = params.getValueForKey<std::string>("debug_signal." + debug_signal, true);
        
        DOUT << "debug-signal: "
             << debug_signal
             << " "
             << message << std::endl;
      
        worker_->getEvent().signal(message, Arguments());
      }
    }
  }
  

  void update() override {
    worker_->update();
  }

  
  void draw() override {
    worker_->draw();
  }

  void cleanup() override {
    // TIPS:iOSもアプリ終了タイミングで読んでくれる
    worker_->cleanup();
  }
  
};

}


// アプリのラウンチコード
CINDER_APP(ngs::App, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(0)),
           [](ci::app::App::Settings* settings) {
             // FIXME:ここで設定ファイルを読むなんて...
             auto params = ngs::Params::load("params.json");

             settings->setWindowSize(ngs::Json::getVec<ci::ivec2>(params["app.size"]));
             
             settings->setMultiTouchEnabled();
             settings->setPowerManagementEnabled(params.getValueForKey<bool>("app.power_management"));
             settings->setHighDensityDisplayEnabled(params.getValueForKey<bool>("app.high_density_display"));
             settings->setFrameRate(params.getValueForKey<int>("app.frame_rate"));
             settings->setTitle(PREPRO_TO_STR(PRODUCT_NAME));
             
             // settings->disableFrameRate();
             // ci::gl::enableVerticalSync();
           });

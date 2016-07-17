//
// BlueOcean プロトタイプ
// 

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include "Game.hpp"


namespace ngs {

class GameApp : public ci::app::App {
  std::unique_ptr<Game> game_;

  
public:
  GameApp()
    : game_(std::unique_ptr<Game>(new Game))
  {
#if defined (CINDER_COCOA_TOUCH)
    // 縦横画面両対応
    getSignalSupportedOrientations().connect([]() { return ci::app::InterfaceOrientation::All; });
#endif

    // アクティブになった時にタッチ情報を初期化
    getSignalDidBecomeActive().connect([this](){ game_->resetTouch(); });
  }
  

private:
  void resize() override {
    game_->resize();
  }

  void mouseDown(ci::app::MouseEvent event) override {
    game_->mouseDown(event);
  }
  
  void mouseDrag(ci::app::MouseEvent event) override {
    game_->mouseDrag(event);
  }
  
  void mouseWheel(ci::app::MouseEvent event) override {
    game_->mouseWheel(event);
  }

  void mouseUp(ci::app::MouseEvent event) override {
    game_->mouseUp(event);
  }

  void keyDown(ci::app::KeyEvent event) override {
    int key_code = event.getCode();

    switch (key_code) {
    case ci::app::KeyEvent::KEY_r:
      // ソフトリセット
      game_.reset();
      game_ = std::unique_ptr<Game>(new Game);
      break;
    }
  }

  void touchesBegan(ci::app::TouchEvent event) override {
    game_->touchesBegan(event);
  }
  
  void touchesMoved(ci::app::TouchEvent event) override {
    game_->touchesMoved(event);
  }
  
  void touchesEnded(ci::app::TouchEvent event) override {
    game_->touchesEnded(event);
  }

  
  void update() override {
    game_->update();
  }

  
  void draw() override {
    game_->draw();
  }
  
};

}


// アプリのラウンチコード
CINDER_APP(ngs::GameApp, ci::app::RendererGl,
           [](ci::app::App::Settings* settings) {
             // FIXME:ここで設定ファイルを読むなんて...
             auto params = ngs::Params::load("params.json");

             settings->setWindowSize(ngs::Json::getVec<ci::ivec2>(params["app.size"]));
             
             settings->setMultiTouchEnabled();
             settings->setPowerManagementEnabled(false);
             settings->setHighDensityDisplayEnabled(false);
             
             // settings->disableFrameRate();
             // ci::gl::enableVerticalSync();
           });

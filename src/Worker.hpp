#pragma once

//
// アプリの外枠
//

#include "Event.hpp"
#include "Arguments.hpp"
#include "Params.hpp"
#include "Touch.hpp"
#include "SceneGame.hpp"
#include "SceneItemReporter.hpp"


namespace ngs {

class Worker {
  // 汎用的なコールバック管理
  Event<Arguments> event_;

  // ゲーム内パラメーター
  ci::JsonTree params_;

  // ゲーム世界
  std::shared_ptr<Game> game_;

  // 現在の画面
  std::shared_ptr<SceneBase> scene_;


  // シーンファクトリー
  void setupFactory() {
    event_.connect("scene_game",
                   [this](const Connection&, const Arguments&) {
                     DOUT << "scene_game" << std::endl;

                     // FIXME:イベント送信元が破棄されてしまうと未定義X(
                     scene_.reset();
                     scene_ = std::make_shared<SceneGame>(event_, game_);
                   });

    event_.connect("scene_item_reporter",
                   [this](const Connection&, const Arguments&) {
                     DOUT << "scene_item_reporter" << std::endl;

                     scene_.reset();
                     scene_ = std::make_shared<SceneItemReporter>(event_, params_);
                   });
  }
  

public:
  Worker()
    : params_(Params::load("params.json")),
      game_(std::make_shared<Game>(event_, params_))
  {
    setupFactory();

    // 最初のシーンを生成
    event_.signal("scene_game", Arguments());
  }


  void cleanup() {
    game_->cleanup();
  }

  
  void resize(const float aspect) {
    scene_->resize(aspect);
  }


  // touching_numはタッチ操作中の数(新たに発生したのも含む)
  // touchesは新たに発生したタッチイベント内容
  void touchesBegan(const int touching_num, const std::vector<Touch>& touches) {
    scene_->touchesBegan(touching_num, touches);
  }

  // touching_numはタッチ操作中の数
  void touchesMoved(const int touching_num, const std::vector<Touch>& touches) {
    scene_->touchesMoved(touching_num, touches);
  }

  // touching_numは残りのタッチ操作中の数
  void touchesEnded(const int touching_num, const std::vector<Touch>& touches) {
    scene_->touchesEnded(touching_num, touches);
  }
  

  void update() {
    scene_->update();
  }

  void draw() {
    scene_->draw();
  }


  // デバッグ用途
  Event<Arguments>& getEvent() { return event_; }
  const ci::JsonTree& getParams() const { return params_; }

};

}

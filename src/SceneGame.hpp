#pragma once

//
// ゲーム本編のシーン
//

#include "SceneBase.hpp"
#include "Game.hpp"


namespace ngs {

class SceneGame
  : public SceneBase {

  // FIXME:スマートポインタの方が安全
  Event<Arguments>& event_;
  
  // ゲーム本編は他のシーンと共有している
  std::shared_ptr<Game> game_;
  

  // TIPS:SceneBase* 経由でないと呼び出せなくしたいので
  //      わざとprivate
  void resize(const float aspect) override {
    game_->resize(aspect);
  }

  void touchesBegan(const int touching_num, const std::vector<Touch>& touches) override {
    game_->touchesBegan(touching_num, touches);
  }
  
  void touchesMoved(const int touching_num, const std::vector<Touch>& touches) override {
    game_->touchesMoved(touching_num, touches);
  }
  
  void touchesEnded(const int touching_num, const std::vector<Touch>& touches) override {
    game_->touchesEnded(touching_num, touches);
  }

  void update() override {
    game_->update();
  }
  
  void draw() override {
    game_->draw();
  }

  
public:
  SceneGame(Event<Arguments>& event,
            const std::shared_ptr<Game>& game)
    : event_(event),
      game_(game)
  {}

};

}

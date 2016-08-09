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
  const ci::JsonTree& params_;
  
  // ゲーム本編は他のシーンと共有している
  std::shared_ptr<Game> game_;

  bool active_ = true;
  

  // TIPS:SceneBase* 経由でないと呼び出せなくしたいので
  //      わざとprivate
  bool isActive() const override { return active_; }

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
            const ci::JsonTree& params,
            const std::shared_ptr<Game>& game)
    : event_(event),
      params_(params),
      game_(game)
  {
    // 移動終了時に適当にアイテムをゲットする
    event_.connect("ship_arrival",
                   [this](const Connection&, const Arguments&) {
                     int total_num = params_["item.body"].getNumChildren();
                     int index = ci::randInt(total_num);
                     
                     Arguments arguments {
                       { "name", params_["item.body"].getValueAtIndex<std::string>(index) }
                     };
                     
                     event_.signal("scene_item_reporter", arguments);
                   });
  }

};

}

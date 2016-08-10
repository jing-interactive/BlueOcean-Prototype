#pragma once

//
// ゲーム本編のシーン
//

#include "SceneBase.hpp"
#include "Game.hpp"
#include "ConnectionHolder.hpp"


namespace ngs {

class SceneGame
  : public SceneBase {

  // FIXME:スマートポインタの方が安全
  Event<Arguments>& event_;
  ConnectionHolder holder_;

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
  
  void draw(const bool offscreen) override {
    game_->draw();
    if (!offscreen) game_->debugDraw();
  }

  
public:
  SceneGame(Event<Arguments>& event,
            const ci::JsonTree& params,
            const std::shared_ptr<Game>& game)
    : event_(event),
      params_(params),
      game_(game)
  {
#if 0
    // 探索終了時に適当にアイテムをゲットする
    holder_ += event_.connect("search_finish",
                              [this](const Connection&, const Arguments&) {
                                int total_num = params_["item.body"].getNumChildren();
                                int index = ci::randInt(total_num);

                                DOUT << total_num << "," << index << std::endl;
                     
                                Arguments arguments {
                                  { "item", index }
                                };
                     
                                event_.signal("scene_item_reporter", arguments);
                              });
#endif

    // アイテムゲット画面起動時はデバッグダイアログを破棄
    holder_ += event_.connect("pause_game",
                              [this](const Connection&, const Arguments&) {
                                game_->destroyDialog();
                              });

    // アイテムゲット画面終了時にデバッグダイアログを復活
    holder_ += event_.connect("resume_game",
                              [this](const Connection&, const Arguments&) {
                                game_->createDialog();
                              });
  }

};

}

#pragma once

//
// アイテム発見画面
//

#include "SceneBase.hpp"
#include "ItemReporter.hpp"


namespace ngs {

class SceneItemReporter
  : public SceneBase {

  // FIXME:スマートポインタの方が安全
  Event<Arguments>& event_;

  ItemReporter item_reporter_;
  

  // TIPS:SceneBase* 経由でないと呼び出せなくしたいので
  //      わざとprivate
  void resize(const float aspect) override {
    item_reporter_.resize(aspect);
  }

  void touchesBegan(const int touching_num, const std::vector<Touch>& touches) override {
    item_reporter_.touchesBegan(touching_num, touches);
  }
  
  void touchesMoved(const int touching_num, const std::vector<Touch>& touches) override {
    item_reporter_.touchesMoved(touching_num, touches);
  }
  
  void touchesEnded(const int touching_num, const std::vector<Touch>& touches) override {
    item_reporter_.touchesEnded(touching_num, touches);
  }

  void update() override {
    item_reporter_.update();
  }
  
  void draw() override {
    item_reporter_.draw();
  }

  
public:
  SceneItemReporter(Event<Arguments>& event,
                    const ci::JsonTree& params)
    : event_(event),
      item_reporter_(params["item_reporter"])
  {}

};

}

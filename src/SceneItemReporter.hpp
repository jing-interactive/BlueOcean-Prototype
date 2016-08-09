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

  bool active_ = true;

  

  // TIPS:SceneBase* 経由でないと呼び出せなくしたいので
  //      わざとprivate
  bool isActive() const override {
    return active_;
  }
  
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
                    const ci::JsonTree& params,
                    const std::string& name)
    : event_(event),
      item_reporter_(event, params["item_reporter"])
  {
    item_reporter_.loadItem(params["item.body"][name]);

    event_.connect("close_item_reporter",
                   [this](const Connection&, const Arguments&) {
                     active_ = false;
                   });
  }

};

}

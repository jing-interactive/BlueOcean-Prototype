#pragma once

//
// 画面遷移の基本クラス
//

#include "Touch.hpp"


namespace ngs {

struct SceneBase {
  virtual ~SceneBase() = default;

  virtual bool isActive() const = 0;
  
  virtual void resize(const float aspect) = 0;

  // TODO:イベント方式にする
  virtual void touchesBegan(const int touching_num, const std::vector<Touch>& touches) = 0;
  virtual void touchesMoved(const int touching_num, const std::vector<Touch>& touches) = 0;
  virtual void touchesEnded(const int touching_num, const std::vector<Touch>& touches) = 0;

  virtual void update() = 0;
  virtual void draw() = 0;
};

}

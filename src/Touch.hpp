#pragma once

//
// タッチ入力
//

namespace ngs {

class Touch {
  uint32_t id_;

  ci::vec2 pos_;
  ci::vec2 prev_pos_;

  bool is_mouse_;
  

public:
  Touch(const uint32_t id,
        ci::vec2 pos, ci::vec2 prev_pos,
        const bool is_mouse)
    : id_(id),
      pos_(std::move(pos)),
      prev_pos_(std::move(prev_pos)),
      is_mouse_(is_mouse)
  {}


  uint32_t getId() const { return id_; }
  const ci::vec2& getPos() const { return pos_; }
  const ci::vec2& getPrevPos() const { return prev_pos_; }
  bool isMouse() const { return is_mouse_; }

};

}

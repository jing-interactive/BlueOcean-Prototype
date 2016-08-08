#pragma once

//
// タッチ入力
//

namespace ngs {

class Touch {
  uint32_t id_;

  ci::vec2 pos_;
  ci::vec2 prev_pos_;
  

public:
  Touch(const uint32_t id,
        ci::vec2 pos, ci::vec2 prev_pos)
    : id_(id),
      pos_(std::move(pos)),
      prev_pos_(std::move(prev_pos))
  {}


  uint32_t getId() const { return id_; }
  const ci::vec2& getPos() const { return pos_; }
  const ci::vec2& getPrevPos() const { return prev_pos_; }

};

}

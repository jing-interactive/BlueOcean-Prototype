#pragma once

//
// ステージ上のオブジェクト
//

#include <glm/gtx/transform.hpp>


namespace ngs {

class StageObj {
  std::string name_;
  ci::mat4 transfomation_;
  
  
public:
  StageObj(std::string name,
           const ci::vec3& position, const ci::vec3& rotation, const ci::vec3& scaling)
    : name_(std::move(name))
  {
    transfomation_ = glm::translate(position) * glm::mat4_cast(glm::quat(rotation)) * glm::scale(scaling);
  }

  const std::string& getName() const {
    return name_;
  }

  const ci::mat4& getTransfomation() const {
    return transfomation_;
  }

};

}

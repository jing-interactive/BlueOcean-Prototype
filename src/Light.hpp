#pragma once

//
// 平行光源
//

namespace ngs {

struct Light {
  // FIXME:試作版なので単なる構造体
  ci::vec4 direction;
  
  ci::ColorA ambient;
  ci::ColorA diffuse;
  ci::ColorA specular;
};

}

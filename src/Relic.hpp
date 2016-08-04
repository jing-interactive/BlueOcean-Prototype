#pragma once

//
// ステージ上の遺物
//

namespace ngs {

struct Relic {
  ci::ivec3 position;
  std::string type;
  bool searched;
};

}

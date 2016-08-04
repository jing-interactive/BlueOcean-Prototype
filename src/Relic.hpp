#pragma once

//
// ステージ上の遺物
//

namespace ngs {

struct Relic {
  ci::ivec3 position;
  std::string type;

  // 水面上にあって、目視された
  bool found;

  // 探索終了
  bool searched;
  // 探索終了までの残り時間
  double time_remains;
};

}

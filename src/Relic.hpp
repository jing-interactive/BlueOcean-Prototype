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
  // 探索に必要な時間
  double search_required_time;
  // 探索した時間
  double searched_time;

  // レア度(1.0でSSS)
  float rare;
};

}

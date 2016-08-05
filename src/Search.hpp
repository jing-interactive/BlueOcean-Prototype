#pragma once

//
// 探索
//

namespace ngs { namespace Search {

struct Result {
  ci::vec2 block_pos;
  size_t index;

    
  Result() = default;
    
  Result(const ci::vec2& p, const size_t i)
    : block_pos(p),
      index(i)
  {}
};


// 指定座標の遺物を探す
//   高さは考慮しない
std::pair<bool, Result> getRelic(const ci::ivec3& pos, const TiledStage& stage) {
  int block_x = glm::floor(pos.x / 64.0f);
  int block_z = glm::floor(pos.z / 64.0f);

  ci::ivec2 block_pos(block_x, block_z);
  ci::ivec3 offset(block_x * 64, 0, block_z * 64);
    
  const auto& relics = stage.getRelics(block_pos);
  for (size_t i = 0; i < relics.size(); ++i) {
    if (relics[i].searched) continue;

    ci::ivec3 p = relics[i].position + offset;
    if ((p.x == pos.x) && (p.z == pos.z)) {
      return std::make_pair(true, Result(block_pos, i));
    }
  }

  return std::make_pair(false, Result());
}
  
  
// 近くに未調査の遺物があるか判定
std::pair<bool, Result> checkNearRelic(const ci::vec3& ship_pos, const TiledStage& stage) {
  ci::ivec3 pos(ship_pos);
    
  ci::ivec3 tbl[] = {
    { 0, 0, 0 },

    {  1, 0,  0 },
    { -1, 0,  0 },
    {  0, 0,  1 },
    {  0, 0, -1 },
  };

  for (const auto& offset : tbl) {
    auto result = getRelic(pos + offset, stage);

    if (result.first) {
      return result;
    }
  }

  return std::make_pair(false, Result());
}

} }

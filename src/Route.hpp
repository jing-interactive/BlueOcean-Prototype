#pragma once

//
// 海上の経路探索
//  A*アルゴリズムを使い、目的地までの最短経路を検索
//  SOURCE:http://qiita.com/2dgames_jp/items/f29e915357c1decbc4b7
//

#include "TiledStage.hpp"
#include <set>
#include <queue>
#include <list>


namespace ngs { namespace Route {

struct Node {
  ci::ivec3 pos;
  ci::ivec3 prev_pos;
  int height;

  int cost;
  int estimate_cost;
  int score;

  bool operator<(const Node& rhs) const {
    return score < rhs.score;
  }

  bool operator>(const Node& rhs) const {
    return score > rhs.score;
  }
};


// 指定座標のステージの高さを求める
int getStageHeight(const ci::ivec3& pos, TiledStage& stage) {
    int block_x = glm::floor(pos.x / 64.0f);
    int block_z = glm::floor(pos.z / 64.0f);

    const auto& height_map = stage.getStage(ci::ivec2(block_x, block_z)).getHeightMap();

    // TIPS:負数の場合に答えが正数になる剰余算を使っている
    //        値: -1, -2, -3, -4...
    //      結果: 63, 62, 61, 60...
    int x = glm::mod(float(pos.x), 64.0f);
    int z = glm::mod(float(pos.z), 64.0f);

    return height_map[z][x];
}

// 海面より１ブロック高く、海面に接した陸地か調べる
// 引数のpos.yが海面の高さを表す
bool canSearch(const ci::ivec3 pos, TiledStage& stage) {
  int height = getStageHeight(pos, stage);

  if (height > (pos.y + 1)) return false;

  // ４方向のどこかに海はあるか
  ci::ivec3 vector[] = {
    {  1, 0,  0 },
    { -1, 0,  0 },
    {  0, 0,  1 },
    {  0, 0, -1 },
  };

  for (const auto& v : vector) {
    auto new_pos = pos + v;
  
    int height = getStageHeight(new_pos, stage);
    if (height <= pos.y) return true;
  }
    
  return false;
}

// 次の経路をキューに積む
void stackNextRoute(std::map<ci::ivec3, Node, LessVec<ci::ivec3>>& opened,
                    std::priority_queue<Node, std::vector<Node>, std::greater<Node>>& queue,
                    const ci::ivec3& prev_pos, const int prev_cost,
                    const ci::ivec3& end,
                    TiledStage& stage) {
  // ４方向へ進んでみてコストを計算する
  ci::ivec3 vector[] = {
    {  1, 0,  0 },
    { -1, 0,  0 },
    {  0, 0,  1 },
    {  0, 0, -1 },
  };

  for (const auto& v : vector) {
    auto new_pos = prev_pos + v;
    if (opened.count(new_pos)) continue;

    int height = getStageHeight(new_pos, stage);
    if (height > new_pos.y) continue;

    auto d = end - new_pos;
    int estimate_cost = int(std::abs(d.x) + std::abs(d.z));
    int cost = prev_cost + 1;
    int score = estimate_cost + cost;
    
    Node node = {
      new_pos,
      prev_pos,
      height,
      
      cost,
      estimate_cost,
      score,
    };

    queue.push(node);
    opened.insert(std::make_pair(new_pos, node));
  }
}


std::vector<ci::ivec3> search(const ci::ivec3& start, const ci::ivec3& end,
                              TiledStage& stage) {
  std::map<ci::ivec3, Node, LessVec<ci::ivec3>> opened;
  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> queue;  

  int height = getStageHeight(start, stage);

  DOUT << "start:" << start << std::endl;
  DOUT << "  end:" << end << std::endl;
  
  // スタート地点をキューに積む
  Node node = {
    start,
    start,
    height,
    0,
    0,
    0,
  };

  opened.insert(std::make_pair(start, node));
  queue.push(node);

  bool arrival = false;
  while (!queue.empty()) {
    const auto node = queue.top();
    queue.pop();

    if (node.pos == end) {
      arrival = true;
      break;
    }

    stackNextRoute(opened, queue,
                   node.pos, node.cost,
                   end,
                   stage);
  }

  if (!arrival) {
    DOUT << "No route." << std::endl;
    return std::vector<ci::ivec3>();
  }
  
  // ゴール地点からスタート地点までを辿る
  ci::ivec3 pos = end;
  std::vector<ci::ivec3> roots;
  while (1) {
    auto node = opened.at(pos);

    // コストが0: スタート地点
    if (!node.cost) break;
    
    ci::ivec3 p(pos.x, node.height, pos.z);
    roots.push_back(p);
    
    pos = node.prev_pos;
  }

  std::reverse(std::begin(roots), std::end(roots));

  return roots;
}

} }

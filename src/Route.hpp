#pragma once

//
// 海上の経路探索
//  A*アルゴリズムを使い、目的地までの最短経路を検索
//  SOURCE:http://qiita.com/2dgames_jp/items/f29e915357c1decbc4b7
//

#include "Waypoint.hpp"
#include "TiledStage.hpp"
#include "Sea.hpp"
#include <set>
#include <queue>
#include <list>


namespace ngs { namespace Route {

struct Node {
  ci::ivec3 pos;
  ci::ivec3 prev_pos;

  double duration;
  double arrived_duration;

  bool operator<(const Node& rhs) const {
    return arrived_duration < rhs.arrived_duration;
  }

  bool operator>(const Node& rhs) const {
    return arrived_duration > rhs.arrived_duration;
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

  DOUT << "height:" << height << " sea_level:" << pos.y << std::endl;
  
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


// 移動コストを計算
//   １ブロック移動するためには現在地と移動先がどちらも海面より低くなければならない
// required: １ブロック移動するのに必要な時間
// 戻り値: 到着時間  
double calcCost(const int current_height, const int target_height,
                double duration, const double required,
                const Sea& sea) {
  while (1) {
    int start_level = sea.getLevel(duration);
    int end_level   = sea.getLevel(duration + required);

    if (current_height < start_level && target_height < end_level) {
      break;
    }

    // 移動開始時間を少し遅らせる
    duration += required * 0.25;
  }
  
  return duration + required;
}


// 次の経路をキューに積む
// 戻り値 trueで到着
bool stackNextRoute(std::map<ci::ivec3, Node, LessVec<ci::ivec3>>& opened,
                    std::priority_queue<Node, std::vector<Node>, std::greater<Node>>& queue,
                    const Node& prev_node,
                    const ci::ivec3& end,
                    const int max_distance,
                    const double required,
                    TiledStage& stage, const Sea& sea) {
  // ４方向へ進んでみてコストを計算する
  ci::ivec3 vector[] = {
    {  1, 0,  0 },
    { -1, 0,  0 },
    {  0, 0,  1 },
    {  0, 0, -1 },
  };

  for (const auto& v : vector) {
    auto new_pos = prev_node.pos + v;
    new_pos.y = getStageHeight(new_pos, stage);

    // 一度通った場所はスルー
    if (opened.count(new_pos)) continue;

    auto d = end - new_pos;
    int distance = std::abs(d.x) + std::abs(d.z);
    // 離れすぎたらスルー
    if (distance > max_distance) continue;

    // 現在位置から最適パターンで到着する場合の所要時間
    double estimate_time = distance * required;

    // 潮の満ち引きを考慮した到着時間
    double arrived_time = calcCost(prev_node.pos.y, new_pos.y,
                                   prev_node.duration, required,
                                   sea);
    
    Node node = {
      new_pos,
      prev_node.pos,
      
      arrived_time,
      arrived_time + estimate_time,
    };

    queue.push(node);
    opened.insert(std::make_pair(new_pos, node));

    if (new_pos == end) return true;
  }
  
  return false;
}


// 経路探索
// duration  移動開始時間
// required  １ブロック移動の所要時間
std::vector<Waypoint> search(ci::ivec3 start, ci::ivec3 end,
                             double duration, const double required,
                             TiledStage& stage, const Sea& sea) {
  std::map<ci::ivec3, Node, LessVec<ci::ivec3>> opened;
  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> queue;  

  start.y = getStageHeight(start, stage);
  end.y   = getStageHeight(end, stage);
  
  DOUT << "start:" << start << std::endl;
  DOUT << "  end:" << end << std::endl;
  
  // スタート地点をキューに積む
  Node node = {
    start,
    start,
    duration,
    duration,
  };

  auto d = end - start;
  int max_distance = (std::abs(d.x) + std::abs(d.z)) * 1.5;
  
  opened.insert(std::make_pair(start, node));
  queue.push(node);

  bool arrival = false;
  while (!arrival && !queue.empty()) {
    // キューに積まれた位置から、もっとも到着時間が早いものを取り出す
    const auto node = queue.top();
    queue.pop();

    arrival = stackNextRoute(opened, queue,
                             node,
                             end,
                             max_distance,
                             required,
                             stage, sea);
  }

  if (!arrival) {
    DOUT << "No route." << std::endl;
    return std::vector<Waypoint>();
  }

  // ゴール地点からスタート地点までを辿る
  ci::ivec3 pos = end;
  std::vector<Waypoint> roots;
  while (1) {
    auto node = opened.at(pos);
    roots.push_back({ pos, node.duration });

    if (pos == start) break;
    
    pos = node.prev_pos;
  }

  std::reverse(std::begin(roots), std::end(roots));

  return roots;
}

} }

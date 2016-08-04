#pragma once

// 
// 経路上の通過地点
//

namespace ngs {

struct Waypoint {
  ci::ivec3 pos;
  double duration;
};

}

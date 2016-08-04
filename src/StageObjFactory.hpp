#pragma once

//
// StageObj生成工場
//

#include <cinder/Rand.h>
#include "JsonUtil.hpp"


namespace ngs {

class StageObjFactory {
  struct Info {
    std::string name;
    float probability;
  };
  
  std::map<int, std::vector<Info>> info_;
  

public:
  StageObjFactory(const ci::JsonTree& params) {
    for (const auto& p : params) {
      Info info = {
        p.getValueForKey<std::string>("name"),
        p.getValueForKey<float>("probability"),
      };
      
      ci::ivec2 height = Json::getVec<ci::ivec2>(p["height"]);
      for (int y = height.x; y <= height.y; ++y) {
        // TIPS:std::mapは要素が存在しない場合、デフォルトコンストラクタを呼ぶ
        //      それを利用してコード量を減らしている
        info_[y].push_back(info);
      }
    }
  }
    

  std::pair<bool, std::string> create(const int height) const {
    if (!info_.count(height)) {
      return std::make_pair(false, std::string());
    }

    float probability = ci::randFloat();
    const auto& info = info_.at(height);
    for (const auto& i : info) {
      if (probability < i.probability) {
        return std::make_pair(true, i.name);
      }
      
      probability -= i.probability;
    }

    return std::make_pair(false, std::string());
  }
  
};

}

#pragma once

//
// 1日の光源の変化
// FIXME:ambientとdiffuseのみの変化
//

#include <cinder/Json.h>
#include "Time.hpp"


namespace ngs {

struct KeyLight {
  double time;
  Light light;
};


class DayLighting {
  std::vector<KeyLight> lights_;


public:
  DayLighting(const ci::JsonTree& params) {
    for (const auto& p : params) {
      KeyLight key;

      key.time = p.getValueForKey<double>("time");
      key.light.ambient = Json::getColorA<float>(p["ambient"]);
      key.light.diffuse = Json::getColorA<float>(p["diffuse"]);

      lights_.push_back(key);
    }
  }

  
  Light update(const double duration) {
    double last_time = lights_.back().time;

    // 1日でループする時間を求める
    double time = std::fmod(duration, last_time);
    size_t i;
    for (i = 0; i < (lights_.size() - 1); ++i) {
      if(time >= lights_[i].time && time <= lights_[i + 1].time) {
        break;
      }
    }
    assert(i < (lights_.size() - 1));

    const auto& start_key = lights_[i];
    const auto& end_key   = lights_[i + 1];
    
    double t = (time - start_key.time) / (end_key.time - start_key.time);

    Light light;
    light.ambient = start_key.light.ambient.lerp(t, end_key.light.ambient);
    light.diffuse = start_key.light.diffuse.lerp(t, end_key.light.diffuse);
    
    return light;
  }
  
};

}

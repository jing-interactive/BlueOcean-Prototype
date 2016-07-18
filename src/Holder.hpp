#pragma once

//
// 汎用的なオブジェクト使い回し
//

#include <string>
#include <map>


namespace ngs {

template <typename T>
class Holder {
  std::map<std::string, T> objects_;


public:
  Holder() = default;

  void add(const std::string& key, const T& object) {
    objects_.insert(std::make_pair(key, object));
  }

  bool hasObject(const std::string& key) const {
    return objects_.count(key);
  }

  T getForKey(const std::string& key) const {
    return objects_.at(key);
  }

};

}

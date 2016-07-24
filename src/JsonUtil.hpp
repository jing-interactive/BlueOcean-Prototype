﻿#pragma once

//
// Json utility
// 配列から色々生成する
//

#include <sstream>
#include <cinder/Vector.h>
#include "Asset.hpp"


namespace ngs { namespace Json {

template<typename T>
std::vector<T> getArray(const ci::JsonTree& json) noexcept {
  size_t num = json.getNumChildren();

  std::vector<T> array;
  array.reserve(num);

  for (size_t i = 0; i < num; ++i) {
    array.push_back(json[i].getValue<T>());
  }

  return array;
}


// FIXME:JSONからdoubleで取ってくるのが微妙
template<typename T>
T getVec(const ci::JsonTree& json) noexcept {
  T v;
  for (size_t i = 0; i < v.size(); ++i) {
    v[i] = json[i].getValue<double>();
  }

  return v;
}

template<typename T>
ci::JsonTree createFromVec(const T& vec) {
  ci::JsonTree json;
  for (size_t i = 0; i < vec.size(); ++i) {
    json.pushBack(ci::JsonTree("", vec[i]));
  }
  return json;
}

template<typename T>
ci::JsonTree createFromVec(const std::string& key, const T& vec) {
  auto json = ci::JsonTree::makeObject(key);
  for (size_t i = 0; i < vec.size(); ++i) {
    json.pushBack(ci::JsonTree("", vec[i]));
  }
  return json;
}

template<typename T>
ci::ColorT<T> getColor(const ci::JsonTree& json) noexcept {
  return ci::ColorT<T>(json[0].getValue<T>(), json[1].getValue<T>(), json[2].getValue<T>());
}

ci::vec3 getHsvColor(const ci::JsonTree& json) noexcept {
  return ci::vec3(json[0].getValue<float>() / 360.0f, json[1].getValue<float>(), json[2].getValue<float>());
}

template<typename T>
ci::ColorAT<T> getColorA(const ci::JsonTree& json) noexcept {
  return ci::ColorAT<T>(json[0].getValue<T>(), json[1].getValue<T>(), json[2].getValue<T>(), json[3].getValue<T>());
}


template<typename T>
T getValue(const ci::JsonTree& json, const std::string& name, const T& default_value) noexcept {
  return (json.hasChild(name)) ? json[name].getValue<T>()
                               : default_value;
}

} }

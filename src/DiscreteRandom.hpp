#pragma once

//
// 確率配列に基いて値を分布させた乱数
// SOURCE:http://cpprefjp.github.io/reference/random/discrete_distribution.html
//

#include <random>


namespace ngs {

class DiscreteRandom {
  std::mt19937 engine_;
  std::discrete_distribution<std::size_t> dist_;


public:
  DiscreteRandom(const std::vector<double>& probabilities,
                 const std::uint32_t seed = std::mt19937::default_seed)
    :engine_(seed),
     dist_(std::begin(probabilities), std::end(probabilities))
  {}


  std::uint32_t operator()() {
    return dist_(engine_);
  }
  
};

}

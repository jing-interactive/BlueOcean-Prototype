#pragma once

//
// 細々した実装
//

namespace ngs {

// 比較関数(a < b を計算する)
// SOURCE:http://tankuma.exblog.jp/11670448/
template <typename T>
struct LessVec {
  bool operator()(const T& lhs, const T& rhs) const {
    for (size_t i = 0; i < lhs.size(); ++i) {
      if (lhs[i] < rhs[i]) return true;
      if (lhs[i] > rhs[i]) return false;
    }

    return false;
  }
};

}

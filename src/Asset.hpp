#pragma once

//
// アセット読み込み
//  OSX版のみDEBUGビルドで特殊なパスから読み込むようにしている
//


namespace ngs { namespace Asset {

std::string fullPath(const std::string& path) {
#if defined (DEBUG) && defined (CINDER_MAC)
  // DEBUG時、OSXはプロジェクトの場所からfileを読み込む
  std::string full_path(std::string(PREPRO_TO_STR(SRCROOT)) + "../assets/" + path);
  return full_path;
#else
  // TIPS:何気にgetAssetPathがいくつかのpathを探してくれている
  auto full_path = ci::app::getAssetPath(path);
  return full_path.string();
#endif
}

ci::DataSourceRef load(const std::string& path) noexcept {
  return ci::loadFile(fullPath(path));
}

} }

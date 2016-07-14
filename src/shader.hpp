#pragma once

//
// シェーダー
//   GLSL3.30と3.0ESの違いを吸収する
//

#include <fstream>
#include <utility>
#include <regex>


namespace ngs {

using Shader = std::pair<std::string, std::string>;


// テキストファイル -> std::string
std::string readFile(const std::string& path) {
  std::ifstream ifs(path);

  std::string str((std::istreambuf_iterator<char>(ifs)),
                  std::istreambuf_iterator<char>());

  return str;
}


// 機種依存部分を置換
std::string replaceText(std::string text) {
  std::vector<std::pair<std::regex, std::string> > replace{
#if defined (CINDER_COCOA_TOUCH)
    { std::regex("\\$version\\$"), "#version 300 es" },
    { std::regex("\\$precision\\$"), "precision mediump float;" },
#else
    { std::regex("\\$version\\$"), "#version 330" },
    { std::regex("\\$precision\\$"), "" },
#endif
  };

  for (const auto& t : replace) {
    text = std::regex_replace(text, t.first, t.second);
  }

  return text;
}


// シェーダーを読み込む
//   パスに拡張子は要らない
Shader readShader(const std::string& vertex_path,
                  const std::string& fragment_path) {
  auto vertex_shader   = readFile(Asset::fullPath(vertex_path + ".vsh"));
  vertex_shader = replaceText(vertex_shader);
  
  auto fragment_shader = readFile(Asset::fullPath(fragment_path + ".fsh"));
  fragment_shader = replaceText(fragment_shader);

  return std::make_pair(vertex_shader, fragment_shader);
}

}

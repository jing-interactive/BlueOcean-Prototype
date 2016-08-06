#pragma once

//
// ステージ上の物体を生成
//

#include "Asset.hpp"


namespace ngs {

class StageObjMesh {
  std::map<std::string, ci::TriMesh> meshes_;


  ci::TriMesh& loadMesh(const std::string& path) {
    if (!meshes_.count(path)) {
      ci::ObjLoader loader(Asset::load(path));
      ci::TriMesh mesh(loader);

      meshes_.insert(std::make_pair(path, mesh));
    }
    
    return meshes_.at(path);
  }

  
public:
  StageObjMesh() = default;

  ci::gl::VboMeshRef createBatch(const std::vector<StageObj>& objects) {
    ci::geom::SourceMods mods;
    for (const auto& obj : objects) {
      // モデルをアフィン変換して追加
      mods &= loadMesh(obj.getName()) >> ci::geom::Transform(obj.getTransfomation());
    }

    return ci::gl::VboMesh::create(mods);
  }

};

}

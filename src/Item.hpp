#pragma once

//
// 収集したアイテム
//

namespace ngs {

class Item {
  ci::gl::VboMeshRef model_;


public:
  Item() = default;
  
  Item(const ci::JsonTree& param) {
    
    ci::ObjLoader loader(Asset::load(param.getValueForKey<std::string>("file")));
    model_ = ci::gl::VboMesh::create(loader);
  }

  void draw() {
    if (!model_) return;

    ci::gl::draw(model_);
  }
  
};

}

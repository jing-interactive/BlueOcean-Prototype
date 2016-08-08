#pragma once

//
// 円グラフ
//

namespace ngs {

class PieChart {
  ci::gl::VboMeshRef bg_;
  ci::gl::VboMeshRef ring_;


public:
  PieChart() {
    {
      ci::ObjLoader loader(Asset::load("fill_circle.obj"));
      bg_ = ci::gl::VboMesh::create(loader);
    }
    {
      ci::ObjLoader loader(Asset::load("ring.obj"));
      ring_ = ci::gl::VboMesh::create(loader);
    }
  }


  void draw(const ci::vec2& center, const float radius, const float rate, const ci::Color& color) {
    ci::gl::pushModelMatrix();

    ci::gl::translate(center);
    ci::gl::scale(radius, radius);
    
    ci::gl::color(0.0, 0.0, 0.0);
    ci::gl::draw(bg_);

    ci::gl::color(1.0, 1.0, 1.0);
    ci::gl::draw(ring_);

    ci::gl::color(color);
    Draw::fillArc(0, 0, 0.85f, 0, -M_PI * 2.0 * rate, 20);
    
    ci::gl::popModelMatrix();
  }
  
};

}
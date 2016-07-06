#pragma once

// 
// Stage描画
//

#include "Stage.hpp"


namespace ngs { namespace StageDraw {

void draw(const std::vector<Terrain>& terrains, const ci::gl::BatchRef& model) {
  for (const auto& t : terrains) {
    ci::gl::pushModelMatrix();
    
    ci::gl::color(t.color);
    ci::gl::translate(t.pos);

    model->draw();
    
    ci::gl::popModelMatrix();
  }
}

}}

#pragma once

//
// UI周り
//

#include "Draw.hpp"


namespace ngs { namespace UI {

// ワールド座標を画面上の座標に再計算
ci::vec3 getScreenPosition(const ci::vec3& pos,
                           const ci::CameraPersp& camera,
                           const ci::CameraPersp& ui_camera) {
  // ワールド座標→スクリーン座標
  ci::vec3 ndc = camera.worldToNdc(pos);

  // 画面奥にまっすぐ伸びるレイ
  ci::Ray ray = ui_camera.generateRay((ndc.x + 1.0f) / 2.0f, (ndc.y + 1.0f) / 2.0f,
                                      ui_camera.getAspectRatio());

  // nearプレーンでの座標
  ci::vec3 p = ray.calcPosition(ui_camera.getNearClip());
  
  return p;
}

void drawPieChart(const ci::vec2& center, const float rate, const ci::Color& color) {
  ci::gl::color(0.0, 0.0, 0.0);
  ci::gl::drawSolidCircle(center, 0.0024, 20);

  ci::gl::color(1.0, 1.0, 1.0);
  ci::gl::lineWidth(2);
  ci::gl::drawStrokedCircle(center, 0.0023, 20);

  ci::gl::color(color);
  Draw::fillArc(center.x, center.y, 0.0019, 0, -M_PI * 2.0 * rate, 20);
}

} }

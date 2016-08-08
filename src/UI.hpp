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

} }

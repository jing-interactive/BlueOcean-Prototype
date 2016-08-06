#pragma once

//
// UI周り
//

namespace ngs { namespace UI {

// ワールド座標を画面上の座標に再計算
ci::vec3 getScreenPosition(const ci::vec3& pos, const ci::CameraPersp& camera) {
  // ワールド座標→スクリーン座標
  ci::vec3 ndc = camera.worldToNdc(pos);

  // 画面奥にまっすぐ伸びるレイ
  ci::Ray ray = camera.generateRay((ndc.x + 1.0f) / 2.0f, (ndc.y + 1.0f) / 2.0f,
                                   camera.getAspectRatio());

  // nearプレーンでの座標
  ci::vec3 p = ray.calcPosition(0.0f);
  
  return p;
}

} }

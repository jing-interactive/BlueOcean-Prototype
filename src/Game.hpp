#pragma once

//
// 試作版アプリ
//

#include <cinder/gl/gl.h>
#include <cinder/Camera.h>
#include <cinder/params/Params.h>
#include <cinder/Perlin.h>
#include <cinder/Ray.h>
#include <cinder/Frustum.h> 
#include "Asset.hpp"
#include "Params.hpp"
#include "Shader.hpp"
#include "Holder.hpp"
#include "StageObj.hpp"
#include "TiledStage.hpp"
#include "StageDraw.hpp"
#include "StageObjDraw.hpp"
#include "RelicDraw.hpp"
#include "Ship.hpp"
#include "ShipCamera.hpp"
#include "Route.hpp"
#include "Time.hpp"
#include "Light.hpp"
#include "DayLighting.hpp"
#include "Target.hpp"
#include "Sea.hpp"
#include "RouteDraw.hpp"
#include "Search.hpp"
#include "UI.hpp"
#include "Draw.hpp"
#include "PieChart.hpp"
#include "ConnectionHolder.hpp"
#include "AudioEvent.hpp"
#include "DiscreteRandom.hpp"


namespace ngs {

class Game {
  enum {
    BLOCK_SIZE = 64,
    
    FBO_WIDTH  = 512,
    FBO_HEIGHT = 512,
  };

  Event<Arguments>& event_;
  ConnectionHolder holder_;
  
  const ci::JsonTree& params_;
  
  ci::CameraPersp camera;

  float fov;
  float near_z;
  float far_z;

  ci::CameraPersp ui_camera_;
  float ui_fov_;
  float ui_near_z_;

  uint32_t touch_id_;

  ci::quat rotate_;
  ci::vec3 translate_;

  ci::vec2 camera_angle_;
  ci::vec2 angle_restriction_;
  float distance_;
  ci::vec2 distance_restriction_;
  
  bool camera_modified_;

  bool touching_;
  double camera_auto_mode_;
  
  // カメラの操作感
  float camera_rotation_sensitivity_;
  float camera_translation_sensitivity_;
  
  ci::Color bg_color;
  
  // Stage生成時の乱数調整用
  int octave;
  int seed;
  ci::vec3 random_scale;
  ci::Perlin random;

  // 陸地
  TiledStage stage;

  // 海面
  Sea sea_;
  float sea_level_;
  
  ci::ColorA sea_color_;
  ci::vec2 sea_offset_;
  ci::vec2 sea_speed_;
  float sea_wave_;
  
  ci::gl::Texture2dRef sea_texture_;
  ci::gl::GlslProgRef	sea_shader_;
  ci::gl::VboMeshRef sea_mesh_;
  
  ci::gl::FboRef fbo_;
  
  StageDrawer stage_drawer_;
  StageObjDrawer stageobj_drawer_;
  RelicDrawer relic_drawer_;
  RouteDrawer route_drawer_;

  bool picked_;
  ci::AxisAlignedBox picked_aabb_;
  ci::vec3 picked_pos_;

  // 船
  Ship ship_;
  ShipCamera ship_camera_;

  // 経路
  bool has_route_;
  double route_start_time_;
  double route_end_time_;
  ci::ivec3 search_pos_;

  Target target_;

  // 遺物の探索
  bool searching_;
  ci::ivec2 search_block_;
  size_t search_index_;
  double search_start_time_;
  double search_end_time_;

  double search_resolution_time_;
  ci::vec2 search_state_rate_;
  
  // 時間管理
  Time start_time_;
  double duration_;

  Light light_;
  DayLighting day_lighting_;
  // 調整用
  ci::vec3 light_direction_;
  
  // UI用
  Light ui_light_;
  
  ci::gl::GlslProgRef ui_shader_;

  PieChart pie_chart_;

  // アイテム発見用
  DiscreteRandom random_item_;
  // 見つけたアイテム
  std::set<std::string> found_items_;

  
  // デバッグ用
  bool disp_stage_;
  bool disp_stage_obj_;
  bool disp_sea_;
  
  bool pause_day_lighting_;
  bool pause_sea_tide_;
  bool pause_ship_camera_;


  
#if !defined (CINDER_COCOA_TOUCH)
  // iOS版はダイアログの実装が無い
  ci::params::InterfaceGlRef params;
#endif



  // アイテムゲット用の分布配列を生成
  static std::vector<double> createItemProbabilities(const ci::JsonTree& params) {
    int total_num = params["item.body"].getNumChildren();
    std::vector<double> probabilities;
    probabilities.reserve(total_num);

    for (const auto& p : params["item.body"]) {
      probabilities.push_back(p.getValueForKey<double>("probability"));
    }

    return probabilities;
  }
  

  void createStage() {
    stage = TiledStage(params_, BLOCK_SIZE, random, random_scale);
    stage_drawer_.clear();
    stageobj_drawer_.clear();

    // 探索は中止
    searching_ = false;
    has_route_ = false;
  }

  void createSeaMesh() {
    ci::TriMesh mesh;
    ci::vec2 size(BLOCK_SIZE, BLOCK_SIZE);

    ci::vec3 p[] = {
      {      0, 0,      0 },
      { size.x, 0,      0 },
      {      0, 0, size.y },
      { size.x, 0, size.y },
    };
    ci::vec2 uv[] = {
      { 0, 0 },
      { 1, 0 },
      { 0, 1 },
      { 1, 1 }
    };
    
    mesh.appendPositions(&p[0], 4);
    mesh.appendTexCoords0(&uv[0], 4);
    mesh.appendTriangle(0, 2, 1);
    mesh.appendTriangle(1, 2, 3);

    sea_shader_ = createShader("water", "water");
    sea_shader_->uniform("uTex0", 0);
    sea_shader_->uniform("uTex1", 1);
    sea_texture_ = ci::gl::Texture2d::create(ci::loadImage(Asset::load(params_.getValueForKey<std::string>("sea.wave_texture"))),
                                             ci::gl::Texture2d::Format().wrap(GL_REPEAT));
    sea_mesh_ = ci::gl::VboMesh::create(mesh);
  }
  
   
  void checkContainsStage(const int x, const int z,
                          bool checked[][7],
                          std::vector<ci::ivec2>& disp_stages,
                          const ci::ivec2& center,
                          const ci::Frustum& frustum) {
    // すでにチェック済み
    if (checked[x + 3][z + 3]) return;
    checked[x + 3][z + 3] = true;

    // 判定用のAABBを取得
    ci::ivec2 stage_pos(x + center.x, z + center.y);
    const auto& s = stage.getStage(stage_pos);
    const auto& b = s.getAABB();

    // TIPS:海面の描画を含む
    ci::vec3 min_pos = b.getMin();
    ci::vec3 max_pos = b.getMax();
    max_pos.y = std::max(max_pos.y, sea_level_);
    
    ci::vec3 pos((x + center.x) * BLOCK_SIZE, 0, (z + center.y) * BLOCK_SIZE);
    ci::AxisAlignedBox aabb(min_pos + pos, max_pos + pos);
    if (!frustum.intersects(aabb)) return;
    
    disp_stages.push_back(stage_pos);
    
    // 再帰で周囲の地形もチェック
    if (x > -3) checkContainsStage(x - 1,     z, checked, disp_stages, center, frustum);
    if (x <  3) checkContainsStage(x + 1,     z, checked, disp_stages, center, frustum);
    if (z > -3) checkContainsStage(    x, z - 1, checked, disp_stages, center, frustum);
    if (z <  3) checkContainsStage(    x, z + 1, checked, disp_stages, center, frustum);
  }
  
  
  // カメラから見える地形を選出
  std::vector<ci::ivec2> checkContainsStage(const ci::ivec2& center,
                                            const ci::Frustum& frustum) {
    // TIPS:再帰を利用して周囲のブロックの可視判定
    std::vector<ci::ivec2> disp_stages;
    bool checked[7][7] = {};
    checkContainsStage(0, 0, checked, disp_stages, center, frustum);

    return disp_stages;
  }

  
  void pickStage(const ci::vec2& pos) {
    picked_ = false;

    // スクリーン座標→正規化座標
    float sx = pos.x / ci::app::getWindowWidth();
    float sy = 1.0f - pos.y / ci::app::getWindowHeight();
    
    ci::Ray ray = camera.generateRay(sx, sy,
                                     camera.getAspectRatio());

    // 海面と交差する必要がある
    float sea_z;
    if (!ray.calcPlaneIntersection(ci::vec3(0, sea_level_, 0), ci::vec3(0, 1, 0), &sea_z)) return;

    ci::vec3 sea_pos = ray.calcPosition(sea_z);
    // TIPS:誤差があると経路検索で目的地にたどり着かないw
    sea_pos.y = sea_level_;

    // どの区画をクリックしたか判定
    float click_z;
    if (!ray.calcPlaneIntersection(ci::vec3(0, 0, 0), ci::vec3(0, 1, 0), &click_z)) return;
    ci::vec3 p = ray.calcPosition(click_z);

    float cross_min_z = std::numeric_limits<float>::max();
    ci::ivec2 center_pos(glm::floor(p.x / BLOCK_SIZE), glm::floor(p.z / BLOCK_SIZE));
    ci::Frustum frustum(camera);
    auto draw_stages = checkContainsStage(center_pos, frustum);

    DOUT << "draw_stages:" << draw_stages.size() << std::endl;
    
    for (const auto& stage_pos : draw_stages) {
      // Rayを平行移動
      ci::Ray t_ray = ray;
      t_ray.setOrigin(ray.getOrigin() + ci::vec3(stage_pos.x * -BLOCK_SIZE, 0, stage_pos.y * -BLOCK_SIZE));
          
      const auto& s = stage.getStage(stage_pos);

      float cross_z[2];
      if (!s.getAABB().intersect(t_ray, &cross_z[0], &cross_z[1])) continue;
      if (cross_z[0] >= cross_min_z) continue;
          
      // TriMeshを調べて交差点を特定する
      auto result = intersect(t_ray, s.getLandMesh());
      if (result.first && result.second < cross_min_z) {
        picked_ = true;

        // Pick座標を保持
        cross_min_z = result.second;
        picked_pos_ = ray.calcPosition(result.second);

        // AABBも保持
        ci::mat4 m = glm::translate(ci::mat4(1.0), ci::vec3(stage_pos.x * BLOCK_SIZE, 0, stage_pos.y * BLOCK_SIZE));
        picked_aabb_ = s.getAABB().transformed(m);
      }

      // 遺物を直接クリックしてるか調べる
      auto relic_cross = intersect(t_ray, stage.getRelics(stage_pos), sea_level_);
      if (std::get<0>(relic_cross) && (std::get<1>(relic_cross) < cross_min_z)) {
        picked_ = true;
        cross_min_z = std::get<1>(relic_cross);

        ci::vec3 p(std::get<2>(relic_cross)); 
        picked_pos_ = p + ci::vec3(stage_pos.x * BLOCK_SIZE + 0.5, 0, stage_pos.y * BLOCK_SIZE + 0.5);

        DOUT << "picked relics " << picked_pos_ << std::endl;
      }
    }

    if (!picked_) return;

    // クリックした場所が海面より低い→Rayと海面の交差を優先
    if (picked_pos_.y < sea_pos.y) {
      picked_pos_ = sea_pos;
      DOUT << "picked sea " << picked_pos_ << std::endl;
    }
  }

  // 経路探索
  void searchRoute() {
    ci::ivec3 start = ship_.getPosition();
    ci::ivec3 end   = glm::floor(picked_pos_);

    Time current_time;
    double duration = current_time - start_time_;

    auto route = Route::search(start, end,
                               duration, ship_.getRequiredTime(),
                               stage, sea_);

    if (!route.empty()) {
      const auto& waypoint = route.back();
      search_pos_ = waypoint.pos;

      ship_.setRoute(route);
      route_start_time_ = duration;
      route_end_time_   = waypoint.duration;
      ship_.start();
      target_.setPosition(search_pos_);
      ship_camera_.start();

      has_route_ = true;

      AudioEvent::play(event_, "route_start");
    }
  }

  // 遺物探索
  void searchRelic() {
    // 到着地点周囲に遺物があるか調べる
    auto search_result = Search::checkNearRelic(search_pos_, stage);
    if (!search_result.first) return;
    
    const auto& result = search_result.second;
    
    auto& relics = stage.getRelics(result.block_pos);
    auto& relic  = relics[result.index];

    double required_time = relic.search_required_time;

    // 経路終了時間から探索開始
    double current_time = route_end_time_;
    while(required_time > 0.0) {
      // 一定間隔で高さを調べる
      float h0 = sea_.getLevel(current_time);
      float h1 = sea_.getLevel(current_time + search_resolution_time_);

      // 探索時間を決める
      // 1. 常に海面上なら通常時間
      // 2. 海面下にある場合は通常の３倍
      // 3. 分解能の間で海面上と海面下が同居する場合は２倍
      double time = search_resolution_time_;
      current_time += time;
      
      if ((h0 >= relic.position.y) && (h1 >= relic.position.y)) {
        // ずっと海面下
        time *= search_state_rate_.x;
      }
      else if ((h0 >= relic.position.y) || (h1 >= relic.position.y)) {
        // 時々海面上
        time *= search_state_rate_.y;
      }
      // else {
      //   // ずっと海面上
      // }
      
      required_time -= time;
      if (required_time < 0.0) {
        // 引きすぎた分を差し戻す
        current_time += required_time;
      }
    }

    // 結果を保存
    searching_         = true;
    search_block_      = result.block_pos;
    search_index_      = result.index;
    search_start_time_ = route_end_time_;
    search_end_time_   = current_time;
  }

  
  // 画面クリックからの行動を始める
  void startAction() {
    // それまでの行動を中止
    // FIXME:ルートが見つからなかった場合は行動続行としたい
    has_route_ = false;
    searching_ = false;
    
    searchRoute();
    if (has_route_) {
      searchRelic();
    }

    DOUT << "route:" << has_route_ << std::endl;
    if (has_route_) {
      DOUT << "to:" << search_pos_ << std::endl
           << "arrived time:" << route_end_time_
           << "(" << route_end_time_ - route_start_time_ << ")"
           << std::endl;
    }

    DOUT << "search:" << searching_ << std::endl;
    if (searching_) {
      DOUT << "search finish time:" << search_end_time_
           << "(" << search_end_time_ - search_start_time_ << ")"
           << std::endl;
    }
  }


  // アイテム発見
  void foundItem() {
    // イベント送信
    int index = random_item_();
    const auto& item = params_["item.body"][index].getValueForKey<std::string>("name");
    bool new_item    = found_items_.count(item) ? false : true;

    found_items_.insert(item);
        
    Arguments arguments = {
      { "item",     index },
      { "new_item", new_item },
    };
      
    event_.signal("scene_item_reporter", arguments);
  }
  
  // 探索を進める
  void progressSearch(const double duration) {
    // まだ時間じゃない
    if (duration < search_start_time_) return;

    auto& relics = stage.getRelics(search_block_);
    auto& relic  = relics[search_index_];
    
    if (duration >= search_end_time_) {
      // 終了
      relic.searched = true;
      relic.searched_time = relic.search_required_time;

      searching_ = false;
      foundItem();
      
      return;
    }

    relic.searched_time = duration - search_start_time_;
  }

  
  // 回転操作
  void handlingRotation(const ci::vec2& current_pos, const ci::vec2& prev_pos) {
    ci::vec2 d{ current_pos - prev_pos };
    float l = ci::length(d);
    if (l > 0.0f) {
      camera_angle_.x += -d.y * camera_rotation_sensitivity_;
      camera_angle_.x = glm::clamp(camera_angle_.x, angle_restriction_.x, angle_restriction_.y);
      camera_angle_.y += -d.x * camera_rotation_sensitivity_;
      
      rotate_ = glm::angleAxis(ci::toRadians(camera_angle_.y), ci::vec3(0.0f, 1.0f, 0.0f))
        * glm::angleAxis(ci::toRadians(camera_angle_.x), ci::vec3(1.0f, 0.0f, 0.0f));
    }
  }

  // 平行移動操作
  void handlingTranslation(const ci::vec2& current_pos, const ci::vec2& prev_pos) {
    ci::vec2 d{ current_pos - prev_pos };
    ci::vec3 v{ d.x, 0.0f, d.y };

    float t = std::tan(ci::toRadians(fov) / 2.0f) * distance_;
    auto p = (rotate_ * v) * t * camera_translation_sensitivity_;
    translate_.x -= p.x;
    translate_.z -= p.z;
  }

  // ズーミング操作
  void handlingZooming(const float zooming) {
    float t = std::tan(ci::toRadians(fov) / 2.0f) * distance_;
    distance_ = glm::clamp(distance_- zooming * t, distance_restriction_.x, distance_restriction_.y);
  }

    
  // 陸地の描画
  void drawStage(const std::vector<ci::ivec2>& draw_stages) {
    ci::gl::ScopedGlslProg shader(stage_drawer_.getShader());

    for (const auto& stage_pos : draw_stages) {
      ci::vec3 pos(ci::vec3(stage_pos.x * BLOCK_SIZE, 0, stage_pos.y * BLOCK_SIZE));

      ci::mat4 transform = glm::translate(pos);
      ci::gl::setModelMatrix(transform);
      
      const auto& s = stage.getStage(stage_pos);
      if (disp_stage_) {
        stage_drawer_.draw(stage_pos, s);
      }
      if (disp_stage_obj_) {
        stageobj_drawer_.draw(stage_pos, s);
      }
    }
  }

  void drawRelics(const std::vector<ci::ivec2>& draw_stages) {
    const ci::vec3 center = ship_.getPosition();
    
    for (const auto& stage_pos : draw_stages) {
      ci::vec3 pos(ci::vec3(stage_pos.x * BLOCK_SIZE, 0, stage_pos.y * BLOCK_SIZE));

      relic_drawer_.draw(stage.getRelics(stage_pos), pos, center - pos, sea_level_);
    }
  }

  // 経路表示
  void drawRoute() {
    route_drawer_.draw(ship_.getRoute(), ui_light_, sea_level_);
  }


  // 各種コールバックを登録
  void registerCallbacks() {
    holder_ += event_.connect("ship_arrival",
                              [this](const Connection&, const Arguments&) {
                                DOUT << "ship_arrival" << std::endl;
                                has_route_ = false;
                                target_.arrived();
                                ship_camera_.arrived();
                                if (searching_) {
                                  AudioEvent::play(event_, "search_start");
                                }
                                else {
                                  AudioEvent::play(event_, "arrived");
                                }
                              });
  }


  // ゲーム内情報を書き出す
  void storeRecords() {
    ci::JsonTree object;

    // 開始時間
    auto duration = start_time_.getDuration();
    object.pushBack(ci::JsonTree("start_time", duration));

    // 探索情報
    {
      object.pushBack(ci::JsonTree("searching", searching_));

      if (searching_) {
        object.pushBack(Json::createFromVec("search_block", search_block_));
        object.pushBack(ci::JsonTree("search_index",        int(search_index_)));
        object.pushBack(ci::JsonTree("search_start_time",   search_start_time_));
        object.pushBack(ci::JsonTree("search_end_time",     search_end_time_));
      }
    }

    // 船の経路
    object.pushBack(ci::JsonTree("has_route", has_route_));
    if (has_route_) {
      auto route = ci::JsonTree::makeArray("route");
      for (const auto& waypoint : ship_.getRoute()) {
        ci::JsonTree value = Json::createFromVec(waypoint.pos);
        value.pushBack(ci::JsonTree("", waypoint.duration));
        route.pushBack(value);
      }
      object.pushBack(route);

      object.pushBack(ci::JsonTree("route_start_time", route_start_time_));
      object.pushBack(ci::JsonTree("route_end_time",   route_end_time_));
      object.pushBack(Json::createFromVec("search_pos", search_pos_));
    }

    // 船の情報
    {
      ci::JsonTree ship_info = ci::JsonTree::makeObject("ship");

      ship_info.pushBack(Json::createFromVec("position", ship_.getPosition()));
      ship_info.pushBack(Json::createFromVec("rotation", ship_.getRotation()));
      ship_info.pushBack(ci::JsonTree("height", ship_.getHeight()));
      
      object.pushBack(ship_info);
    }

    // 海の情報
    {
      ci::JsonTree sea_info = ci::JsonTree::makeObject("sea");

      sea_info.pushBack(ci::JsonTree("level", sea_level_));
      sea_info.pushBack(ci::JsonTree("wave", sea_wave_));

      sea_info.pushBack(Json::createFromVec("speed", sea_speed_));
      sea_info.pushBack(Json::createFromVec("color", ci::vec4(sea_color_)));

      object.pushBack(sea_info);
    }
    
    // カメラ情報
    {
      ci::JsonTree camera_info = ci::JsonTree::makeObject("camera");
      
      camera_info.pushBack(Json::createFromVec("angle", camera_angle_));
      camera_info.pushBack(ci::JsonTree("distance", distance_));

      object.pushBack(camera_info);
    }

    // 光源
    {
      ci::JsonTree light_info = ci::JsonTree::makeObject("light");

      light_info.pushBack(Json::createFromVec("direction", light_.direction));
      light_info.pushBack(Json::createFromColorA("ambient", light_.ambient));
      light_info.pushBack(Json::createFromColorA("diffuse", light_.diffuse));
      light_info.pushBack(Json::createFromColorA("specular", light_.specular));

      object.pushBack(light_info);
    }

    // 発見済みアイテム
    {
      ci::JsonTree item_info = ci::JsonTree::makeObject("found_items");

      for (const auto& item : found_items_) {
        item_info.pushBack(ci::JsonTree("", item));
      }
      
      object.pushBack(item_info);
    }
    
    // デバッグ設定
    {
      ci::JsonTree debug_info = ci::JsonTree::makeObject("debug");

      debug_info.pushBack(ci::JsonTree("disp_stage",     disp_stage_));
      debug_info.pushBack(ci::JsonTree("disp_stage_obj", disp_stage_obj_));
      debug_info.pushBack(ci::JsonTree("disp_sea",       disp_sea_));

      debug_info.pushBack(ci::JsonTree("pause_day_lighting", pause_day_lighting_));
      debug_info.pushBack(ci::JsonTree("pause_sea_tide",     pause_sea_tide_));
      debug_info.pushBack(ci::JsonTree("pause_ship_camera",  pause_ship_camera_));
      
      object.pushBack(debug_info);
    }

    object.pushBack(stage.serialize());
    
    object.write(getDocumentPath() / "record.json");
  }
  
  // セーブデータから色々復元
  bool restoreFromRecords() {
    auto path = getDocumentPath() / "record.json";
    if (!ci::fs::is_regular_file(path)) return false;
    
    ci::JsonTree record(ci::loadFile(path));

    // ゲーム開始時刻を復元
    start_time_ = Time(record.getValueForKey<double>("start_time"));

    // 探索状況
    searching_ = record.getValueForKey<bool>("searching");
    if (searching_) {
      search_block_      = Json::getVec<ci::ivec2>(record["search_block"]);
      search_index_      = record.getValueForKey<int>("search_index");
      search_start_time_ = record.getValueForKey<double>("search_start_time");
      search_end_time_   = record.getValueForKey<double>("search_end_time");
    }
    
    // 船の状態
    ship_.setPosition(Json::getVec<ci::vec3>(record["ship.position"]));
    ship_.setHeight(record.getValueForKey<float>("ship.height"));
    ship_.setRotation(Json::getVec<ci::quat>(record["ship.rotation"]));

    // ステージ
    stage.deserialize(record["stage"]);
    
    // カメラ
    translate_    = ship_.getPosition();
    distance_     = record.getValueForKey<float>("camera.distance");
    camera_angle_ = Json::getVec<ci::vec2>(record["camera.angle"]);

    rotate_ = glm::angleAxis(ci::toRadians(camera_angle_.y), ci::vec3(0.0f, 1.0f, 0.0f))
      * glm::angleAxis(ci::toRadians(camera_angle_.x), ci::vec3(1.0f, 0.0f, 0.0f));
    
    // 船の経路
    has_route_ = record.getValueForKey<bool>("has_route");
    if (has_route_) {
      const auto& route = record["route"];
      std::vector<Waypoint> ship_route;
      for (const auto& waypoint : route) {
        auto pos = Json::getVec<ci::ivec3>(waypoint);
        double duration = waypoint.getValueAtIndex<double>(3);
        
        ship_route.push_back({ pos, duration });
      }

      route_start_time_ = record.getValueForKey<double>("route_start_time");
      route_end_time_   = record.getValueForKey<double>("route_end_time");

      ship_.setRoute(ship_route);
      ship_.start();

      search_pos_ = Json::getVec<ci::vec3>(record["search_pos"]);
      target_.setPosition(search_pos_);
      
      // カメラ設定
      ship_camera_.start();
      ship_camera_.update(search_pos_);
      translate_ = ship_camera_.getPosition();
      distance_  = ship_camera_.getDistance();
    }

    // 海演出
    sea_color_ = Json::getColorA<float>(record["sea.color"]);
    sea_speed_ = Json::getVec<ci::vec2>(record["sea.speed"]);
    sea_wave_  = record.getValueForKey<float>("sea.wave");
    sea_level_ = record.getValueForKey<float>("sea.level");

    // 光源
    light_.direction = Json::getVec<ci::vec4>(record["light.direction"]);
    light_.ambient   = Json::getColorA<float>(record["light.ambient"]);
    light_.diffuse   = Json::getColorA<float>(record["light.diffuse"]);
    light_.specular  = Json::getColorA<float>(record["light.specular"]);
    
    light_direction_.x = light_.direction.x;
    light_direction_.y = light_.direction.y;
    light_direction_.z = light_.direction.z;

    // 発見済みアイテム
    {
      for (const auto& item : record["found_items"]) {
        found_items_.insert(item.getValue<std::string>());
      }
    }
    
    // デバッグ設定
    if (record.hasChild("debug")) {
      disp_stage_     = record.getValueForKey<bool>("debug.disp_stage");
      disp_stage_obj_ = record.getValueForKey<bool>("debug.disp_stage_obj");
      disp_sea_       = record.getValueForKey<bool>("debug.disp_sea");
      
      pause_day_lighting_ = record.getValueForKey<bool>("debug.pause_day_lighting");
      pause_sea_tide_     = record.getValueForKey<bool>("debug.pause_sea_tide");
      pause_ship_camera_  = record.getValueForKey<bool>("debug.pause_ship_camera");
    }

    return true;
  }


  void setupDebugEvent() {
    holder_ += event_.connect("debug_item_reporter",
                              [this](const Connection&, const Arguments&) {
                                foundItem();                                
                              });
  }

  
public:
  Game(Event<Arguments>& event, const ci::JsonTree& params)
    : event_(event),
      params_(params),
      fov(params_.getValueForKey<float>("camera.fov")),
      near_z(params_.getValueForKey<float>("camera.near_z")),
      far_z(params_.getValueForKey<float>("camera.far_z")),
      ui_fov_(params_.getValueForKey<float>("ui_camera.fov")),
      ui_near_z_(params_.getValueForKey<float>("ui_camera.near_z")),
      camera_rotation_sensitivity_(params_.getValueForKey<float>("app.camera_rotation_sensitivity")),
      camera_translation_sensitivity_(params_.getValueForKey<float>("app.camera_translation_sensitivity")),
      camera_angle_(Json::getVec<ci::vec2>(params_["camera.angle"])),
      angle_restriction_(Json::getVec<ci::vec2>(params_["camera.angle_restriction"])),
      distance_(params_.getValueForKey<float>("camera.distance")),
      distance_restriction_(Json::getVec<ci::vec2>(params_["camera.distance_restriction"])),
      camera_modified_(false),
      touching_(false),
      camera_auto_mode_(0.0),
      octave(params_.getValueForKey<float>("stage.octave")),
      seed(params_.getValueForKey<float>("stage.seed")),
      random_scale(Json::getVec<ci::vec3>(params_["stage.random_scale"])),
      random(octave, seed),
      stage(params_, BLOCK_SIZE, random, random_scale),
      sea_(params_["sea"]),
      sea_color_(Json::getColorA<float>(params_["sea.color"])),
      sea_speed_(Json::getVec<ci::vec2>(params_["sea.speed"])),
      sea_wave_(params_.getValueForKey<float>("sea.wave")),
      relic_drawer_(params_["relic"]),
      route_drawer_(params_["route"]),
      picked_(false),
      ship_(event_, params_["ship"]),
      ship_camera_(event_, params_),
      has_route_(false),
      target_(params_["target"]),
      searching_(false),
      search_resolution_time_(params_.getValueForKey<double>("search.resolution")),
      search_state_rate_(Json::getVec<ci::vec2>(params_["search.state_rate"])),
      duration_(0.0),
      light_(createLight(params_["light"])),
      day_lighting_(params_["day_lighting"]),
      ui_light_(createLight(params_["ui_light"])),
      ui_shader_(createShader("ui", "ui")),
      random_item_(createItemProbabilities(params_)),
      disp_stage_(true),
      disp_stage_obj_(true),
      disp_sea_(true),
      pause_day_lighting_(false),
      pause_sea_tide_(false),
      pause_ship_camera_(false)
  {
    int width  = ci::app::getWindowWidth();
    int height = ci::app::getWindowHeight();

    camera = ci::CameraPersp(width, height,
                             fov,
                             near_z, far_z);

    camera.setEyePoint(ci::vec3());
    camera.setViewDirection(ci::vec3{ 0.0f, 0.0f, -1.0f });

    ui_camera_ = ci::CameraPersp(width, height,
                                 ui_fov_,
                                 ui_near_z_,
                                 params_.getValueForKey<float>("ui_camera.far_z"));

    ui_camera_.setEyePoint(ci::vec3());
    ui_camera_.setViewDirection(ci::vec3{ 0.0f, 0.0f, -1.0f });

    rotate_ = glm::angleAxis(ci::toRadians(camera_angle_.y), ci::vec3(0.0f, 1.0f, 0.0f))
      * glm::angleAxis(ci::toRadians(camera_angle_.x), ci::vec3(1.0f, 0.0f, 0.0f));

    createDialog();

    bg_color = ci::Color(0, 0, 0);

    createSeaMesh();
    
    // FBO
    auto format = ci::gl::Fbo::Format()
      .colorTexture()
      ;
    fbo_ = ci::gl::Fbo::create(FBO_WIDTH, FBO_HEIGHT, format);

    registerCallbacks();

    {
      // 船の現在位置の高さを取得
      int height = Route::getStageHeight(ci::ivec3(ship_.getPosition()), stage);
      ship_.setHeight(height);
    }
    
    // 記録ファイルがあるなら読み込む
    restoreFromRecords();

    setupDebugEvent();
  }

  
  void resize(const float aspect) {
    camera.setAspectRatio(aspect);
    camera.setFov(getVerticalFov(aspect, fov, near_z));
    
    ui_camera_.setAspectRatio(aspect);
    ui_camera_.setFov(getVerticalFov(aspect, ui_fov_, ui_near_z_));
  }


  void touchesBegan(const int touching_num, const std::vector<Touch>& touches) {
#if defined (CINDER_COCOA_TOUCH)
    // iOS版は最初のを覚えとく
    if (touching_num == 1) {
      touch_id_ = touches[0].getId();
    }
#else
    // PC版はMouseDownを覚えておく
    if ((touches.size() == 1) && touches[0].isMouse()) {
      touch_id_ = touches[0].getId();
    }
#endif

    
    touching_ = true;
  }
  
  void touchesMoved(const int touching_num, const std::vector<Touch>& touches) {
#if defined (CINDER_COCOA_TOUCH)
    // iOS版はシングルタッチ操作
    if (touching_num == 1 && touches.size() == 1)
#else
      // PCはマウス操作で回転
    if ((touches.size() == 1) && touches[0].isMouse())
#endif
    {
      // シングルタッチ操作は回転
      handlingRotation(touches[0].getPos(),
                         touches[0].getPrevPos());
      camera_modified_ = true;
      return;
    }

    if (touches.size() < 2) return;

    auto v1 = touches[0].getPos() - touches[1].getPos();
    auto v2 = touches[0].getPrevPos() - touches[1].getPrevPos();

    float ld = ci::length(v1) - ci::length(v2);
    
    if (std::abs(ld) < 3.0f) {
      handlingTranslation(touches[0].getPos(), touches[0].getPrevPos());
    }
    else {
      handlingZooming(ld * 0.1f);
    }
    camera_modified_ = true;
  }
  
  void touchesEnded(const int touching_num, const std::vector<Touch>& touches) {
#if defined (CINDER_COCOA_TOUCH)
    // iOS版は最初のタッチが終わったら
    if (!camera_modified_
        && (touching_num == 0)
        && (touches.size() == 1)
        && touches[0].getId() == touch_id_) {
      pickAndStartAction(touches[0]);
    }
#else
    // PC版はMouseUpで判定
    if (!camera_modified_
        && (touches.size() == 1)
        && (touches[0].isMouse())) {
      pickAndStartAction(touches[0]);
    }
#endif

    if (touching_num == 0) {
      camera_modified_ = false;
      
      touching_ = false;
      camera_auto_mode_ = 3.0;
    }
  }

  void pickAndStartAction(const Touch& touch) {
    // クリックした位置のAABBを特定
    ci::ivec2 pos = touch.getPos();
    pickStage(pos);

    if (picked_) {
      // 行動開始
      startAction();
    }
  }

  
  void update() {
    Time current_time;
    // アプリ開始時からの経過時間
    duration_ = current_time - start_time_;

    // 探索
    if (searching_) {
      progressSearch(duration_);
    }
    
    if (!pause_sea_tide_) {
      sea_level_ = sea_.getLevel(duration_);
    }

    if (!pause_day_lighting_) {
      auto l = day_lighting_.update(duration_);
      light_.ambient = l.ambient;
      light_.diffuse = l.diffuse;
    }

    ship_camera_.update(ship_.getPosition());
    
    // カメラ位置の計算
    translate_.y = sea_level_;

    // タッチ操作をしばらく行わないと船を中心に捉える
    if (!touching_) {
      if (camera_auto_mode_ > 0.0) {
        camera_auto_mode_ -= 1 / 60.0;
      }
      else if(!pause_ship_camera_) {
        translate_ += (ship_camera_.getPosition() - translate_) * 0.1f;
        distance_  += (ship_camera_.getDistance() - distance_) * 0.1f;
      }
    }

#if 0
    if (has_route_) {
      if(!pause_ship_camera_) {
        translate_ += (ship_camera_.getPosition() - translate_) * 0.1f;
        distance_  += (ship_camera_.getDistance() - distance_) * 0.1f;
      }
    }
#endif

    auto pos = rotate_ * ci::vec3(0, 0, distance_) + translate_;
    
    camera.setEyePoint(pos);
    camera.setOrientation(rotate_);

    sea_offset_ += sea_speed_;

    ship_.update(duration_, sea_level_);
    target_.update(duration_, sea_level_);

    relic_drawer_.update();
  }
  
  void draw() {
    ci::gl::setMatrices(camera);
    ci::gl::disableAlphaBlending();
    ci::gl::enableDepth(true);
    ci::gl::enable(GL_CULL_FACE);

    stage_drawer_.setupLight(light_);
    relic_drawer_.setupLight(ui_light_);
    
    // 陸地の描画
    // 画面中央の座標をレイキャストして求めている
    ci::Ray ray = camera.generateRay(0.5f, 0.5f,
                                     camera.getAspectRatio());
    
    float z;
    if (ray.calcPlaneIntersection(ci::vec3(0, 0, 0), ci::vec3(0, 1, 0), &z)) {
      ci::vec3 p = ray.calcPosition(z);

      // 中央ブロックの座標
      ci::ivec2 pos(glm::floor(p.x / BLOCK_SIZE), glm::floor(p.z / BLOCK_SIZE));

      // 時々データを整理
      {
        // とりあえず1分に一回程度
        if ((ci::app::getElapsedFrames() % (60 * 60)) == 0) {
          DOUT << "Garbage Collection" << std::endl;
          stage.garbageCollection(pos, ci::ivec2(5, 5));
          stage_drawer_.garbageCollection(pos, ci::ivec2(5, 5));
        }
      }

      ci::Frustum frustum(camera);
      auto draw_stages = checkContainsStage(pos, frustum);

      {
        // 海面演出のためにFBOへ描画
        ci::gl::ScopedViewport viewportScope(fbo_->getSize());
        ci::gl::ScopedFramebuffer fboScope(fbo_);
        ci::gl::clear(bg_color);

        drawStage(draw_stages);
        drawRelics(draw_stages);
        ship_.draw(light_);
      }

      ci::gl::clear(bg_color);

      // 海面の描画
      if (disp_sea_) {
        ci::gl::ScopedGlslProg shader(sea_shader_);
        ci::gl::ScopedTextureBind fbo_texture(fbo_->getColorTexture(), 0);
        ci::gl::ScopedTextureBind sea_texture(sea_texture_, 1);
        
        sea_shader_->uniform("offset", sea_offset_);
        sea_shader_->uniform("wave", sea_wave_);
        sea_shader_->uniform("color", sea_color_);
        auto vp = ci::gl::getViewport();
        sea_shader_->uniform("window_size", ci::vec2(vp.second));

        // TIPS:まっさらな画面に描画するので
        //      デプステストは必要ない
        glDepthFunc(GL_ALWAYS);

        for (const auto& stage_pos : draw_stages) {
          ci::vec3 pos(ci::vec3(stage_pos.x * BLOCK_SIZE, sea_level_, stage_pos.y * BLOCK_SIZE));
          ci::mat4 transform = glm::translate(pos);
          ci::gl::setModelMatrix(transform);

          ci::gl::draw(sea_mesh_);
        }

        // デプステストを元に戻す
        glDepthFunc(GL_LESS);
      }
      
      drawStage(draw_stages);
      drawRelics(draw_stages);
      ship_.draw(light_);
      target_.draw(ui_light_);
      
#if 0
      if (picked_) {
        ci::gl::setModelMatrix(ci::mat4(1.0f));

        ci::gl::color(1, 0, 0);
        ci::gl::drawStrokedCube(picked_aabb_);
        ci::gl::drawSphere(picked_pos_, 0.1f);
      }
#endif

      if (has_route_) {
        drawRoute();
      }
    }

    // UI
    ci::gl::setMatrices(ui_camera_);
    ci::gl::enableDepth(false);
    ci::gl::disable(GL_CULL_FACE);

    ci::mat4 transform = glm::translate(ci::vec3(0, 0, -ui_camera_.getNearClip()));
    ci::gl::setModelMatrix(transform);

    {
      ci::gl::ScopedGlslProg shader(ui_shader_);

      if (has_route_) {
        float t = (route_end_time_ - duration_) / (route_end_time_ - route_start_time_);
        
        ci::vec3 pos = UI::getScreenPosition(ship_.getPosition() + ci::vec3(0.5, 1.5, 0.5), camera, ui_camera_);
        pie_chart_.draw(ci::vec2(pos.x, pos.y), 0.0024f, t, ci::Color(0, 1, 0));
      }
      else if (searching_) {
        float t = (search_end_time_ - duration_) / (search_end_time_ - search_start_time_);
        
        ci::vec3 pos = UI::getScreenPosition(ship_.getPosition() + ci::vec3(0.5, 1.5, 0.5), camera, ui_camera_);
        pie_chart_.draw(ci::vec2(pos.x, pos.y), 0.0024f, t, ci::Color(0, 0, 1));
      }
    }
  }

  void debugDraw() {
    // ダイアログ表示
    drawDialog();
  }

  // アプリ終了時
  void cleanup() {
    DOUT << "cleanup" << std::endl;

    storeRecords();
  }


  // ダイアログ関連
#if defined (CINDER_COCOA_TOUCH)
  void createDialog() {}
  void drawDialog() {}
  void destroyDialog() {}
#else
  void createDialog() {
    // 各種パラメーター設定
    params = ci::params::InterfaceGl::create("Preview params", ci::app::toPixels(ci::ivec2(300, 600)));

    params->addParam("Fov", &fov).min(1.0f).max(180.0f).updateFn([this]() {
        camera.setFov(fov);
      });

    params->addParam("Near Z", &near_z).min(0.1f).updateFn([this]() {
        camera.setNearClip(near_z);
      });
    params->addParam("Far Z", &far_z).min(0.1f).updateFn([this]() {
        camera.setFarClip(far_z);
      });

    params->addSeparator();

    params->addParam("BG", &bg_color);

    params->addSeparator();

    params->addParam("Octave", &octave).min(0).max(255)
      .updateFn([this]() {
          createStage();
        });
    params->addParam("Seed", &seed)
      .updateFn([this]() {
          createStage();
        });
    params->addParam("Random Scale X", &random_scale.x).min(0.001f).step(0.001f)
      .updateFn([this]() {
          createStage();
        });
    params->addParam("Random Scale Y", &random_scale.y).min(0.001f).step(0.001f)
      .updateFn([this]() {
          createStage();
        });
    params->addParam("Random Scale Z", &random_scale.z).min(1.0f).step(0.1f)
      .updateFn([this]() {
          createStage();
        });

    params->addSeparator();

    params->addParam("Sea Color",   &sea_color_);
    params->addParam("Sea Speed x", &sea_speed_.x).step(0.00001f);
    params->addParam("Sea Speed y", &sea_speed_.y).step(0.00001f);
    params->addParam("Sea Wave",    &sea_wave_).step(0.001f);
    params->addParam("Sea Level",   &sea_level_).step(0.25f);

    params->addSeparator();

    params->addParam("Disp Stage",    &disp_stage_);
    params->addParam("Disp StageObj", &disp_stage_obj_);
    params->addParam("Disp Sea",      &disp_sea_);

    params->addSeparator();

    light_direction_.x = light_.direction.x;
    light_direction_.y = light_.direction.y;
    light_direction_.z = light_.direction.z;
    
    params->addParam("Light direction", &light_direction_).updateFn([this]() {
        light_.direction.x = light_direction_.x;
        light_.direction.y = light_direction_.y;
        light_.direction.z = light_direction_.z;
      });
    
    params->addParam("Light ambient",  &light_.ambient);
    params->addParam("Light diffuse",  &light_.diffuse);
    params->addParam("Light specular", &light_.specular);
    
    params->addSeparator();

    params->addParam("Pause Day Lighting", &pause_day_lighting_);
    params->addParam("Pause Sea Tide",     &pause_sea_tide_);
    params->addParam("Pause Ship Camera",  &pause_ship_camera_);
  }

  void drawDialog() {
    if (params) params->draw();
  }

  void destroyDialog() {
    params.reset();
  }
#endif

};

}

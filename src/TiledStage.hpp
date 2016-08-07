#pragma once

//
// タイル状に並んだステージ
//

#include <map>
#include <cinder/Perlin.h>
#include "StageObjFactory.hpp"
#include "RelicFactory.hpp"
#include "Stage.hpp"
#include "Relic.hpp"
#include "RelicFactory.hpp"
#include "Misc.hpp"


namespace ngs {

class TiledStage {
  int block_size_;
  
  ci::Perlin random_;
  ci::vec3 random_scale_;

  StageObjFactory stageobj_factory_;
  RelicFactory relic_factory_;
  
  std::map<ci::ivec2, Stage, LessVec<ci::ivec2>> stages_;
  std::map<ci::ivec2, std::vector<Relic>, LessVec<ci::ivec2>> relics_;
  

  void createRelics(const ci::ivec2& pos, const std::vector<std::vector<int>>& height_map) {
    std::vector<Relic> relics;
      
    for (int z = 0; z < block_size_; ++z) {
      for (int x = 0; x < block_size_; ++x) {
        int y = height_map[z][x];
        auto relic = relic_factory_.create(ci::ivec3(x, y, z));
        if (!relic.first) continue;

        relics.push_back(relic.second);
      }
    }

    relics_.insert(std::make_pair(pos, relics));
  }

  bool hasRelics(const ci::ivec2& pos) const {
    return relics_.count(pos);
  }

  
public:
  TiledStage(const ci::JsonTree& params,
             const int block_size, const ci::Perlin& random,
             ci::vec3 random_scale)
    : block_size_(block_size),
      random_(random),
      random_scale_(std::move(random_scale)),
      stageobj_factory_(params["stage_obj"]),
      relic_factory_(params["relic"])
  {}


  bool hasStage(const ci::ivec2& pos) const {
    return stages_.count(pos);
  }
  
  const Stage& getStage(const ci::ivec2& pos) {
    if (!hasStage(pos)) {
      stages_.insert(std::make_pair(pos,
                                    Stage(block_size_, block_size_,
                                          pos.x, pos.y,
                                          random_,
                                          stageobj_factory_,
                                          random_scale_)));

      if (!hasRelics(pos)) {
        const auto& stage = stages_.at(pos);
        createRelics(pos, stage.getHeightMap());
      }
    }
    
    return stages_.at(pos);
  }

  const std::vector<Relic>& getRelics(const ci::ivec2& pos) const {
    return relics_.at(pos);
  }

  std::vector<Relic>& getRelics(const ci::ivec2& pos) {
    return relics_.at(pos);
  }

  int getBlockSize() const {
    return block_size_;
  }

  // シリアライズ
  ci::JsonTree serialize() const {
    ci::JsonTree stage = ci::JsonTree::makeObject("stage");

    // 遺物
    {
      ci::JsonTree relics = ci::JsonTree::makeObject("relics");

      for (const auto& relic : relics_) {
        if (relic.second.empty()) continue;

        ci::JsonTree json = ci::JsonTree::makeObject();
        json.pushBack(Json::createFromVec("pos", relic.first));

        ci::JsonTree body = ci::JsonTree::makeObject("body");
        for (const auto& r : relic.second) {
          ci::JsonTree b;
          
          b.pushBack(Json::createFromVec("position", r.position));
          b.pushBack(ci::JsonTree("type", r.type));
          b.pushBack(ci::JsonTree("found", r.found));
          b.pushBack(ci::JsonTree("searched", r.searched));
          b.pushBack(ci::JsonTree("search_required_time", r.search_required_time));
          b.pushBack(ci::JsonTree("searched_time", r.searched_time));
          b.pushBack(ci::JsonTree("rare", r.rare));

          body.pushBack(b);
        }
        json.pushBack(body);

        relics.pushBack(json);
      }
      
      stage.pushBack(relics);
    }

    return stage;
  }

  void deserialize(const ci::JsonTree& params) {
    // 遺物
    {
      const ci::JsonTree& relics = params["relics"];
      for (const auto& relics : relics) {
        ci::ivec2 pos = Json::getVec<ci::ivec2>(relics["pos"]);

        std::vector<Relic> body;
        for (const auto& b : relics["body"]) {
          body.push_back({
              Json::getVec<ci::ivec3>(b["position"]),
              b.getValueForKey<std::string>("type"),
              b.getValueForKey<bool>("found"),
              b.getValueForKey<bool>("searched"),
              b.getValueForKey<double>("search_required_time"),
              b.getValueForKey<double>("searched_time"),
              b.getValueForKey<float>("rare"),
            });
        }
        relics_.insert(std::make_pair(pos, body));
      }
    }
  }
  
};

}

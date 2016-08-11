#pragma once

//
// オーディオ管理
//
//  同じ名前のサウンドは直前のを止めて鳴らす方式
//

#include <cinder/audio/audio.h>
#include <map>
#include "Asset.hpp"


namespace ngs {

struct AudioInfo {
  std::string type;
  std::string category;
  bool loop;
};


class Audio {
  std::map<std::string, AudioInfo> infos_;

  
  // 効果音用
  std::map<std::string, ci::audio::BufferRef> buffer_;
  std::map<std::string, ci::audio::BufferPlayerNodeRef> buffer_node_;

  // ストリーミング再生用
  std::map<std::string, ci::audio::SourceFileRef> source_;
  std::map<std::string, ci::audio::FilePlayerNodeRef> file_node_;

  // 停止用
  std::map<std::string, ci::audio::SamplePlayerNodeRef> category_node_;


  void setupBufferPlayer(const ci::JsonTree& params,
                         ci::audio::Context* ctx,
                         const ci::audio::SourceFileRef& source) {
    auto buffer = source->loadBuffer();
    
    const auto& name = params.getValueForKey<std::string>("name");
    buffer_.insert(std::make_pair(name, buffer));

    const auto& category = params.getValueForKey<std::string>("category");
    if (!file_node_.count(category)) {
      // TIPS:SPECIFIEDにしないと、STEREOの音源を直接MONO出力できない
      ci::audio::Node::Format format;
      format.channelMode(ci::audio::Node::ChannelMode::SPECIFIED);
      auto node = ctx->makeNode(new ci::audio::BufferPlayerNode(format));
      
      buffer_node_.insert(std::make_pair(category, node));
      category_node_.insert(std::make_pair(category, node));
    }
  }

  void setupFilePlayer(const ci::JsonTree& params,
                       ci::audio::Context* ctx,
                       const ci::audio::SourceFileRef& source) {
    // TIPS:初期値より増やしておかないと、処理負荷で音が切れる
    source->setMaxFramesPerRead(8192);

    const auto& name = params.getValueForKey<std::string>("name");
    source_.insert(std::make_pair(name, source));

    const auto& category = params.getValueForKey<std::string>("category");
    if (!file_node_.count(category)) {
      // TIPS:SPECIFIEDにしないと、STEREOの音源を直接MONO出力できない
      ci::audio::Node::Format format;
      format.channelMode(ci::audio::Node::ChannelMode::SPECIFIED);
      auto node = ctx->makeNode(new ci::audio::FilePlayerNode(format));
      
      file_node_.insert(std::make_pair(category, node));
      category_node_.insert(std::make_pair(category, node));
    }
  }

  
public:
  Audio(const ci::JsonTree& params) {
    auto* ctx = ci::audio::Context::master();
    ctx->enable();

    std::map<std::string,
             std::function<void (const ci::JsonTree&,
                                 ci::audio::Context*,
                                 const ci::audio::SourceFileRef&)>> creator = {
      {
        "se",
        std::bind(&Audio::setupBufferPlayer,
                  this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
      },
      {
        "bgm",
        std::bind(&Audio::setupFilePlayer,
                  this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
      },
    };
    
    for (const auto& p : params) {
      const auto& file = p.getValueForKey<std::string>("file");
      auto source = ci::audio::load(Asset::load(file), ctx->getSampleRate());

      const auto& type = p.getValueForKey<std::string>("type");
      creator.at(type)(p, ctx, source);
      
      AudioInfo info = {
        p.getValueForKey<std::string>("type"),
        p.getValueForKey<std::string>("category"),
        p.getValueForKey<bool>("looping"),
      };

      const auto& name = p.getValueForKey<std::string>("name");
      infos_.insert(std::make_pair(name, info));
    }
  }

  ~Audio() {
    auto* ctx = ci::audio::Context::master();
    ctx->disable();
    ctx->disconnectAllNodes();
  }


  void play(const std::string& name) {
    std::map<std::string,
             std::function<void (const std::string&, const AudioInfo&)>> assign = {
      {
        "se",
        [this](const std::string& name, const AudioInfo& info) {
          auto& buffer = buffer_.at(name);
          auto& node   = buffer_node_.at(info.category);

          if (node->isEnabled()) {
            node->stop();
          }
          
          auto* ctx = ci::audio::Context::master();

          node->setBuffer(buffer);
          node->setLoopEnabled(info.loop);
          
          node >> ctx->getOutput();
          node->start();
        }
      },
      {
        "bgm",
        [this](const std::string& name, const AudioInfo& info) {
          auto& source = source_.at(name);
          auto& node   = file_node_.at(info.category);
          
          if (node->isEnabled()) {
            node->stop();
          }
          
          auto* ctx = ci::audio::Context::master();

          node->setSourceFile(source);
          node->setLoopEnabled(info.loop);

          node >> ctx->getOutput();
          node->start();
        }
      },
    };
    
    const auto& info = infos_.at(name);
    assign.at(info.type)(name, info);
  }

  void stopAll() {
    for (auto& it : category_node_) {
      if (it.second->isEnabled()) {
        it.second->stop();
      }
    }
  }

};

}

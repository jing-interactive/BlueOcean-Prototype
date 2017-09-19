#pragma once

//
// サウンドの一元管理
//

#include "Event.hpp"
#include "Arguments.hpp"


namespace ngs { namespace AudioEvent {

void play(Event& event, const std::string& name) {
  Arguments arguments = {
    { "name", name }
  };

  event.signal("audio", arguments);
}

void stopAll(Event& event) {
  event.signal("audio_stop", Arguments());
}

} }

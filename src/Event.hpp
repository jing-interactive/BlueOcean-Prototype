#pragma once

//
// boost::signals2を利用した汎用的なイベント
//

#include "cinder/Signals.h"
#include "cinder/Noncopyable.h"
#include "Arguments.hpp"
#include <map>


namespace ngs {

    using Connection = ci::signals::Connection;

    class Event : private ci::Noncopyable {
        typedef std::function<void(const Arguments&)> CallbackType;
        using SignalType = ci::signals::Signal<void(const Arguments&)>;

        std::map<std::string, SignalType> signals_;

    public:
        Event() = default;


        Connection connect(const std::string& msg, CallbackType callback) noexcept {
            return signals_[msg].connect(callback);
        }

        void signal(const std::string& msg, const Arguments& args) noexcept {
            signals_[msg].emit(args);
        }

    private:

    };

}

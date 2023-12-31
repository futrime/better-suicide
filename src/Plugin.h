#pragma once

#include <memory>

#include <ll/api/base/KeyValueDB.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/plugin/NativePlugin.h>

#include "Config.h"

namespace plugin {

class Plugin {
public:
    Plugin(ll::plugin::NativePlugin& self);

    Plugin(const Plugin&) = delete;
    Plugin(const Plugin&&) = delete;
    Plugin& operator=(const Plugin&) = delete;
    Plugin& operator=(const Plugin&&) = delete;

    ~Plugin() = default;

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();

private:
    Config                        mConfig;
    std::unique_ptr<ll::KeyValueDB> mPlayerDb;
    ll::plugin::NativePlugin&           mSelf;

    // Event listeners
    ll::event::ListenerPtr mPlayerJoinEventListener;
    ll::event::ListenerPtr mPlayerUseItemEventListener;
    ll::event::ListenerPtr mSetupCommandEventListener;
};

} // namespace plugin

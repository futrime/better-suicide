#pragma once

#include <memory>

#include <ll/api/base/KeyValueDB.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/plugin/Plugin.h>

#include "Config.h"

namespace better_suicide {

class Plugin {
public:
    Plugin(ll::plugin::Plugin& self);

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
    ll::plugin::Plugin&           mSelf;

    // Event listeners
    ll::event::ListenerPtr mPlayerJoinEventListener;
    ll::event::ListenerPtr mPlayerUseItemEventListener;
    ll::event::ListenerPtr mSetupCommandEventListener;
};

} // namespace better_suicide

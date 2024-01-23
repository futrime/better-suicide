#include "Plugin.h"

#include <ll/api/plugin/NativePlugin.h>
#include <memory>

namespace plugin {

// The global plugin instance.
std::unique_ptr<Plugin> plugin;

extern "C" {
_declspec(dllexport) bool ll_plugin_load(ll::plugin::NativePlugin& self) {
    plugin = std::make_unique<plugin::Plugin>(self);

    return true;
}

_declspec(dllexport) bool ll_plugin_unload(ll::plugin::NativePlugin&) {
    plugin.reset();

    return true;
}

_declspec(dllexport) bool ll_plugin_enable(ll::plugin::NativePlugin&) { return plugin->enable(); }

_declspec(dllexport) bool ll_plugin_disable(ll::plugin::NativePlugin&) { return plugin->disable(); }
}

} // namespace plugin

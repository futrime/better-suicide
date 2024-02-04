#pragma once

#include <ll/api/plugin/NativePlugin.h>

namespace better_suicide {

[[nodiscard]] auto getSelfPluginInstance() -> ll::plugin::NativePlugin&;

} // namespace better_suicide

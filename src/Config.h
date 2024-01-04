#pragma once

namespace plugin {

struct Config {
    int  version                = 1;
    bool doGiveClockOnFirstJoin = true;
    bool enableClockMenu        = true;
};

} // namespace plugin

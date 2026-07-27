#pragma once
#include "il2cpp_resolver.h"

// Shared mod state accessed by all features
struct ModState {
    bool aimbot;
    bool autoDodge;
    bool xray;
    bool potatoGfx;
    bool fpsUnlock;
    bool maxViewDist;
    bool ammoDisplay;
    bool esports;
    bool injected;
};

extern ModState g_state;

namespace Hooks {
    void Install();
}

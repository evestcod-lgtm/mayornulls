#include "xray.h"
#include "../hooks.h"
#include "../il2cpp_resolver.h"
#include <android/log.h>

#define TAG "NullsMod/Xray"

// ── Trampoline storage ────────────────────────────────────────────────────────
void (*BushAlphaOrig)(void* self, float alpha, void* method)    = nullptr;
bool (*FogIsVisibleOrig)(void* self, Vec3 pos, void* method)    = nullptr;

// ── BushRenderer::SetAlpha hook — clamp alpha to ~0.15 so bushes are ghost ───
void BushAlphaHook(void* self, float alpha, void* method) {
    if (g_state.xray) {
        // Semi-transparent: you can see through but bushes still render
        alpha = 0.12f;
    }
    if (BushAlphaOrig) BushAlphaOrig(self, alpha, method);
}

// ── FogOfWar::IsVisible hook — always return true ────────────────────────────
// This reveals invisible Brawlers (Leon, Sandy super, etc.)
bool FogIsVisibleHook(void* self, Vec3 pos, void* method) {
    if (g_state.xray) return true;
    if (FogIsVisibleOrig) return FogIsVisibleOrig(self, pos, method);
    return false;
}

namespace Xray {
    void SetEnabled(bool val) {
        // Immediate effect: the hooks read g_state.xray every frame
        // No additional action needed
    }
}

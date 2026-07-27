#include "esports.h"
#include "../hooks.h"
#include "../il2cpp_resolver.h"
#include <android/log.h>

#define TAG "NullsMod/Esports"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

namespace Esports {

// Esports mode: max FPS + max view dist + clean HUD + player outlines
// The HUD cleanup and outlines are done on the Java side (overlay).
// Here we handle the native side: FOV tweak + farClip.

void SetEnabled(bool val) {
    // FOV: crank slightly for better peripheral vision
    uintptr_t fovFn = IL2CPP::Resolve(Offsets::Camera_set_fieldOfView);
    if (fovFn) {
        typedef void (*SetFOV_t)(float, void*);
        ((SetFOV_t)fovFn)(val ? 70.0f : 60.0f, nullptr);
    }

    // Far clip: 300 units in esports mode
    uintptr_t clipFn = IL2CPP::Resolve(Offsets::Camera_set_farClipPlane);
    if (clipFn) {
        typedef void (*SetClip_t)(float, void*);
        ((SetClip_t)clipFn)(val ? 300.0f : 80.0f, nullptr);
    }

    LOGI("Esports mode: %s", val ? "ON" : "OFF");
}

} // namespace Esports

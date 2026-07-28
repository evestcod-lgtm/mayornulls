#include "graphics.h"
#include "../hooks.h"
#include "../il2cpp_resolver.h"
#include <android/log.h>

#define TAG "NullsMod/Gfx"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

void (*QualitySetOrig)(int level, bool applyExpensive, void* method) = nullptr;
void (*CameraFarClipOrig)(float val, void* method)                   = nullptr;

// QualitySettings::SetQuality — force lowest when potato on
void QualitySetHook(int level, bool applyExpensive, void* method) {
    if (g_state.potatoGfx) {
        level = 0;
        applyExpensive = false;
    }
    if (QualitySetOrig) QualitySetOrig(level, applyExpensive, method);
}

// Camera::set_farClipPlane — clamp to 300 when max view dist on
void CameraFarClipHook(float val, void* method) {
    if (g_state.maxViewDist) val = 300.0f;
    if (CameraFarClipOrig) CameraFarClipOrig(val, method);
}

namespace Graphics {

void SetPotato(bool enabled) {
    uintptr_t shadowFn = IL2CPP::Resolve(Offsets::QualitySettings_set_shadows);
    if (shadowFn) {
        typedef void (*SetShadows_t)(int, void*);
        ((SetShadows_t)shadowFn)(enabled ? 0 : 2, nullptr);
    }
    uintptr_t particleFn = IL2CPP::Resolve(Offsets::QualitySettings_set_particles);
    if (particleFn) {
        typedef void (*SetParticles_t)(int, void*);
        ((SetParticles_t)particleFn)(enabled ? 0 : 100, nullptr);
    }
    LOGI("Potato: %s", enabled ? "ON" : "OFF");
}

} // namespace Graphics

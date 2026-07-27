#include "graphics.h"
#include "../hooks.h"
#include "../il2cpp_resolver.h"
#include <android/log.h>

#define TAG "NullsMod/Gfx"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

void (*QualitySetOrig)(int level, bool applyExpensive, void* method) = nullptr;

// ── QualitySettings::SetQuality hook — intercept and force lowest ─────────────
void QualitySetHook(int level, bool applyExpensive, void* method) {
    if (g_state.potatoGfx) {
        level = 0; // Force lowest quality level
        applyExpensive = false;
        LOGI("QualitySettings forced to 0 (potato)");
    }
    if (QualitySetOrig) QualitySetOrig(level, applyExpensive, method);
}

// ── Direct call to disable shadows + particle systems ────────────────────────
namespace Graphics {

void SetPotato(bool enabled) {
    // QualitySettings.shadowDistance = 0
    uintptr_t shadowFn = IL2CPP::Resolve(Offsets::QualitySettings_set_shadows);
    if (shadowFn) {
        typedef void (*SetShadows_t)(int, void*);
        // ShadowQuality.Disable = 0
        ((SetShadows_t)shadowFn)(enabled ? 0 : 2, nullptr);
    }

    // Disable particles by setting global multiplier to 0
    uintptr_t particleFn = IL2CPP::Resolve(Offsets::QualitySettings_set_particles);
    if (particleFn) {
        typedef void (*SetParticles_t)(int, void*);
        ((SetParticles_t)particleFn)(enabled ? 0 : 100, nullptr);
    }

    LOGI("Potato mode: %s", enabled ? "ON" : "OFF");
}

} // namespace Graphics

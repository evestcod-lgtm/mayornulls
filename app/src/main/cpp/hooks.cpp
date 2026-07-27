#include "hooks.h"
#include "il2cpp_resolver.h"
#include "features/aimbot.h"
#include "features/autododge.h"
#include "features/xray.h"
#include "features/graphics.h"
#include "features/esp.h"
#include "features/fps_unlocker.h"
#include "features/esports.h"
#include <android/log.h>

#define TAG "NullsMod"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

namespace Hooks {

void Install() {
    uintptr_t base = IL2CPP::GetLibBase();
    if (!base) {
        LOGE("Install: no base address");
        return;
    }

    bool ok = true;

    // BrawlerComponent::Update — runs every frame per entity
    // We piggyback aimbot + autododge + ammo esp + view dist here
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::BrawlerComponent_Update),
        (void*)BrawlerUpdateHook,
        (void**)&BrawlerUpdateOrig
    );

    // AttackProjectile::Update — trajectory prediction for auto-dodge
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::AttackProjectile_Update),
        (void*)ProjectileUpdateHook,
        (void**)&ProjectileUpdateOrig
    );

    // BushRenderer::SetAlpha — xray (force alpha = 0)
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::BushRenderer_SetAlpha),
        (void*)BushAlphaHook,
        (void**)&BushAlphaOrig
    );

    // FogOfWar::IsVisible — xray (always return true)
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::FogOfWar_IsVisible),
        (void*)FogIsVisibleHook,
        (void**)&FogIsVisibleOrig
    );

    // Camera farClipPlane — max view distance
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::Camera_set_farClipPlane),
        (void*)CameraFarClipHook,
        (void**)&CameraFarClipOrig
    );

    // QualitySettings::SetQuality — potato graphics
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::QualitySettings_SetQuality),
        (void*)QualitySetHook,
        (void**)&QualitySetOrig
    );

    LOGI("Hook install %s", ok ? "complete" : "partial (some offsets wrong — update them)");
}

} // namespace Hooks

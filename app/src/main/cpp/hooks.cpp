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
    if (!base) { LOGE("Install: no base"); return; }

    bool ok = true;

    // BrawlerComponent::Update — aimbot + ammo esp + view dist + xray
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::BrawlerComponent_Update),
        (void*)BrawlerUpdateHook,
        (void**)&BrawlerUpdateOrig
    );

    // AttackProjectile::Update — auto-dodge trajectory
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::AttackProjectile_Update),
        (void*)ProjectileUpdateHook,
        (void**)&ProjectileUpdateOrig
    );

    // BushRenderer::SetAlpha — xray bush ghost
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::BushRenderer_SetAlpha),
        (void*)BushAlphaHook,
        (void**)&BushAlphaOrig
    );

    // FogOfWar::IsVisible — xray invisible reveal
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::FogOfWar_IsVisible),
        (void*)FogIsVisibleHook,
        (void**)&FogIsVisibleOrig
    );

    // Camera::set_farClipPlane — max view dist
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::Camera_set_farClipPlane),
        (void*)CameraFarClipHook,
        (void**)&CameraFarClipOrig
    );

    // QualitySettings::SetQuality — potato gfx
    ok &= IL2CPP::Hook(
        IL2CPP::Resolve(Offsets::QualitySettings_SetQuality),
        (void*)QualitySetHook,
        (void**)&QualitySetOrig
    );

    LOGI("Hooks %s", ok ? "OK" : "partial — update offsets in il2cpp_resolver.h");
}

} // namespace Hooks

#pragma once
#include <cstdint>
#include <cstring>

// ── Null's Brawl IL2CPP offsets (arm64-v8a) ──────────────────────────────────
// These are for a common Null's Brawl build. Update if the version changes.
// Find updated offsets with: il2cppdumper → script.json → search method names
namespace Offsets {
    // Camera
    constexpr uintptr_t Camera_set_farClipPlane   = 0x123456789; // UPDATE
    constexpr uintptr_t Camera_set_fieldOfView     = 0x123456790;

    // Unity Application
    constexpr uintptr_t Application_set_targetFPS  = 0x123456800;

    // QualitySettings
    constexpr uintptr_t QualitySettings_SetQuality  = 0x123456900;
    constexpr uintptr_t QualitySettings_set_shadows  = 0x123456910;
    constexpr uintptr_t QualitySettings_set_particles = 0x123456920;

    // BrawlerComponent (player/enemy entity)
    constexpr uintptr_t BrawlerComponent_Update     = 0x123457000;
    constexpr uintptr_t BrawlerComponent_GetAmmo    = 0x123457010;
    constexpr uintptr_t BrawlerComponent_GetPos     = 0x123457020;
    constexpr uintptr_t BrawlerComponent_IsEnemy    = 0x123457030;
    constexpr uintptr_t BrawlerComponent_IsVisible  = 0x123457040;
    constexpr uintptr_t BrawlerComponent_SetPos     = 0x123457050;

    // Attack / Projectile
    constexpr uintptr_t AttackProjectile_Update     = 0x123458000;
    constexpr uintptr_t AttackProjectile_GetVelocity = 0x123458010;
    constexpr uintptr_t AttackProjectile_GetOwner   = 0x123458020;

    // Aimbot — shooting
    constexpr uintptr_t PlayerInput_SetAimDir       = 0x123459000;
    constexpr uintptr_t PlayerInput_TriggerShoot    = 0x123459010;

    // Rendering — bushes / fog
    constexpr uintptr_t BushRenderer_SetAlpha       = 0x12345A000;
    constexpr uintptr_t FogOfWar_IsVisible          = 0x12345A010;

    // Entity manager (get all entities)
    constexpr uintptr_t EntityManager_GetList       = 0x12345B000;
}

// ── Vector2 / Vector3 structs matching Unity layout ──────────────────────────
struct Vec2 { float x, y; };
struct Vec3 { float x, y, z; };

namespace IL2CPP {
    uintptr_t GetLibBase();
    uintptr_t Resolve(uintptr_t offset);
    bool      Hook(uintptr_t target, void* replacement, void** original);
    bool      ReadMemory(uintptr_t addr, void* buf, size_t size);
    bool      WriteMemory(uintptr_t addr, const void* buf, size_t size);

    template<typename T>
    T Read(uintptr_t addr) {
        T val{};
        ReadMemory(addr, &val, sizeof(T));
        return val;
    }

    template<typename T>
    void Write(uintptr_t addr, T val) {
        WriteMemory(addr, &val, sizeof(T));
    }
}

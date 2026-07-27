#include "aimbot.h"
#include "../hooks.h"
#include "../il2cpp_resolver.h"
#include <android/log.h>
#include <cmath>
#include <cfloat>

#define TAG "NullsMod/Aimbot"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

// ── Trampoline pointers ───────────────────────────────────────────────────────
void (*BrawlerUpdateOrig)(void* self, void* method) = nullptr;
void (*BrawlerUpdateHook_impl)(void* self, void* method);

// ── Field offsets within BrawlerComponent (update per-version) ───────────────
// Find with il2cppdumper + frida memory read
namespace FieldOff {
    constexpr int Position  = 0x60;  // Vec3
    constexpr int IsEnemy   = 0x94;  // bool
    constexpr int Ammo      = 0x98;  // int
    constexpr int MaxAmmo   = 0x9C;  // int
    constexpr int IsVisible = 0xA0;  // bool — for invisible reveal
    constexpr int AimDir    = 0xB0;  // Vec2 — aim direction, write to aim
}

static Vec3 ReadVec3(void* obj, int off) {
    return IL2CPP::Read<Vec3>((uintptr_t)obj + off);
}

static float Dist2D(Vec3 a, Vec3 b) {
    float dx = a.x - b.x, dz = a.z - b.z;
    return sqrtf(dx*dx + dz*dz);
}

// ── Entity list helper: iterate all BrawlerComponent instances ───────────────
// In Unity IL2CPP the fastest approach is walking the static instance list
// that the EntityManager holds. We call it via the hooked resolver.
typedef void* (*GetEntityList_t)(void* method);
static GetEntityList_t GetEntityListFn = nullptr;

static void EnsureEntityFn() {
    if (!GetEntityListFn) {
        GetEntityListFn = (GetEntityList_t)IL2CPP::Resolve(Offsets::EntityManager_GetList);
    }
}

// ── BrawlerComponent::Update hook ────────────────────────────────────────────
// Called every frame for every brawler. We use the local player instance
// (IsEnemy == false) as our reference, then find best target.
void BrawlerUpdateHook(void* self, void* method) {
    // Always call original first — keeps game logic intact
    if (BrawlerUpdateOrig) BrawlerUpdateOrig(self, method);

    bool isEnemy = IL2CPP::Read<bool>((uintptr_t)self + FieldOff::IsEnemy);

    // ── Ammo Display: tag ammo onto enemy for Java overlay to read ───────────
    if (g_state.ammoDisplay && isEnemy) {
        int ammo    = IL2CPP::Read<int>((uintptr_t)self + FieldOff::Ammo);
        int maxAmmo = IL2CPP::Read<int>((uintptr_t)self + FieldOff::MaxAmmo);
        // Write into a shared ring buffer that Java reads via JNI
        AmmoESP::RecordAmmo(self, ammo, maxAmmo);
    }

    // ── X-ray: force all entities visible ────────────────────────────────────
    if (g_state.xray) {
        IL2CPP::Write<bool>((uintptr_t)self + FieldOff::IsVisible, true);
    }

    // ── Max view distance: patch Camera when it ticks ────────────────────────
    if (g_state.maxViewDist && !isEnemy) {
        // Camera farClipPlane is normally ~60, crank to 300
        uintptr_t camFn = IL2CPP::Resolve(Offsets::Camera_set_farClipPlane);
        if (camFn) {
            typedef void (*SetFar_t)(float, void*);
            ((SetFar_t)camFn)(300.0f, nullptr);
        }
    }

    // ── Aimbot: only run on local player ─────────────────────────────────────
    if (!g_state.aimbot || isEnemy) return;

    Vec3 myPos = ReadVec3(self, FieldOff::Position);

    // Find nearest enemy
    EnsureEntityFn();
    void* entityList = GetEntityListFn ? GetEntityListFn(nullptr) : nullptr;
    if (!entityList) return;

    // Unity List<T>: items at offset 0x10, count at 0x18
    int count = IL2CPP::Read<int>((uintptr_t)entityList + 0x18);
    uintptr_t items = IL2CPP::Read<uintptr_t>((uintptr_t)entityList + 0x10);

    void* bestTarget = nullptr;
    float bestDist   = FLT_MAX;

    for (int i = 0; i < count; ++i) {
        void* entity = IL2CPP::Read<void*>(items + 0x20 + i * 8);
        if (!entity) continue;
        bool enemy = IL2CPP::Read<bool>((uintptr_t)entity + FieldOff::IsEnemy);
        if (!enemy) continue;

        Vec3 ePos = ReadVec3(entity, FieldOff::Position);
        float d   = Dist2D(myPos, ePos);
        if (d < bestDist) {
            bestDist   = d;
            bestTarget = entity;
        }
    }

    if (!bestTarget) return;

    // Calculate aim direction to target
    Vec3 tPos = ReadVec3(bestTarget, FieldOff::Position);
    float dx  = tPos.x - myPos.x;
    float dz  = tPos.z - myPos.z;
    float len = sqrtf(dx*dx + dz*dz);
    if (len < 0.001f) return;

    Vec2 aimDir = { dx / len, dz / len };

    // Write aim direction into player
    IL2CPP::Write<Vec2>((uintptr_t)self + FieldOff::AimDir, aimDir);

    // Trigger shoot if target is close enough (range check)
    if (bestDist < 12.0f) {
        uintptr_t shootFn = IL2CPP::Resolve(Offsets::PlayerInput_TriggerShoot);
        if (shootFn) {
            typedef void (*Shoot_t)(void*, void*);
            ((Shoot_t)shootFn)(self, nullptr);
        }
    }
}

#include "autododge.h"
#include "../hooks.h"
#include "../il2cpp_resolver.h"
#include <android/log.h>
#include <cmath>
#include <vector>

#define TAG "NullsMod/Dodge"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

void (*ProjectileUpdateOrig)(void* self, void* method) = nullptr;

// ── Projectile field offsets ──────────────────────────────────────────────────
namespace ProjOff {
    constexpr int Position   = 0x50; // Vec3
    constexpr int Velocity   = 0x5C; // Vec3 (units/sec)
    constexpr int Owner      = 0x70; // void* BrawlerComponent
    constexpr int IsEnemy    = 0x94; // inherited from owner check
}

// ── Local player pointer (set on BrawlerUpdate for our player) ────────────────
static void* s_localPlayer = nullptr;
static Vec3  s_localPos    = {};

void AutoDodge::SetLocalPlayer(void* player, Vec3 pos) {
    s_localPlayer = player;
    s_localPos    = pos;
}

// ── Predict hit: does projectile trajectory pass within radius of player? ─────
static bool WillHit(Vec3 projPos, Vec3 vel, Vec3 playerPos, float radius = 1.2f) {
    // Closest point on ray to playerPos
    // P = projPos + t*vel, minimize |P - playerPos|^2
    float speed = sqrtf(vel.x*vel.x + vel.y*vel.y + vel.z*vel.z);
    if (speed < 0.01f) return false;

    Vec3 d = { vel.x / speed, vel.y / speed, vel.z / speed };
    Vec3 w = { projPos.x - playerPos.x, projPos.y - playerPos.y, projPos.z - playerPos.z };

    float t = -(w.x*d.x + w.y*d.y + w.z*d.z);
    if (t < 0) t = 0;

    // Max travel: check within ~60 frames at 30fps = 2 seconds
    float tMax = 2.0f * speed;
    if (t > tMax) return false;

    Vec3 closest = {
        projPos.x + d.x * t,
        projPos.y + d.y * t,
        projPos.z + d.z * t
    };

    float dx = closest.x - playerPos.x;
    float dz = closest.z - playerPos.z;
    return (dx*dx + dz*dz) < (radius * radius);
}

// ── Dodge: move perpendicular to incoming velocity ────────────────────────────
static void Dodge(Vec3 incomingVel) {
    if (!s_localPlayer) return;

    float speed = sqrtf(incomingVel.x*incomingVel.x + incomingVel.z*incomingVel.z);
    if (speed < 0.01f) return;

    // Perpendicular in XZ plane: (-z, 0, x) normalized
    Vec3 perp = {
        -incomingVel.z / speed,
        0.0f,
        incomingVel.x / speed
    };

    // Target dodge position = current pos + perp * 2.5 units
    Vec3 dodgePos = {
        s_localPos.x + perp.x * 2.5f,
        s_localPos.y,
        s_localPos.z + perp.z * 2.5f
    };

    // Write new position via SetPos
    uintptr_t setPosFn = IL2CPP::Resolve(Offsets::BrawlerComponent_SetPos);
    if (setPosFn) {
        typedef void (*SetPos_t)(void*, Vec3, void*);
        ((SetPos_t)setPosFn)(s_localPlayer, dodgePos, nullptr);
        LOGI("Dodged to (%.1f, %.1f, %.1f)", dodgePos.x, dodgePos.y, dodgePos.z);
    }
}

// ── AttackProjectile::Update hook ────────────────────────────────────────────
void ProjectileUpdateHook(void* self, void* method) {
    if (ProjectileUpdateOrig) ProjectileUpdateOrig(self, method);

    if (!g_state.autoDodge || !s_localPlayer) return;

    // Read projectile owner — skip if it's our own projectile
    void* owner = IL2CPP::Read<void*>((uintptr_t)self + ProjOff::Owner);
    if (owner == s_localPlayer) return;

    Vec3 projPos = IL2CPP::Read<Vec3>((uintptr_t)self + ProjOff::Position);
    Vec3 vel     = IL2CPP::Read<Vec3>((uintptr_t)self + ProjOff::Velocity);

    if (WillHit(projPos, vel, s_localPos)) {
        Dodge(vel);
    }
}

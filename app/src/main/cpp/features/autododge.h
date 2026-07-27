#pragma once
#include "../il2cpp_resolver.h"

extern void (*ProjectileUpdateOrig)(void* self, void* method);
void ProjectileUpdateHook(void* self, void* method);

namespace AutoDodge {
    void SetLocalPlayer(void* player, Vec3 pos);
}

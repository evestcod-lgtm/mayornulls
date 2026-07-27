#pragma once
#include "../il2cpp_resolver.h"

// Trampoline — defined in aimbot.cpp, declared extern in hooks.cpp
extern void (*BrawlerUpdateOrig)(void* self, void* method);
void BrawlerUpdateHook(void* self, void* method);

namespace AmmoESP {
    struct Entry { void* entity; int ammo; int maxAmmo; };
    constexpr int MAX_ENTRIES = 32;
    extern Entry entries[MAX_ENTRIES];
    extern int   count;
    void RecordAmmo(void* entity, int ammo, int maxAmmo);
}
